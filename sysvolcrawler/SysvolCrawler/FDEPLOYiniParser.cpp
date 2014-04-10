/**************************************************
* SysvolCrawler - FDEPLOYiniParser.cpp
* AUTHOR: Luc Delsalle	
*
* Parsing engine for folder deployment file
*	
* ANSSI/COSSI/DTO/BAI - 2014
***************************************************/

#include "FDEPLOYiniParser.h"

VOID RegisterFdeployIniParser(_Inout_ PPARSER_IDENTIFIER *pParserID)
{
	*pParserID = (PPARSER_IDENTIFIER) HeapAlloc(hCrawlerHeap, HEAP_ZERO_MEMORY, sizeof (PARSER_IDENTIFIER));
	if (!pParserID)
	{
		DEBUG_LOG(D_ERROR, "Unable to allocate PARSER_IDENTIFIER structure.\r\nExiting now...");
		DoExit(D_ERROR);
	}

	(*pParserID)->tParserName = FDEPLOYINI_PARSER_NAME;
	(*pParserID)->tFileMatchingRegExp = FDEPLOYINI_MATCHING_FILE_REGEXP;
	(*pParserID)->tFolderMatchingRegExp = NULL;
	(*pParserID)->pParserEntryPoint = ParseFdeployIniFile;
}

