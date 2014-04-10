/**************************************************
* SysvolCrawler - SCRIPTSiniParser.cpp
* AUTHOR: Luc Delsalle	
*
* Parsing engine for scripts.ini file
*
* ANSSI/COSSI/DTO/BAI - 2014
***************************************************/

#include "SCRIPTSiniParser.h"

VOID RegisterScriptsIniParser(_Inout_ PPARSER_IDENTIFIER *pParserID)
{
	*pParserID = (PPARSER_IDENTIFIER) HeapAlloc(hCrawlerHeap, HEAP_ZERO_MEMORY, sizeof (PARSER_IDENTIFIER));
	if (!pParserID)
	{
		DEBUG_LOG(D_ERROR, "Unable to allocate PARSER_IDENTIFIER structure.\r\nExiting now...");
		DoExit(D_ERROR);
	}

	(*pParserID)->tParserName = SCRIPTSINI_PARSER_NAME;
	(*pParserID)->tFileMatchingRegExp = SCRIPTSINI_MATCHING_FILE_REGEXP;
	(*pParserID)->tFolderMatchingRegExp = NULL;
	(*pParserID)->pParserEntryPoint = ParseScriptsIniFile;
}

BOOL ParseScriptsIniFile(_In_ PTCHAR tFilePath)
{
	HANDLE hScriptsIniFile = INVALID_HANDLE_VALUE;
	PSCRIPTSINI_FILE_DATA pScriptsIniFileData = NULL;
	PINI_FILE_DATA pGenericIniFileData = NULL;
	DWORD dwFileSize = 0, dwNumberOfBytesRead = 0;
	PBYTE pbINIRawDATA = NULL;
	BOOL bMemoryAreaMoved = FALSE;

	if (tFilePath == NULL)
	{
		DEBUG_LOG(D_ERROR, "FILEPATH pointer invalid.\r\nExiting now...");
		DoExit(D_ERROR);
	}
	DEBUG_LOG(D_MISC, "[SCRIPTS.INI] Now parsing %ws\r\n", tFilePath);

	hScriptsIniFile = CreateFile_s(tFilePath, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
	if (hScriptsIniFile == INVALID_HANDLE_VALUE)
	{
		DEBUG_LOG(D_ERROR, "Unable to open file %ws. ErrorCode: %d.\r\n", tFilePath, GetLastError());
		SetLastError(ERROR_ACCESS_DENIED);
		return FALSE;
	}

	dwFileSize = GetFileSize(hScriptsIniFile, NULL);
	if (dwFileSize == INVALID_FILE_SIZE)
	{
		DEBUG_LOG(D_ERROR, "Error during reading FileSize for %ws.\r\nExiting now...", tFilePath);
		DoExit(D_ERROR);
	}
	else if (dwFileSize == 0)
	{
		return TRUE;
	}

	pbINIRawDATA = (PBYTE) HeapAlloc(hCrawlerHeap, HEAP_ZERO_MEMORY, sizeof (DWORD) * dwFileSize);
	if (pbINIRawDATA == NULL)
	{
		DEBUG_LOG(D_ERROR, "pbINIRawDATA pointer invalid.\r\nExiting now...");
		DoExit(D_ERROR);
	}

	if (!ReadFile(hScriptsIniFile, pbINIRawDATA, dwFileSize, &dwNumberOfBytesRead, NULL))
	{
		DEBUG_LOG(D_ERROR, "Unable to read file %ws. ErrorCode: %d.\r\n", tFilePath, GetLastError());
		return FALSE;
	}
	CloseHandle(hScriptsIniFile);

	if (IsIniFileWcharEncoded(pbINIRawDATA, dwNumberOfBytesRead) == FALSE)
	{
		PBYTE pbINIRawDATATmp = pbINIRawDATA;

		// SCRIPTS.ini is an ANSI file, we need to convert it to WCHAR
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
		// SCRIPTS.ini if a WCHAR file, we just need to skip BOM
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

	// Build scripts.ini structure
	pScriptsIniFileData = (PSCRIPTSINI_FILE_DATA) HeapAlloc(hCrawlerHeap, HEAP_ZERO_MEMORY, sizeof (SCRIPTSINI_FILE_DATA));
	if (pScriptsIniFileData == NULL)
	{
		DEBUG_LOG(D_ERROR, "pScriptsIniFileData pointer invalid.\r\nExiting now...");
		DoExit(D_ERROR);
	}
	pScriptsIniFileData->tFilePath = tFilePath;
	pScriptsIniFileData->dwLogoffScriptNum = 0;
	pScriptsIniFileData->dwLogonScriptNum = 0;
	pScriptsIniFileData->dwStartupScriptNum = 0;
	pScriptsIniFileData->dwShutdownScriptNum = 0;
	pScriptsIniFileData->dwNumberOfUnReferrencedSections = 0;

	// Fill PSCRIPTSINI_FILE_DATA following scripts.ini data and delete generic structure data
	FillScriptsIniMethods(pScriptsIniFileData, pGenericIniFileData);

	// Keep track of unknown section or property
	if (pGenericIniFileData->iNumberOfSection)
	{
		pScriptsIniFileData->dwNumberOfUnReferrencedSections = pGenericIniFileData->iNumberOfSection;
		for (DWORD i = 0; i < pScriptsIniFileData->dwNumberOfUnReferrencedSections; ++i)
			pScriptsIniFileData->pUnReferrencedSections[i] = pGenericIniFileData->pSections[i];
		pGenericIniFileData->iNumberOfSection = 0; 
	}

	// Call printers
	PrintScriptsIniDataHeader(pScriptsIniFileData->tFilePath);
	PrintData(pScriptsIniFileData);
	PrintScriptsIniDataFooter(pScriptsIniFileData->tFilePath);

	// Cleanup
	if (pbINIRawDATA)
	{
		if (bMemoryAreaMoved == TRUE)
			pbINIRawDATA -=2;
		HeapFree(hCrawlerHeap, NULL, pbINIRawDATA);
	}
	FreeScriptsIniFileData(pScriptsIniFileData);
	FreeIniFileData(pGenericIniFileData);
	return TRUE;
}

BOOL FillScriptsIniMethods(_Inout_ PSCRIPTSINI_FILE_DATA pScriptsIniFileData, _In_ PINI_FILE_DATA pGenericIniFileData)
{
	DWORD dwSectionsToDelNum = 0;
	PINI_SECTION_DATA pSectionsToDelete[MAX_INI_SECTIONS];

	if (!pScriptsIniFileData || !pGenericIniFileData)
	{
		DEBUG_LOG(D_ERROR, "SCRIPTSINI_FILE_DATA or INI_FILE_DATA pointer is invalid.\r\nExiting now...");
		DoExit(D_ERROR);
	}

	for (DWORD i = 0; i < pGenericIniFileData->iNumberOfSection; ++i)
	{
		BOOL bIsSectionFound = FALSE;
		PINI_SECTION_DATA pCurrSection = pGenericIniFileData->pSections[i];

		if (!pCurrSection)
			continue;

		if (_tcsstr(pCurrSection->tSectionName, SCRIPTS_LOGON_SECTION))
		{
			bIsSectionFound = TRUE;
			if (FillScriptsIniMethodsActions(pScriptsIniFileData, pCurrSection, i, SCRIPTS_LOGON_SECTION_ID) == FALSE)
			{
				DEBUG_LOG(D_ERROR, "Unable to handle properties for section %ws.\r\nExiting now...", pCurrSection->tSectionName);
				DoExit(D_ERROR);
			}
		}
		else if (_tcsstr(pCurrSection->tSectionName, SCRIPTS_LOGOFF_SECTION))
		{
			bIsSectionFound = TRUE;
			if (FillScriptsIniMethodsActions(pScriptsIniFileData, pCurrSection, i, SCRIPTS_LOGOFF_SECTION_ID) == FALSE)
			{
				DEBUG_LOG(D_ERROR, "Unable to handle properties for section %ws.\r\nExiting now...", pCurrSection->tSectionName);
				DoExit(D_ERROR);
			}
		}
		else if (_tcsstr(pCurrSection->tSectionName, SCRIPTS_STARTUP_SECTION))
		{
			bIsSectionFound = TRUE;
			if (FillScriptsIniMethodsActions(pScriptsIniFileData, pCurrSection, i, SCRIPTS_STARTUP_SECTION_ID) == FALSE)
			{
				DEBUG_LOG(D_ERROR, "Unable to handle properties for section %ws.\r\nExiting now...", pCurrSection->tSectionName);
				DoExit(D_ERROR);
			}
		}
		else if (_tcsstr(pCurrSection->tSectionName, SCRIPTS_SHUTDOWN_SECTION))
		{
			bIsSectionFound = TRUE;
			if (FillScriptsIniMethodsActions(pScriptsIniFileData, pCurrSection, i, SCRIPTS_SHUTDOWN_SECTION_ID) == FALSE)
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

BOOL FillScriptsIniMethodsActions(_Inout_ PSCRIPTSINI_FILE_DATA pScriptsIniFileData, _In_ PINI_SECTION_DATA pGenericIniSection, _In_ DWORD dwSectionNumb, _In_ SCRIPTS_SECTION_ID dwSectionID)
{
	DWORD dwPropertiesToDelNum = 0;
	PINI_PROPERTY_DATA pPropertiesToDelete[MAX_INI_PROPERTIES];

	if (!pScriptsIniFileData || !pGenericIniSection)
	{
		DEBUG_LOG(D_ERROR, "SCRIPTSINI_FILE_DATA, INI_SECTION_DATA pointer or section number is invalid.\r\nExiting now...");
		DoExit(D_ERROR);
	}

	for (DWORD i = 0; (i + 1) <= pGenericIniSection->iNumberOfProperty; i += 2)
	{
		PINI_PROPERTY_DATA pCurrProperty = pGenericIniSection->pProperties[i];
		PINI_PROPERTY_DATA pNextProperty = pGenericIniSection->pProperties[i + 1];

		if ((_tcsstr(pCurrProperty->tName, SCRIPTS_CMDLINE_PROPERTY_NAME)) && (_tcsstr(pNextProperty->tName, SCRIPTS_PARAM_PROPERTY_NAME)))
		{
			PSCRIPTSINI_ACTION_DATA pNewActionData = NULL;

			pNewActionData = (PSCRIPTSINI_ACTION_DATA) HeapAlloc(hCrawlerHeap, NULL, sizeof(SCRIPTSINI_ACTION_DATA));
			if (!pNewActionData)
			{
				DEBUG_LOG(D_ERROR, "pNewActionData pointer invalid.\r\nExiting now...");
				DoExit(D_ERROR);
			}
			pNewActionData->tCmdLine = NULL;
			pNewActionData->tParameters = NULL;

			if (pCurrProperty->tValue)
			{
				DWORD dwPropertyLen = (DWORD) _tcslen(pCurrProperty->tValue);

				pNewActionData->tCmdLine = (PTCHAR) HeapAlloc(hCrawlerHeap, NULL, (dwPropertyLen + 1) * sizeof(WCHAR));
				if (!(pNewActionData->tCmdLine))
				{
					DEBUG_LOG(D_ERROR, "tCmdLine pointer invalid.\r\nExiting now...");
					DoExit(D_ERROR);
				}
				if (memcpy_s((pNewActionData->tCmdLine), sizeof (TCHAR) * dwPropertyLen, pCurrProperty->tValue, sizeof (WCHAR) * dwPropertyLen))
				{
					DEBUG_LOG(D_ERROR, "Unable to extract Cmdline.\r\nExiting now...");
					DoExit(D_ERROR);
				}
				pNewActionData->tCmdLine[dwPropertyLen] = TEXT('\0');
				
				pPropertiesToDelete[dwPropertiesToDelNum] = pCurrProperty;
				dwPropertiesToDelNum++;
			}

			if (pNextProperty->tValue)
			{
				DWORD dwPropertyLen = (DWORD) _tcslen(pNextProperty->tValue);

				pNewActionData->tParameters = (PTCHAR) HeapAlloc(hCrawlerHeap, NULL, (dwPropertyLen + 1) * sizeof(WCHAR));
				if (!(pNewActionData->tParameters))
				{
					DEBUG_LOG(D_ERROR, "tCmdLine pointer invalid.\r\nExiting now...");
					DoExit(D_ERROR);
				}
				if (memcpy_s((pNewActionData->tParameters), sizeof (TCHAR) * dwPropertyLen, pNextProperty->tValue, sizeof (WCHAR) * dwPropertyLen))
				{
					DEBUG_LOG(D_ERROR, "Unable to extract Parameters.\r\nExiting now...");
					DoExit(D_ERROR);
				}
				pNewActionData->tParameters[dwPropertyLen] = TEXT('\0');

				pPropertiesToDelete[dwPropertiesToDelNum] = pNextProperty;
				dwPropertiesToDelNum++;
			}

			switch(dwSectionID)
			{
			case SCRIPTS_LOGON_SECTION_ID:
				pScriptsIniFileData->pLogonScripts[pScriptsIniFileData->dwLogonScriptNum] = pNewActionData;
				pScriptsIniFileData->dwLogonScriptNum++;				
				break;
			case SCRIPTS_LOGOFF_SECTION_ID:
				pScriptsIniFileData->pLogoffScripts[pScriptsIniFileData->dwLogoffScriptNum] = pNewActionData;
				pScriptsIniFileData->dwLogoffScriptNum++;
				break;
			case SCRIPTS_STARTUP_SECTION_ID:
				pScriptsIniFileData->pStartupScripts[pScriptsIniFileData->dwStartupScriptNum] = pNewActionData;
				pScriptsIniFileData->dwStartupScriptNum++;
				break;
			case SCRIPTS_SHUTDOWN_SECTION_ID:
				pScriptsIniFileData->pShutdownScripts[pScriptsIniFileData->dwShutdownScriptNum] = pNewActionData;
				pScriptsIniFileData->dwShutdownScriptNum++;
				break;
			default:
				DEBUG_LOG(D_ERROR, "Unable to identify SCRIPTS_SECTION_ID.\r\nExiting now...");
				DoExit(D_ERROR);
				break;
			}			
		}
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

BOOL FreeScriptsIniFileData(_Inout_ PSCRIPTSINI_FILE_DATA pScriptsIniFileData)
{
	if (pScriptsIniFileData == NULL)
	{
		DEBUG_LOG(D_ERROR, "SCRIPTSINI_FILE_DATA pointer invalid.\r\nExiting now...");
		DoExit(D_ERROR);
	}	

	for (DWORD i = 0; i < pScriptsIniFileData->dwLogonScriptNum; ++i)
	{
		PSCRIPTSINI_ACTION_DATA pActionData = pScriptsIniFileData->pLogonScripts[i];

		if (!pActionData)
			continue;

		if (pActionData->tCmdLine)
		{
			HeapFree(hCrawlerHeap, NULL, pActionData->tCmdLine);
			pActionData->tCmdLine = NULL;
		}
		if (pActionData->tParameters)
		{
			HeapFree(hCrawlerHeap, NULL, pActionData->tParameters);
			pActionData->tParameters = NULL;
		}
		HeapFree(hCrawlerHeap, NULL, pActionData);
	}

	for (DWORD i = 0; i < pScriptsIniFileData->dwLogoffScriptNum; ++i)
	{
		PSCRIPTSINI_ACTION_DATA pActionData = pScriptsIniFileData->pLogoffScripts[i];

		if (!pActionData)
			continue;

		if (pActionData->tCmdLine)
		{
			HeapFree(hCrawlerHeap, NULL, pActionData->tCmdLine);
			pActionData->tCmdLine = NULL;
		}
		if (pActionData->tParameters)
		{
			HeapFree(hCrawlerHeap, NULL, pActionData->tParameters);
			pActionData->tParameters = NULL;
		}
		HeapFree(hCrawlerHeap, NULL, pActionData);
	}

	for (DWORD i = 0; i < pScriptsIniFileData->dwStartupScriptNum; ++i)
	{
		PSCRIPTSINI_ACTION_DATA pActionData = pScriptsIniFileData->pStartupScripts[i];

		if (!pActionData)
			continue;

		if (pActionData->tCmdLine)
		{
			HeapFree(hCrawlerHeap, NULL, pActionData->tCmdLine);
			pActionData->tCmdLine = NULL;
		}
		if (pActionData->tParameters)
		{
			HeapFree(hCrawlerHeap, NULL, pActionData->tParameters);
			pActionData->tParameters = NULL;
		}
		HeapFree(hCrawlerHeap, NULL, pActionData);
	}

	for (DWORD i = 0; i < pScriptsIniFileData->dwShutdownScriptNum; ++i)
	{
		PSCRIPTSINI_ACTION_DATA pActionData = pScriptsIniFileData->pShutdownScripts[i];

		if (!pActionData)
			continue;

		if (pActionData->tCmdLine)
		{
			HeapFree(hCrawlerHeap, NULL, pActionData->tCmdLine);
			pActionData->tCmdLine = NULL;
		}
		if (pActionData->tParameters)
		{
			HeapFree(hCrawlerHeap, NULL, pActionData->tParameters);
			pActionData->tParameters = NULL;
		}
		HeapFree(hCrawlerHeap, NULL, pActionData);
	}
	for (DWORD i = 0; i < pScriptsIniFileData->dwNumberOfUnReferrencedSections; ++i)
	{
		PINI_SECTION_DATA pSectionToDelete = pScriptsIniFileData->pUnReferrencedSections[i];

		if (pSectionToDelete)
			FreeSectionData(pSectionToDelete);
	}

	HeapFree(hCrawlerHeap, NULL, pScriptsIniFileData);
	pScriptsIniFileData = NULL;
	return TRUE;
}