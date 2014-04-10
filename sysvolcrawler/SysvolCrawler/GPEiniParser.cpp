/**************************************************
* SysvolCrawler - GPEiniParser.cpp
* AUTHOR: Luc Delsalle	
*
* Parsing engine for GPE.ini file
*	
* ANSSI/COSSI/DTO/BAI - 2014
***************************************************/

#include "GPEiniParser.h"

VOID RegisterGpeIniParser(_Inout_ PPARSER_IDENTIFIER *pParserID)
{
	*pParserID = (PPARSER_IDENTIFIER) HeapAlloc(hCrawlerHeap, HEAP_ZERO_MEMORY, sizeof (PARSER_IDENTIFIER));
	if (!pParserID)
	{
		DEBUG_LOG(D_ERROR, "Unable to allocate PARSER_IDENTIFIER structure.\r\nExiting now...");
		DoExit(D_ERROR);
	}

	(*pParserID)->tParserName = GPEINI_PARSER_NAME;
	(*pParserID)->tFileMatchingRegExp = GPEINI_MATCHING_FILE_REGEXP;
	(*pParserID)->tFolderMatchingRegExp = NULL;
	(*pParserID)->pParserEntryPoint = ParseGpeIniFile;
}

BOOL ParseGpeIniFile(_In_ PTCHAR tFilePath)
{
	HANDLE hGpeIniFile = INVALID_HANDLE_VALUE;
	PINI_FILE_DATA pGenericIniFileData = NULL;
	PGPEINI_FILE_DATA pGpeIniFileData = NULL;
	PBYTE pbINIRawDATA = NULL;
	DWORD dwFileSize = 0, dwNumberOfBytesRead = 0;
	BOOL bMemoryAreaMoved = FALSE;

	if (tFilePath == NULL)
	{
		DEBUG_LOG(D_ERROR, "FILEPATH pointer invalid.\r\nExiting now...");
		DoExit(D_ERROR);
	}
	DEBUG_LOG(D_MISC, "[GPE.INI] Now parsing %ws\r\n", tFilePath);

	hGpeIniFile = CreateFile_s(tFilePath, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
	if (hGpeIniFile == INVALID_HANDLE_VALUE)
	{
		DEBUG_LOG(D_ERROR, "Unable to open file %ws. ErrorCode: %d.\r\n", tFilePath, GetLastError());
		SetLastError(ERROR_ACCESS_DENIED);
		return FALSE;
	}

	dwFileSize = GetFileSize(hGpeIniFile, NULL);
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

	if (!ReadFile(hGpeIniFile, pbINIRawDATA, dwFileSize, &dwNumberOfBytesRead, NULL))
	{
		DEBUG_LOG(D_ERROR, "Unable to read file %ws. ErrorCode: %d.\r\n", tFilePath, GetLastError());
		return FALSE;
	}
	CloseHandle(hGpeIniFile);

	if (IsIniFileWcharEncoded(pbINIRawDATA, dwNumberOfBytesRead) == FALSE)
	{
		PBYTE pbINIRawDATATmp = pbINIRawDATA;
		// GPE.ini is an ANSI file, we need to convert it to WCHAR
		pbINIRawDATA = (PBYTE) CStrToPtchar(pbINIRawDATA, dwNumberOfBytesRead);
		dwNumberOfBytesRead *= sizeof (WCHAR);
		if ((pbINIRawDATATmp != pbINIRawDATA) && (pbINIRawDATATmp))
			HeapFree(hCrawlerHeap, NULL, pbINIRawDATATmp);
		if (!pbINIRawDATA)
		{
			DEBUG_LOG(D_ERROR, "Unable to convert file %ws to WideChar.\r\n", tFilePath);
			return FALSE;
		}
	}
	else
	{
		// GPE.ini if a WCHAR file, we just need to skip BOM
		bMemoryAreaMoved = TRUE;
		pbINIRawDATA +=2;
	}

	pGenericIniFileData = ParseIniFile((PWCHAR) pbINIRawDATA, dwNumberOfBytesRead, tFilePath);
	if (!pGenericIniFileData)
	{
		DEBUG_LOG(D_ERROR, "Unable to parse generic ini file : %ws.\r\nExiting now...", tFilePath);
		DoExit(D_ERROR);
	}

	pGpeIniFileData = (PGPEINI_FILE_DATA) HeapAlloc(hCrawlerHeap, HEAP_ZERO_MEMORY, sizeof (GPEINI_FILE_DATA));
	if (pGpeIniFileData == NULL)
	{
		DEBUG_LOG(D_ERROR, "pGptIniFileData pointer invalid.\r\nExiting now...");
		DoExit(D_ERROR);
	}
	pGpeIniFileData->tFilePath = tFilePath;
	pGpeIniFileData->dwMachineExtensionVersionsNum = 0;
	pGpeIniFileData->dwUserExtensionVersionsNum = 0;
	pGpeIniFileData->dwNumberOfUnReferrencedSections = 0;


	// Fill PGPEINI_FILE_DATA with GPE.ini data and delete generic section structure
	if (FillGpeIniAttributes(pGpeIniFileData, pGenericIniFileData) == FALSE)
	{
		DEBUG_LOG(D_ERROR, "Error during GPE.ini parsing.\r\nExiting now...");
		DoExit(D_ERROR);
	}

	// Keep track of unknown section or property
	if (pGenericIniFileData->iNumberOfSection)
	{
		pGpeIniFileData->dwNumberOfUnReferrencedSections = pGenericIniFileData->iNumberOfSection;
		for (DWORD i = 0; i < pGpeIniFileData->dwNumberOfUnReferrencedSections; ++i)
			pGpeIniFileData->pUnReferrencedSections[i] = pGenericIniFileData->pSections[i];
		pGenericIniFileData->iNumberOfSection = 0; 
	}

	// Call printers
	PrintGpeIniDataHeader(pGpeIniFileData->tFilePath);
	PrintData(pGpeIniFileData);
	PrintGpeIniDataFooter(pGpeIniFileData->tFilePath);

	// Cleanup
	if (pbINIRawDATA)
	{
		if (bMemoryAreaMoved == TRUE)
			pbINIRawDATA -=2;
		HeapFree(hCrawlerHeap, NULL, pbINIRawDATA);
	}
	FreeGpeIniFileData(pGpeIniFileData);
	FreeIniFileData(pGenericIniFileData);
	return TRUE;
}

BOOL FreeGpeIniFileData(_Inout_ PGPEINI_FILE_DATA pGpeIniFileData)
{
	if (!pGpeIniFileData)
	{
		DEBUG_LOG(D_ERROR, "GPEINI_FILE_DATA pointer is invalid.\r\nExiting now...");
		DoExit(D_ERROR);
	}

	for (DWORD i = 0; i < pGpeIniFileData->dwMachineExtensionVersionsNum; ++i)
	{
		PGPEINI_CSE_DATA pCurrCseData = pGpeIniFileData->pMachineExtensionVersions[i];

		if (!pCurrCseData)
			continue;

		for (DWORD j = 0; j < pCurrCseData->dwCSEValuesNum; ++j)
			HeapFree(hCrawlerHeap, NULL, pCurrCseData->pCSEValues[j]);
		HeapFree(hCrawlerHeap, NULL, pCurrCseData);
	}

	for (DWORD i = 0; i < pGpeIniFileData->dwUserExtensionVersionsNum; ++i)
	{
		PGPEINI_CSE_DATA pCurrCseData = pGpeIniFileData->pUserExtensionVersions[i];

		if (!pCurrCseData)
			continue;

		for (DWORD j = 0; j < pCurrCseData->dwCSEValuesNum; ++j)
			HeapFree(hCrawlerHeap, NULL, pCurrCseData->pCSEValues[j]);
		HeapFree(hCrawlerHeap, NULL, pCurrCseData);
	}

	for (DWORD i = 0; i < pGpeIniFileData->dwNumberOfUnReferrencedSections; ++i)
	{
		PINI_SECTION_DATA pSectionToDelete = pGpeIniFileData->pUnReferrencedSections[i];

		if (pSectionToDelete)
			FreeSectionData(pSectionToDelete);
	}

	if (pGpeIniFileData)
		HeapFree(hCrawlerHeap, NULL, pGpeIniFileData);
	return TRUE;
}

BOOL FillGpeIniAttributes(_Inout_ PGPEINI_FILE_DATA pGpeIniFileData, _In_ PINI_FILE_DATA pGenericIniFileData)
{
	DWORD dwSectionsToDelNum = 0;
	PINI_SECTION_DATA pSectionsToDelete[MAX_INI_SECTIONS];

	if (!pGpeIniFileData || !pGenericIniFileData)
	{
		DEBUG_LOG(D_ERROR, "GPEINI_FILE_DATA or INI_FILE_DATA pointer is invalid.\r\nExiting now...");
		DoExit(D_ERROR);
	}

	for (DWORD i = 0; i < pGenericIniFileData->iNumberOfSection; ++i)
	{
		BOOL bIsSectionFound = FALSE;
		PINI_SECTION_DATA pCurrSection = pGenericIniFileData->pSections[i];

		if (!pCurrSection)
			continue;

		if (_tcsstr(pCurrSection->tSectionName, GPE_GENERAL_SECTION))
		{
			bIsSectionFound = TRUE;
			if (FillExtensionAttributes(pGpeIniFileData, pCurrSection, i) == FALSE)
			{
				DEBUG_LOG(D_ERROR, "Unable to handle properties for section %ws.\r\nExiting now...", pCurrSection->tSectionName);
				DoExit(D_ERROR);
			}
		}

		if (bIsSectionFound == TRUE)
		{
			if (IsSectionEmpty(pCurrSection))
			{
				pSectionsToDelete[dwSectionsToDelNum] = pCurrSection;
				dwSectionsToDelNum++;
			}
		}
	}

	// Only delete section if has been entirely handled
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

BOOL FillExtensionAttributes(_Inout_ PGPEINI_FILE_DATA pGpeIniFileData, _In_ PINI_SECTION_DATA pGenericIniSection, _In_ DWORD dwSectionNumb)
{
	DWORD dwPropertiesToDelNum = 0;
	PINI_PROPERTY_DATA pPropertiesToDelete[MAX_INI_PROPERTIES];
	BOOL bIsMachineExtensionAlreadyFound = FALSE, bIsUserExtensionAlreadyFound = FALSE;

	if (!pGpeIniFileData || !pGenericIniSection)
	{
		DEBUG_LOG(D_ERROR, "SCRIPTSINI_FILE_DATA, INI_SECTION_DATA pointer or section number is invalid.\r\nExiting now...");
		DoExit(D_ERROR);
	}

	for (DWORD i = 0; i < pGenericIniSection->iNumberOfProperty; ++i)
	{
		PINI_PROPERTY_DATA pCurrProperty = pGenericIniSection->pProperties[i];
		

		if (_tcsstr(pCurrProperty->tName, GPE_MACHINE_EXTENSION_VERSION))
		{
			DWORD dwPropertyLen = (DWORD) _tcslen(pCurrProperty->tValue), dwIndex = 0;
			PTCHAR tExtractedCSE =  NULL;

			if (bIsMachineExtensionAlreadyFound || (dwPropertyLen == 0))
				continue;
			bIsMachineExtensionAlreadyFound = TRUE;

			do
			{
				tExtractedCSE = ExtractCSEFromProperty(pCurrProperty->tValue, dwPropertyLen, &dwIndex);
				if (tExtractedCSE)
				{
					PGPEINI_CSE_DATA pCseData = (PGPEINI_CSE_DATA) HeapAlloc(hCrawlerHeap, NULL, sizeof(GPEINI_CSE_DATA));

					if (!pCseData)
					{
						DEBUG_LOG(D_ERROR, "pCseData pointer invalid.\r\nExiting now...");
						DoExit(D_ERROR);
					}
					pCseData->dwCSEValuesNum = 0;
					FillCSEAttributes(pCseData, tExtractedCSE);

					pGpeIniFileData->pMachineExtensionVersions[pGpeIniFileData->dwMachineExtensionVersionsNum] = pCseData;
					pGpeIniFileData->dwMachineExtensionVersionsNum++;
					HeapFree(hCrawlerHeap, NULL, tExtractedCSE);
				}
			}
			while (tExtractedCSE);
		}
		
		if (_tcsstr(pCurrProperty->tName, GPE_USER_EXTENSION_VERSION))
		{
			DWORD dwPropertyLen = (DWORD) _tcslen(pCurrProperty->tValue), dwIndex = 0;
			PTCHAR tExtractedCSE =  NULL;

			if (bIsUserExtensionAlreadyFound || (dwPropertyLen == 0))
				continue;
			bIsUserExtensionAlreadyFound = TRUE;
		
			do
			{
				tExtractedCSE = ExtractCSEFromProperty(pCurrProperty->tValue, dwPropertyLen, &dwIndex);
				if (tExtractedCSE)
				{
					PGPEINI_CSE_DATA pCseData = (PGPEINI_CSE_DATA) HeapAlloc(hCrawlerHeap, NULL, sizeof(GPEINI_CSE_DATA));

					if (!pCseData)
					{
						DEBUG_LOG(D_ERROR, "pCseData pointer invalid.\r\nExiting now...");
						DoExit(D_ERROR);
					}
					pCseData->dwCSEValuesNum = 0;
					FillCSEAttributes(pCseData, tExtractedCSE);

					pGpeIniFileData->pUserExtensionVersions[pGpeIniFileData->dwUserExtensionVersionsNum] = pCseData;
					pGpeIniFileData->dwUserExtensionVersionsNum++;
					HeapFree(hCrawlerHeap, NULL, tExtractedCSE);
				}
			}
			while (tExtractedCSE);
		}

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

BOOL FillCSEAttributes(_Inout_ PGPEINI_CSE_DATA pCseData, _In_ PTCHAR tRawCSEAttributes)
{
	PTCHAR tExtractedGUID = NULL, tExtractedID = NULL;
	DWORD dwRawCSELen = 0, dwIndex = 0;

	if (!pCseData || !tRawCSEAttributes)
	{
		DEBUG_LOG(D_ERROR, "GPEINI_CSE_DATA or extracted cse is invalid.\r\nExiting now...");
		DoExit(D_ERROR);
	}
	dwRawCSELen = (DWORD) _tcslen(tRawCSEAttributes);

	do
	{
		tExtractedGUID = ExtractCSEValuesFromProperty(tRawCSEAttributes, dwRawCSELen, &dwIndex);
		if (tExtractedGUID)
		{
			pCseData->pCSEValues[pCseData->dwCSEValuesNum]= tExtractedGUID;
			pCseData->dwCSEValuesNum++;
		}

	}
	while (tExtractedGUID);

	tExtractedID = ExtractCSEIdFromProperty(tRawCSEAttributes);
	if (!tExtractedID)
	{
		DEBUG_LOG(D_ERROR, "Unable to extract ID from GPE.ini action.\r\nExiting now...");
		DoExit(D_ERROR);
	}
	
	pCseData->dwCSEID  = _tstoi(tExtractedID);
	if (!(pCseData->dwCSEID))
	{
		DEBUG_LOG(D_ERROR, "Unable to convert ID from GPE.ini action.\r\nExiting now...");
		DoExit(D_ERROR);
	}
	HeapFree(hCrawlerHeap, NULL, tExtractedID);

	return TRUE;
}

PTCHAR ExtractCSEFromProperty(_In_ PTCHAR tProperty, _In_ DWORD dwPropertyLen, _In_ PDWORD pdwIndex)
{
	DWORD dwCurrentIndex = 0, dwStartIndex = 0, dwNewIndex = 0;
	BOOL bBeginGuidFound = FALSE;
	PTCHAR tExtractedCSE = NULL;

	if (!tProperty || !pdwIndex)
	{
		DEBUG_LOG(D_ERROR, "INI Property or index invalid.\r\nExiting now...");
		DoExit(D_ERROR);
	}
	dwCurrentIndex = *pdwIndex;

	for (DWORD i = dwCurrentIndex; i < dwPropertyLen; ++i)
	{
		if (tProperty[i] == TEXT('['))
		{
			bBeginGuidFound = TRUE;
			dwStartIndex = i;
		}

		if ((tProperty[i] == TEXT(']')) && (bBeginGuidFound == TRUE))
		{
			DWORD dwGUIDLen = i - dwCurrentIndex + 1;

			tExtractedCSE = (PTCHAR) HeapAlloc(hCrawlerHeap, NULL, (dwGUIDLen + 1) * sizeof(TCHAR));
			if (!tExtractedCSE)
			{
				DEBUG_LOG(D_ERROR, "tExtractedGUID pointer invalid.\r\nExiting now...");
				DoExit(D_ERROR);
			}
			if (memcpy_s(tExtractedCSE, sizeof (TCHAR) * dwGUIDLen, (PTCHAR) (tProperty + dwStartIndex), sizeof (TCHAR) * dwGUIDLen))
			{
				DEBUG_LOG(D_ERROR, "Unable to extract CSE.\r\nExiting now...");
				DoExit(D_ERROR);
			}
			tExtractedCSE[dwGUIDLen] = TEXT('\0');
			dwNewIndex = dwCurrentIndex + i + 1;
			break;
		}
	}
	*pdwIndex = dwNewIndex;
	return tExtractedCSE;
}

PTCHAR ExtractCSEValuesFromProperty(_In_ PTCHAR tProperty, _In_ DWORD dwPropertyLen, _In_ PDWORD pdwIndex)
{
	DWORD dwCurrentIndex = 0, dwStartIndex = 0, dwNewIndex = 0;
	BOOL bBeginGuidFound = FALSE;
	PTCHAR tExtractedGUID = NULL;

	if (!tProperty || !pdwIndex || tProperty[0] != TEXT('[') || tProperty[dwPropertyLen - 1] != TEXT(']'))
	{
		DEBUG_LOG(D_ERROR, "INI Property or index invalid.\r\nExiting now...");
		DoExit(D_ERROR);
	}
	dwCurrentIndex = *pdwIndex;

	for (DWORD i = dwCurrentIndex; i < dwPropertyLen; ++i)
	{
		if (tProperty[i] == TEXT('{'))
		{
			bBeginGuidFound = TRUE;
			dwStartIndex = i;
		}

		if ((tProperty[i] == TEXT('}')) && (bBeginGuidFound == TRUE))
		{
			DWORD dwGUIDLen = i - dwCurrentIndex + 1;

			tExtractedGUID = (PTCHAR) HeapAlloc(hCrawlerHeap, NULL, (dwGUIDLen + 1) * sizeof(TCHAR));
			if (!tExtractedGUID)
			{
				DEBUG_LOG(D_ERROR, "tExtractedGUID pointer invalid.\r\nExiting now...");
				DoExit(D_ERROR);
			}
			if (memcpy_s(tExtractedGUID, sizeof (TCHAR) * dwGUIDLen, (PTCHAR) (tProperty + dwStartIndex), sizeof (TCHAR) * dwGUIDLen))
			{
				DEBUG_LOG(D_ERROR, "Unable to extract GUID.\r\nExiting now...");
				DoExit(D_ERROR);
			}
			tExtractedGUID[dwGUIDLen - 1] = TEXT('\0');
			dwNewIndex = dwCurrentIndex + i + 1;
			break;
		}
	}
	*pdwIndex = dwNewIndex;
	return tExtractedGUID;
}

PTCHAR ExtractCSEIdFromProperty(_In_ PTCHAR tProperty)
{
	PTCHAR tExtractedPattern = 0;
	DWORD dwIndex = 0;
	PTCHAR tExtractedID = NULL;

	if (!tProperty)
	{
		DEBUG_LOG(D_ERROR, "INI Property.\r\nExiting now...");
		DoExit(D_ERROR);
	}

	tExtractedPattern = _tcsstr(tProperty, TEXT("}:"));
	if (!tExtractedPattern)
		return NULL;

	tExtractedPattern += 2;
	while (tExtractedPattern)
	{
		if (tExtractedPattern[dwIndex] == TEXT(']'))
		{
			DWORD dwIDLen = dwIndex;

			tExtractedID = (PTCHAR) HeapAlloc(hCrawlerHeap, NULL, (dwIDLen + 1) * sizeof(TCHAR));
			if (!tExtractedID)
			{
				DEBUG_LOG(D_ERROR, "tExtractedID pointer invalid.\r\nExiting now...");
				DoExit(D_ERROR);
			}
			if (memcpy_s(tExtractedID, sizeof (TCHAR) * dwIDLen, tExtractedPattern, sizeof (TCHAR) * dwIDLen))
			{
				DEBUG_LOG(D_ERROR, "Unable to extract ID.\r\nExiting now...");
				DoExit(D_ERROR);
			}
			tExtractedID[dwIDLen] = TEXT('\0');
			break;
		}
		++dwIndex;
	}
	return tExtractedID;
}