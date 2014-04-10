/**************************************************
* SysvolCrawler - ADMFILESiniParser.cpp
* AUTHOR: Luc Delsalle	
*
* Parsing engine for administrative template file
*	
* ANSSI/COSSI/DTO/BAI - 2014
***************************************************/

#include "ADMFILESiniParser.h"

VOID RegisterAdmFilesIniParser(_Inout_ PPARSER_IDENTIFIER *pParserID)
{
	*pParserID = (PPARSER_IDENTIFIER) HeapAlloc(hCrawlerHeap, HEAP_ZERO_MEMORY, sizeof (PARSER_IDENTIFIER));
	if (!pParserID)
	{
		DEBUG_LOG(D_ERROR, "Unable to allocate PARSER_IDENTIFIER structure.\r\nExiting now...");
		DoExit(D_ERROR);
	}

	(*pParserID)->tParserName = ADMFILESINI_PARSER_NAME;
	(*pParserID)->tFileMatchingRegExp = ADMFILESINI_MATCHING_FILE_REGEXP;
	(*pParserID)->tFolderMatchingRegExp = NULL;
	(*pParserID)->pParserEntryPoint = ParseAdmFilesIniFile;
}

BOOL ParseAdmFilesIniFile(_In_ PTCHAR tFilePath)
{
	HANDLE hAdmFilesIniFile = INVALID_HANDLE_VALUE;
	PADMFILESINI_FILE_DATA pAdmFilesIniFileData = NULL;
	PINI_FILE_DATA pGenericIniFileData = NULL;
	DWORD dwFileSize = 0, dwNumberOfBytesRead = 0;
	PBYTE pbINIRawDATA = NULL;
	BOOL bMemoryAreaMoved = FALSE;

	if (tFilePath == NULL)
	{
		DEBUG_LOG(D_ERROR, "FILEPATH pointer invalid.\r\nExiting now...");
		DoExit(D_ERROR);
	}
	DEBUG_LOG(D_MISC, "[ADMFILES] Now parsing %ws\r\n", tFilePath);

	hAdmFilesIniFile = CreateFile_s(tFilePath, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
	if (hAdmFilesIniFile == INVALID_HANDLE_VALUE)
	{
		DEBUG_LOG(D_ERROR, "Unable to open file %ws. ErrorCode: %d.\r\n", tFilePath, GetLastError());
		SetLastError(ERROR_ACCESS_DENIED);
		return FALSE;
	}

	dwFileSize = GetFileSize(hAdmFilesIniFile, NULL);
	if (dwFileSize == INVALID_FILE_SIZE)
	{
		DEBUG_LOG(D_ERROR, "Error during reading FileSize for %ws.\r\nExiting now...", tFilePath);
		DoExit(D_ERROR);
	}

	pbINIRawDATA = (PBYTE) HeapAlloc(hCrawlerHeap, HEAP_ZERO_MEMORY, sizeof (DWORD) * dwFileSize);
	if (pbINIRawDATA == NULL)
	{
		DEBUG_LOG(D_ERROR, "pbINIRawDATA pointer invalid.\r\nExiting now...");
		DoExit(D_ERROR);
	}

	if (!ReadFile(hAdmFilesIniFile, pbINIRawDATA, dwFileSize, &dwNumberOfBytesRead, NULL))
	{
		DEBUG_LOG(D_ERROR, "Unable to read file %ws. ErrorCode: %d.\r\n", tFilePath, GetLastError());
		return FALSE;
	}
	CloseHandle(hAdmFilesIniFile);

	if (IsIniFileWcharEncoded(pbINIRawDATA, dwNumberOfBytesRead) == FALSE)
	{
		PBYTE pbINIRawDATATmp = pbINIRawDATA;

		// ADMFILES.ini seems to be an ANSI file, we need to convert it into WHAR
		pbINIRawDATA = (PBYTE) CStrToPtchar(pbINIRawDATA, dwNumberOfBytesRead);
		if ((pbINIRawDATATmp != pbINIRawDATA) && (pbINIRawDATATmp))
			HeapFree(hCrawlerHeap, NULL, pbINIRawDATATmp);

		dwNumberOfBytesRead *= sizeof (WCHAR);
		if (!pbINIRawDATA)
		{
			DEBUG_LOG(D_ERROR, "Unable to convert file %ws to WideChar.\r\n", tFilePath);
			return FALSE;
		}
	}
	else
		// ADMFILES.ini seems to be an WCHAR file, we just need to skip the BOM
		pbINIRawDATA +=2;

	// Parse file to build generic INI structure
	pGenericIniFileData = ParseIniFile((PWCHAR) pbINIRawDATA, dwNumberOfBytesRead, tFilePath);
	if (!pGenericIniFileData)
	{
		DEBUG_LOG(D_ERROR, "Unable to parse generic ini file : %ws.\r\nExiting now...", tFilePath);
		DoExit(D_ERROR);
	}

	// Create structure wich contains ADMFILES.ini data
	pAdmFilesIniFileData = (PADMFILESINI_FILE_DATA) HeapAlloc(hCrawlerHeap, HEAP_ZERO_MEMORY, sizeof (ADMFILESINI_FILE_DATA));
	if (pAdmFilesIniFileData == NULL)
	{
		DEBUG_LOG(D_ERROR, "pAdmFilesIniFileData pointer invalid.\r\nExiting now...");
		DoExit(D_ERROR);
	}
	pAdmFilesIniFileData->dwAdmFileListNum = 0;
	pAdmFilesIniFileData->dwNumberOfUnReferrencedSections = 0;
	pAdmFilesIniFileData->tFilePath = tFilePath;

	FillAdmFilesIniMethods(pAdmFilesIniFileData, pGenericIniFileData);

	// Keep tracking of unknown sections 
	if (pGenericIniFileData->iNumberOfSection)
	{
		pAdmFilesIniFileData->dwNumberOfUnReferrencedSections = pGenericIniFileData->iNumberOfSection;
		for (DWORD i = 0; i < pAdmFilesIniFileData->dwNumberOfUnReferrencedSections; ++i)
		{
			pAdmFilesIniFileData->pUnReferrencedSections[i] = pGenericIniFileData->pSections[i];
		}
		pGenericIniFileData->iNumberOfSection = 0; // set to 0 in order to prevent double free
	}

	// Call printers
	PrintAdmFilesIniDataHeader(pAdmFilesIniFileData->tFilePath);
	PrintData(pAdmFilesIniFileData);
	PrintAdmFilesIniDataFooter(pAdmFilesIniFileData->tFilePath);

	// Cleanup
	if (pbINIRawDATA)
	{
		if (bMemoryAreaMoved == TRUE)
			pbINIRawDATA -=2;
		HeapFree(hCrawlerHeap, NULL, pbINIRawDATA);
	}
	FreeAdmFilesIniFileData(pAdmFilesIniFileData);
	FreeIniFileData(pGenericIniFileData);
	return TRUE;
}

BOOL FreeAdmFilesIniFileData(_In_ PADMFILESINI_FILE_DATA pAdmFilesIniData)
{
	if (pAdmFilesIniData == NULL)
	{
		DEBUG_LOG(D_ERROR, "ADMFILESINI_FILE_DATA pointer invalid.\r\nExiting now...");
		DoExit(D_ERROR);
	}	

	for (DWORD i = 0; i < pAdmFilesIniData->dwAdmFileListNum; ++i)
	{
		PADMFILESINI_ADM_DATA pAdmData = pAdmFilesIniData->pAdmFileList[i];

		if (!pAdmData)
			continue;

		if (pAdmData->tAdmName)
		{
			HeapFree(hCrawlerHeap, NULL, pAdmData->tAdmName);
			pAdmData->tAdmName = NULL;
		}
		HeapFree(hCrawlerHeap, NULL, pAdmData);
	}

	for (DWORD i = 0; i < pAdmFilesIniData->dwNumberOfUnReferrencedSections; ++i)
	{
		PINI_SECTION_DATA pSectionToDelete = pAdmFilesIniData->pUnReferrencedSections[i];

		if (pSectionToDelete)
			FreeSectionData(pSectionToDelete);
	}

	HeapFree(hCrawlerHeap, NULL, pAdmFilesIniData);
	pAdmFilesIniData = NULL;
	return TRUE;
}

BOOL FillAdmFilesIniMethods(_Inout_ PADMFILESINI_FILE_DATA pAdmFilesIniData, _In_ PINI_FILE_DATA pGenericIniFileData)
{
	DWORD dwSectionsToDelNum = 0;
	PINI_SECTION_DATA pSectionsToDelete[MAX_INI_SECTIONS];

	if (!pAdmFilesIniData || !pGenericIniFileData)
	{
		DEBUG_LOG(D_ERROR, "ADMFILESINI_FILE_DATA or INI_FILE_DATA pointer is invalid.\r\nExiting now...");
		DoExit(D_ERROR);
	}

	for (DWORD i = 0; i < pGenericIniFileData->iNumberOfSection; ++i)
	{
		BOOL bIsSectionFound = FALSE;
		PINI_SECTION_DATA pCurrSection = pGenericIniFileData->pSections[i];

		if (!pCurrSection)
			continue;

		if (_tcsstr(pCurrSection->tSectionName, ADMFILES_FILELIST_SECTION))
		{
			bIsSectionFound = TRUE;
			if (FillAdmFilesIniMethodsActions(pAdmFilesIniData, pCurrSection, i) == FALSE)
			{
				DEBUG_LOG(D_ERROR, "Unable to handle properties for section %ws.\r\nExiting now...", pCurrSection->tSectionName);
				DoExit(D_ERROR);
			}
		}

		// Section should be deleted
		if (bIsSectionFound == TRUE)
		{
			if (IsSectionEmpty(pCurrSection))
			{
				pSectionsToDelete[dwSectionsToDelNum] = pCurrSection;
				dwSectionsToDelNum++;
			}
		}
	}

	// Delete section only if every component has been handled
	for (DWORD i = 0; i < dwSectionsToDelNum; ++i)
	{
		if (RemoveSectionInIniData(pGenericIniFileData, pSectionsToDelete[i]) == FALSE)
		{
			DEBUG_LOG(D_ERROR, "Unable to remove property from section.\r\nExiting now...");
			DoExit(D_ERROR);
		}
	}

	return TRUE;
}

BOOL FillAdmFilesIniMethodsActions(_Inout_ PADMFILESINI_FILE_DATA pAdmFilesIniData, _In_ PINI_SECTION_DATA pGenericIniSection, _In_ DWORD dwSectionNumb)
{
	DWORD dwPropertiesToDelNum = 0;
	PINI_PROPERTY_DATA pPropertiesToDelete[MAX_INI_PROPERTIES];

	if (!pAdmFilesIniData || !pGenericIniSection)
	{
		DEBUG_LOG(D_ERROR, "ADMFILESINI_FILE_DATA, INI_SECTION_DATA pointer or section number is invalid.\r\nExiting now...");
		DoExit(D_ERROR);
	}

	for (DWORD i = 0; i < pGenericIniSection->iNumberOfProperty; ++i)
	{
		PINI_PROPERTY_DATA pCurrProperty = pGenericIniSection->pProperties[i];
		PADMFILESINI_ADM_DATA pNewAdmData = NULL;

		pNewAdmData = (PADMFILESINI_ADM_DATA) HeapAlloc(hCrawlerHeap, NULL, sizeof(ADMFILESINI_ADM_DATA));
		if (!pNewAdmData)
		{
			DEBUG_LOG(D_ERROR, "pNewAdmData pointer invalid.\r\nExiting now...");
			DoExit(D_ERROR);
		}
		pNewAdmData->dwAdmVersion = 0;
		pNewAdmData->tAdmName = NULL;

		if (pCurrProperty->tName)
		{
			DWORD dwPropertyLen = (DWORD) _tcslen(pCurrProperty->tName);

			pNewAdmData->tAdmName = (PTCHAR) HeapAlloc(hCrawlerHeap, NULL, (dwPropertyLen + 1) * sizeof(WCHAR));
			if (!(pNewAdmData->tAdmName))
			{
				DEBUG_LOG(D_ERROR, "tAdmName pointer invalid.\r\nExiting now...");
				DoExit(D_ERROR);
			}
			if (memcpy_s((pNewAdmData->tAdmName), sizeof (TCHAR) * dwPropertyLen, pCurrProperty->tName, sizeof (WCHAR) * dwPropertyLen))
			{
				DEBUG_LOG(D_ERROR, "Unable to extract adm name.\r\nExiting now...");
				DoExit(D_ERROR);
			}
			pNewAdmData->tAdmName[dwPropertyLen] = TEXT('\0');
		}

		if (pCurrProperty->tValue)
		{
			DWORD dwAdmVersion = _tstoi(pCurrProperty->tValue);

			pNewAdmData->dwAdmVersion = dwAdmVersion;
		}	

		pAdmFilesIniData->pAdmFileList[pAdmFilesIniData->dwAdmFileListNum] = pNewAdmData;
		pAdmFilesIniData->dwAdmFileListNum++;

		pPropertiesToDelete[dwPropertiesToDelNum] = pCurrProperty;
		dwPropertiesToDelNum++;
	}

	for (DWORD i = 0; i < dwPropertiesToDelNum; ++i)
	{
		if (RemovePropertyInSection(pGenericIniSection, pPropertiesToDelete[i]) == FALSE)
		{
			DEBUG_LOG(D_ERROR, "Unable to remove property from section.\r\nExiting now...");
			DoExit(D_ERROR);
		}
	}

	return TRUE;
}