BOOL ParseFdeployIniFile(_In_ PTCHAR tFilePath)
{
	HANDLE hFdeployIniFile = INVALID_HANDLE_VALUE;
	PFDEPLOYINI_FILE_DATA pFdeployIniFileData = NULL;
	PINI_FILE_DATA pGenericIniFileData = NULL;
	DWORD dwFileSize = 0, dwNumberOfBytesRead = 0;
	PBYTE pbINIRawDATA = NULL;
	BOOL bMemoryAreaMoved = FALSE;

	if (tFilePath == NULL)
	{
		DEBUG_LOG(D_ERROR, "FILEPATH pointer invalid.\r\nExiting now...");
		DoExit(D_ERROR);
	}
	DEBUG_LOG(D_MISC, "[FDEPLOY.INI] Now parsing %ws\r\n", tFilePath);

	hFdeployIniFile = CreateFile_s(tFilePath, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
	if (hFdeployIniFile == INVALID_HANDLE_VALUE)
	{
		DEBUG_LOG(D_ERROR, "Unable to open file %ws. ErrorCode: %d.\r\n", tFilePath, GetLastError());
		SetLastError(ERROR_ACCESS_DENIED);
		return FALSE;
	}

	dwFileSize = GetFileSize(hFdeployIniFile, NULL);
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

	if (!ReadFile(hFdeployIniFile, pbINIRawDATA, dwFileSize, &dwNumberOfBytesRead, NULL))
	{
		DEBUG_LOG(D_ERROR, "Unable to read file %ws. ErrorCode: %d.\r\n", tFilePath, GetLastError());
		return FALSE;
	}
	CloseHandle(hFdeployIniFile);

	if (IsIniFileWcharEncoded(pbINIRawDATA, dwNumberOfBytesRead) == FALSE)
	{
		PBYTE pbINIRawDATATmp = pbINIRawDATA;

		// FDEPLOY.ini is an ANSI file, we need to convert it to WCHAR
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
		// FDEPLOY.ini if a WCHAR file, we just need to skip BOM
		bMemoryAreaMoved = TRUE;
		pbINIRawDATA +=2;
	}

	// Parse file to build generic INI structure
	pGenericIniFileData = ParseIniFile((PWCHAR) pbINIRawDATA, dwNumberOfBytesRead, tFilePath);
	if (!pGenericIniFileData)
	{
		DEBUG_LOG(D_ERROR, "Unable to parse generic ini file : %ws.\r\nExiting now...", tFilePath);
		DoExit(D_ERROR);
	}

	// Create FDEPLOY structure
	pFdeployIniFileData = (PFDEPLOYINI_FILE_DATA) HeapAlloc(hCrawlerHeap, HEAP_ZERO_MEMORY, sizeof (FDEPLOYINI_FILE_DATA));
	if (pFdeployIniFileData == NULL)
	{
		DEBUG_LOG(D_ERROR, "pAdmFilesIniFileData pointer invalid.\r\nExiting now...");
		DoExit(D_ERROR);
	}
	pFdeployIniFileData->tFilePath = tFilePath;
	pFdeployIniFileData->dwFolderStatusNum = 0;
	pFdeployIniFileData->dwMyDocumentsRedirectionNum = 0;
	pFdeployIniFileData->dwMyPicturesRedirectionNum = 0;
	pFdeployIniFileData->dwAppDataRedirectionNum = 0;
	pFdeployIniFileData->dwDesktopRedirectionNum = 0;
	pFdeployIniFileData->dwStartMenuRedirectionNum = 0;
	pFdeployIniFileData->dwProgramsRedirectionNum = 0;
	pFdeployIniFileData->dwStartupRedirectionNum = 0;
	pFdeployIniFileData->dwNumberOfUnReferrencedSections = 0;

	FillFdeployIniMethods(pFdeployIniFileData, pGenericIniFileData);

	// Keep track of unknown section or property
	if (pGenericIniFileData->iNumberOfSection)
	{
		pFdeployIniFileData->dwNumberOfUnReferrencedSections = pGenericIniFileData->iNumberOfSection;
		for (DWORD i = 0; i < pFdeployIniFileData->dwNumberOfUnReferrencedSections; ++i)
			pFdeployIniFileData->pUnReferrencedSections[i] = pGenericIniFileData->pSections[i];
		pGenericIniFileData->iNumberOfSection = 0; 
	}

	// Call printers
	PrintFdeployIniDataHeader(pFdeployIniFileData->tFilePath);
	PrintData(pFdeployIniFileData);
	PrintFdeployIniDataFooter(pFdeployIniFileData->tFilePath);

	// Cleanup
	if (pbINIRawDATA)
	{
		if (bMemoryAreaMoved == TRUE)
			pbINIRawDATA -=2;
		HeapFree(hCrawlerHeap, NULL, pbINIRawDATA);
	}
	FreeFdeployIniFileData(pFdeployIniFileData);
	FreeIniFileData(pGenericIniFileData);
	return TRUE;
}

BOOL FillFdeployIniMethods(_Inout_ PFDEPLOYINI_FILE_DATA pFdeployIniFileData, _In_ PINI_FILE_DATA pGenericIniFileData)
{
	DWORD dwSectionsToDelNum = 0;
	PINI_SECTION_DATA pSectionsToDelete[MAX_INI_SECTIONS];

	if (!pFdeployIniFileData || !pGenericIniFileData)
	{
		DEBUG_LOG(D_ERROR, "FDEPLOYINI_FILE_DATA or INI_FILE_DATA pointer is invalid.\r\nExiting now...");
		DoExit(D_ERROR);
	}

	for (DWORD i = 0; i < pGenericIniFileData->iNumberOfSection; ++i)
	{
		BOOL bIsSectionFound = FALSE;
		PINI_SECTION_DATA pCurrSection = pGenericIniFileData->pSections[i];

		if (!pCurrSection)
			continue;

		if (_tcsstr(pCurrSection->tSectionName, FDEPLOY_STATUS_SECTION))
		{
			bIsSectionFound = TRUE;
			if (FillFolderStatusSection(pFdeployIniFileData, pCurrSection, i) == FALSE)
			{
				DEBUG_LOG(D_ERROR, "Unable to handle section %ws.\r\nExiting now...", pCurrSection->tSectionName);
				DoExit(D_ERROR);
			}
		}
		else if (_tcsstr(pCurrSection->tSectionName, FDEPLOY_MYDOCUMENTS_SECTION))
		{
			bIsSectionFound = TRUE;
			if (FillFolderRedirectionSection(pFdeployIniFileData, pCurrSection, i, FDEPLOY_MYDOCUMENTS_REDIRECTION_ID) == FALSE)
			{
				DEBUG_LOG(D_ERROR, "Unable to handle section %ws.\r\nExiting now...", pCurrSection->tSectionName);
				DoExit(D_ERROR);
			}
		}
		else if (_tcsstr(pCurrSection->tSectionName, FDEPLOY_MYPICTURES_SECTION))
		{
			bIsSectionFound = TRUE;
			if (FillFolderRedirectionSection(pFdeployIniFileData, pCurrSection, i, FDEPLOY_MYPICTURES_REDIRECTION_ID) == FALSE)
			{
				DEBUG_LOG(D_ERROR, "Unable to handle section %ws.\r\nExiting now...", pCurrSection->tSectionName);
				DoExit(D_ERROR);
			}
		}
		else if (_tcsstr(pCurrSection->tSectionName, FDEPLOY_APPDATA_SECTION))
		{
			bIsSectionFound = TRUE;
			if (FillFolderRedirectionSection(pFdeployIniFileData, pCurrSection, i, FDEPLOY_APPDATA_REDIRECTION_ID) == FALSE)
			{
				DEBUG_LOG(D_ERROR, "Unable to handle section %ws.\r\nExiting now...", pCurrSection->tSectionName);
				DoExit(D_ERROR);
			}
		}
		else if (_tcsstr(pCurrSection->tSectionName, FDEPLOY_DESKTOP_SECTION))
		{
			bIsSectionFound = TRUE;
			if (FillFolderRedirectionSection(pFdeployIniFileData, pCurrSection, i, FDEPLOY_DESKTOP_REDIRECTION_ID) == FALSE)
			{
				DEBUG_LOG(D_ERROR, "Unable to handle section %ws.\r\nExiting now...", pCurrSection->tSectionName);
				DoExit(D_ERROR);
			}
		}
		else if (_tcsstr(pCurrSection->tSectionName, FDEPLOY_STARTMENU_SECTION))
		{
			bIsSectionFound = TRUE;
			if (FillFolderRedirectionSection(pFdeployIniFileData, pCurrSection, i, FDEPLOY_STARTMENU_REDIRECTION_ID) == FALSE)
			{
				DEBUG_LOG(D_ERROR, "Unable to handle section %ws.\r\nExiting now...", pCurrSection->tSectionName);
				DoExit(D_ERROR);
			}
		}
		else if (_tcsstr(pCurrSection->tSectionName, FDEPLOY_PROGRAMS_SECTION))
		{
			bIsSectionFound = TRUE;
			if (FillFolderRedirectionSection(pFdeployIniFileData, pCurrSection, i, FDEPLOY_PROGRAMS_REDIRECTION_ID) == FALSE)
			{
				DEBUG_LOG(D_ERROR, "Unable to handle section %ws.\r\nExiting now...", pCurrSection->tSectionName);
				DoExit(D_ERROR);
			}
		}
		else if (_tcsstr(pCurrSection->tSectionName, FDEPLOY_STARTUP_SECTION))
		{
			bIsSectionFound = TRUE;
			if (FillFolderRedirectionSection(pFdeployIniFileData, pCurrSection, i, FDEPLOY_STARTUP_REDIRECTION_ID) == FALSE)
			{
				DEBUG_LOG(D_ERROR, "Unable to handle section %ws.\r\nExiting now...", pCurrSection->tSectionName);
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

BOOL FillFolderStatusSection(_Inout_ PFDEPLOYINI_FILE_DATA pFdeployIniFileData, _In_ PINI_SECTION_DATA pGenericIniSection, _In_ DWORD dwSectionNumb)
{
	DWORD dwPropertiesToDelNum = 0;
	PINI_PROPERTY_DATA pPropertiesToDelete[MAX_INI_PROPERTIES];

	if (!pFdeployIniFileData || !pGenericIniSection)
	{
		DEBUG_LOG(D_ERROR, "FDEPLOYINI_FILE_DATA, INI_SECTION_DATA pointer or section number is invalid.\r\nExiting now...");
		DoExit(D_ERROR);
	}

	for (DWORD i = 0; i < pGenericIniSection->iNumberOfProperty; ++i)
	{
		PINI_PROPERTY_DATA pCurrProperty = pGenericIniSection->pProperties[i];
		PFDEPLOYINI_FOLDER_STATUS pNewFolderStatus = NULL;

		pNewFolderStatus = (PFDEPLOYINI_FOLDER_STATUS) HeapAlloc(hCrawlerHeap, NULL, sizeof(FDEPLOYINI_FOLDER_STATUS));
		if (!pNewFolderStatus)
		{
			DEBUG_LOG(D_ERROR, "pNewFolderStatus pointer invalid.\r\nExiting now...");
			DoExit(D_ERROR);
		}
		pNewFolderStatus->dwStatus = 0;
		pNewFolderStatus->tTargetedFolder = NULL;

		if (pCurrProperty->tName)
		{
			DWORD dwPropertyLen = (DWORD) _tcslen(pCurrProperty->tName);

			pNewFolderStatus->tTargetedFolder = (PTCHAR) HeapAlloc(hCrawlerHeap, NULL, (dwPropertyLen + 1) * sizeof(WCHAR));
			if (!(pNewFolderStatus->tTargetedFolder))
			{
				DEBUG_LOG(D_ERROR, "tTargetedFolder pointer invalid.\r\nExiting now...");
				DoExit(D_ERROR);
			}
			if (memcpy_s((pNewFolderStatus->tTargetedFolder), sizeof (TCHAR) * dwPropertyLen, pCurrProperty->tName, sizeof (WCHAR) * dwPropertyLen))
			{
				DEBUG_LOG(D_ERROR, "Unable to extract folder target.\r\nExiting now...");
				DoExit(D_ERROR);
			}
			pNewFolderStatus->tTargetedFolder[dwPropertyLen] = TEXT('\0');
		}

		if (pCurrProperty->tValue)
		{
			DWORD dwFolderVersion = _tstoi(pCurrProperty->tValue);

			pNewFolderStatus->dwStatus = dwFolderVersion;
		}	

		pFdeployIniFileData->pFolderStatus[pFdeployIniFileData->dwFolderStatusNum] = pNewFolderStatus;
		pFdeployIniFileData->dwFolderStatusNum++;

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

BOOL FillFolderRedirectionSection(_Inout_ PFDEPLOYINI_FILE_DATA pFdeployIniFileData, _In_ PINI_SECTION_DATA pGenericIniSection, _In_ DWORD dwSectionNumb, _In_ FDEPLOY_REDIRECTION_ID dwRedirectionID)
{
	DWORD dwPropertiesToDelNum = 0;
	PINI_PROPERTY_DATA pPropertiesToDelete[MAX_INI_PROPERTIES];

	if (!pFdeployIniFileData || !pGenericIniSection)
	{
		DEBUG_LOG(D_ERROR, "FDEPLOYINI_FILE_DATA, INI_SECTION_DATA pointer or section number is invalid.\r\nExiting now...");
		DoExit(D_ERROR);
	}

	for (DWORD i = 0; i < pGenericIniSection->iNumberOfProperty; ++i)
	{
		PINI_PROPERTY_DATA pCurrProperty = pGenericIniSection->pProperties[i];
		PFDEPLOYINI_FOLDER_REDIRECTION pNewFolderRedirection = NULL;

		pNewFolderRedirection = (PFDEPLOYINI_FOLDER_REDIRECTION) HeapAlloc(hCrawlerHeap, NULL, sizeof(FDEPLOYINI_FOLDER_REDIRECTION));
		if (!pNewFolderRedirection)
		{
			DEBUG_LOG(D_ERROR, "pNewFolderRedirection pointer invalid.\r\nExiting now...");
			DoExit(D_ERROR);
		}
		pNewFolderRedirection->tTargetedSID = NULL;
		pNewFolderRedirection->tRedirectionPath = NULL;

		if (pCurrProperty->tName)
		{
			DWORD dwPropertyLen = (DWORD) _tcslen(pCurrProperty->tName);

			pNewFolderRedirection->tTargetedSID = (PTCHAR) HeapAlloc(hCrawlerHeap, NULL, (dwPropertyLen + 1) * sizeof(WCHAR));
			if (!(pNewFolderRedirection->tTargetedSID))
			{
				DEBUG_LOG(D_ERROR, "tTargetedSID pointer invalid.\r\nExiting now...");
				DoExit(D_ERROR);
			}
			if (memcpy_s((pNewFolderRedirection->tTargetedSID), sizeof (TCHAR) * dwPropertyLen, pCurrProperty->tName, sizeof (WCHAR) * dwPropertyLen))
			{
				DEBUG_LOG(D_ERROR, "Unable to extract targeted SID.\r\nExiting now...");
				DoExit(D_ERROR);
			}
			pNewFolderRedirection->tTargetedSID[dwPropertyLen] = TEXT('\0');
		}

		if (pCurrProperty->tValue)
		{
			DWORD dwPropertyLen = (DWORD) _tcslen(pCurrProperty->tValue);

			pNewFolderRedirection->tRedirectionPath = (PTCHAR) HeapAlloc(hCrawlerHeap, NULL, (dwPropertyLen + 1) * sizeof(WCHAR));
			if (!(pNewFolderRedirection->tRedirectionPath))
			{
				DEBUG_LOG(D_ERROR, "tRedirectionPath pointer invalid.\r\nExiting now...");
				DoExit(D_ERROR);
			}
			if (memcpy_s((pNewFolderRedirection->tRedirectionPath), sizeof (TCHAR) * dwPropertyLen, pCurrProperty->tValue, sizeof (WCHAR) * dwPropertyLen))
			{
				DEBUG_LOG(D_ERROR, "Unable to extract redirection path.\r\nExiting now...");
				DoExit(D_ERROR);
			}
			pNewFolderRedirection->tRedirectionPath[dwPropertyLen] = TEXT('\0');
		}	

		switch(dwRedirectionID)
		{
		case FDEPLOY_MYDOCUMENTS_REDIRECTION_ID:
			pFdeployIniFileData->pMyDocumentsRedirection[pFdeployIniFileData->dwMyDocumentsRedirectionNum] = pNewFolderRedirection;
			pFdeployIniFileData->dwMyDocumentsRedirectionNum++;
			break;
		case FDEPLOY_MYPICTURES_REDIRECTION_ID:
			pFdeployIniFileData->pMyPicturesRedirection[pFdeployIniFileData->dwMyPicturesRedirectionNum] = pNewFolderRedirection;
			pFdeployIniFileData->dwMyPicturesRedirectionNum++;
			break;
		case FDEPLOY_APPDATA_REDIRECTION_ID:
			pFdeployIniFileData->pAppdataRedirection[pFdeployIniFileData->dwAppDataRedirectionNum] = pNewFolderRedirection;
			pFdeployIniFileData->dwAppDataRedirectionNum++;
			break;
		case FDEPLOY_DESKTOP_REDIRECTION_ID:
			pFdeployIniFileData->pDesktopRedirection[pFdeployIniFileData->dwDesktopRedirectionNum] = pNewFolderRedirection;
			pFdeployIniFileData->dwDesktopRedirectionNum++;
			break;
		case FDEPLOY_STARTMENU_REDIRECTION_ID:
			pFdeployIniFileData->pStartMenuRedirection[pFdeployIniFileData->dwStartMenuRedirectionNum] = pNewFolderRedirection;
			pFdeployIniFileData->dwStartMenuRedirectionNum++;
			break;
		case FDEPLOY_PROGRAMS_REDIRECTION_ID:
			pFdeployIniFileData->pProgramsRedirection[pFdeployIniFileData->dwProgramsRedirectionNum] = pNewFolderRedirection;
			pFdeployIniFileData->dwProgramsRedirectionNum++;
			break;
		case FDEPLOY_STARTUP_REDIRECTION_ID:
			pFdeployIniFileData->pStartupRedirection[pFdeployIniFileData->dwStartupRedirectionNum] = pNewFolderRedirection;
			pFdeployIniFileData->dwStartupRedirectionNum++;
			break;
		default:
			DEBUG_LOG(D_ERROR, "Unable to identify FDEPLOY_REDIRECTION_ID.\r\nExiting now...");
			DoExit(D_ERROR);
			break;
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

BOOL FreeFdeployIniFileData(_Inout_ PFDEPLOYINI_FILE_DATA pFdeployIniData)
{
	if (pFdeployIniData == NULL)
	{
		DEBUG_LOG(D_ERROR, "FDEPLOYINI_FILE_DATA pointer invalid.\r\nExiting now...");
		DoExit(D_ERROR);
	}	

	for (DWORD i = 0; i < pFdeployIniData->dwFolderStatusNum; ++i)
	{
		PFDEPLOYINI_FOLDER_STATUS pFolderStatus = pFdeployIniData->pFolderStatus[i];

		if (!pFolderStatus)
			continue;

		if (pFolderStatus->tTargetedFolder)
		{
			HeapFree(hCrawlerHeap, NULL, pFolderStatus->tTargetedFolder);
			pFolderStatus->tTargetedFolder = NULL;
		}
		HeapFree(hCrawlerHeap, NULL, pFolderStatus);
	}

	for (DWORD i = 0; i < pFdeployIniData->dwMyDocumentsRedirectionNum; ++i)
	{
		PFDEPLOYINI_FOLDER_REDIRECTION pNewFolderRedirection = pFdeployIniData->pMyDocumentsRedirection[i];

		if (!pNewFolderRedirection)
			continue;

		if (pNewFolderRedirection->tRedirectionPath)
		{
			HeapFree(hCrawlerHeap, NULL, pNewFolderRedirection->tRedirectionPath);
			pNewFolderRedirection->tRedirectionPath = NULL;
		}
		if (pNewFolderRedirection->tTargetedSID)
		{
			HeapFree(hCrawlerHeap, NULL, pNewFolderRedirection->tTargetedSID);
			pNewFolderRedirection->tTargetedSID = NULL;
		}
		HeapFree(hCrawlerHeap, NULL, pNewFolderRedirection);
	}

	for (DWORD i = 0; i < pFdeployIniData->dwMyPicturesRedirectionNum; ++i)
	{
		PFDEPLOYINI_FOLDER_REDIRECTION pNewFolderRedirection = pFdeployIniData->pMyPicturesRedirection[i];

		if (!pNewFolderRedirection)
			continue;

		if (pNewFolderRedirection->tRedirectionPath)
		{
			HeapFree(hCrawlerHeap, NULL, pNewFolderRedirection->tRedirectionPath);
			pNewFolderRedirection->tRedirectionPath = NULL;
		}
		if (pNewFolderRedirection->tTargetedSID)
		{
			HeapFree(hCrawlerHeap, NULL, pNewFolderRedirection->tTargetedSID);
			pNewFolderRedirection->tTargetedSID = NULL;
		}
		HeapFree(hCrawlerHeap, NULL, pNewFolderRedirection);
	}

	for (DWORD i = 0; i < pFdeployIniData->dwAppDataRedirectionNum; ++i)
	{
		PFDEPLOYINI_FOLDER_REDIRECTION pNewFolderRedirection = pFdeployIniData->pAppdataRedirection[i];

		if (!pNewFolderRedirection)
			continue;

		if (pNewFolderRedirection->tRedirectionPath)
		{
			HeapFree(hCrawlerHeap, NULL, pNewFolderRedirection->tRedirectionPath);
			pNewFolderRedirection->tRedirectionPath = NULL;
		}
		if (pNewFolderRedirection->tTargetedSID)
		{
			HeapFree(hCrawlerHeap, NULL, pNewFolderRedirection->tTargetedSID);
			pNewFolderRedirection->tTargetedSID = NULL;
		}
		HeapFree(hCrawlerHeap, NULL, pNewFolderRedirection);
	}

	for (DWORD i = 0; i < pFdeployIniData->dwDesktopRedirectionNum; ++i)
	{
		PFDEPLOYINI_FOLDER_REDIRECTION pNewFolderRedirection = pFdeployIniData->pDesktopRedirection[i];

		if (!pNewFolderRedirection)
			continue;

		if (pNewFolderRedirection->tRedirectionPath)
		{
			HeapFree(hCrawlerHeap, NULL, pNewFolderRedirection->tRedirectionPath);
			pNewFolderRedirection->tRedirectionPath = NULL;
		}
		if (pNewFolderRedirection->tTargetedSID)
		{
			HeapFree(hCrawlerHeap, NULL, pNewFolderRedirection->tTargetedSID);
			pNewFolderRedirection->tTargetedSID = NULL;
		}
		HeapFree(hCrawlerHeap, NULL, pNewFolderRedirection);
	}

	for (DWORD i = 0; i < pFdeployIniData->dwStartMenuRedirectionNum; ++i)
	{
		PFDEPLOYINI_FOLDER_REDIRECTION pNewFolderRedirection = pFdeployIniData->pStartMenuRedirection[i];

		if (!pNewFolderRedirection)
			continue;

		if (pNewFolderRedirection->tRedirectionPath)
		{
			HeapFree(hCrawlerHeap, NULL, pNewFolderRedirection->tRedirectionPath);
			pNewFolderRedirection->tRedirectionPath = NULL;
		}
		if (pNewFolderRedirection->tTargetedSID)
		{
			HeapFree(hCrawlerHeap, NULL, pNewFolderRedirection->tTargetedSID);
			pNewFolderRedirection->tTargetedSID = NULL;
		}
		HeapFree(hCrawlerHeap, NULL, pNewFolderRedirection);
	}

	for (DWORD i = 0; i < pFdeployIniData->dwProgramsRedirectionNum; ++i)
	{
		PFDEPLOYINI_FOLDER_REDIRECTION pNewFolderRedirection = pFdeployIniData->pProgramsRedirection[i];

		if (!pNewFolderRedirection)
			continue;

		if (pNewFolderRedirection->tRedirectionPath)
		{
			HeapFree(hCrawlerHeap, NULL, pNewFolderRedirection->tRedirectionPath);
			pNewFolderRedirection->tRedirectionPath = NULL;
		}
		if (pNewFolderRedirection->tTargetedSID)
		{
			HeapFree(hCrawlerHeap, NULL, pNewFolderRedirection->tTargetedSID);
			pNewFolderRedirection->tTargetedSID = NULL;
		}
		HeapFree(hCrawlerHeap, NULL, pNewFolderRedirection);
	}

	for (DWORD i = 0; i < pFdeployIniData->dwStartupRedirectionNum; ++i)
	{
		PFDEPLOYINI_FOLDER_REDIRECTION pNewFolderRedirection = pFdeployIniData->pStartupRedirection[i];

		if (!pNewFolderRedirection)
			continue;

		if (pNewFolderRedirection->tRedirectionPath)
		{
			HeapFree(hCrawlerHeap, NULL, pNewFolderRedirection->tRedirectionPath);
			pNewFolderRedirection->tRedirectionPath = NULL;
		}
		if (pNewFolderRedirection->tTargetedSID)
		{
			HeapFree(hCrawlerHeap, NULL, pNewFolderRedirection->tTargetedSID);
			pNewFolderRedirection->tTargetedSID = NULL;
		}
		HeapFree(hCrawlerHeap, NULL, pNewFolderRedirection);
	}

	for (DWORD i = 0; i < pFdeployIniData->dwNumberOfUnReferrencedSections; ++i)
	{
		PINI_SECTION_DATA pSectionToDelete = pFdeployIniData->pUnReferrencedSections[i];

		if (pSectionToDelete)
			FreeSectionData(pSectionToDelete);
	}

	HeapFree(hCrawlerHeap, NULL, pFdeployIniData);
	pFdeployIniData = NULL;
	return TRUE;
}