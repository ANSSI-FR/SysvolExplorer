/**************************************************
* SysvolCrawler - Dispatcher.c
* AUTHOR: Luc Delsalle	
*
* Crawl the SYSVOL and dispatch content to the correct
* parser
*	
* ANSSI/COSSI/DTO/BAI - 2014
***************************************************/

#include "Dispatcher.h"

// Array of parsers metadata
PPARSER_IDENTIFIER pParserTable[MAX_PARSER];

BOOL InitDispatcher()
{
	PPARSER_IDENTIFIER pParserID = NULL;

	for (DWORD i = 0; i < MAX_PARSER; ++i)
		pParserTable[i] = NULL;

	// Add parser for POL file
	RegisterPOLParser(&pParserID);
	if (pParserID)
		pParserTable[0] = pParserID;
	else
	{
		DEBUG_LOG(D_ERROR, "Failed to register POL parser.\r\n");
		return FALSE;
	}

	// Add parser for INF file
	RegisterInfParser(&pParserID);
	if (pParserID)
		pParserTable[1] = pParserID;
	else
	{
		DEBUG_LOG(D_ERROR, "Failed to register INF parser.\r\n");
		return FALSE;
	}

	// Add parser for GPT.ini file
	RegisterGptIniParser(&pParserID);
	if (pParserID)
		pParserTable[2] = pParserID;
	else
	{
		DEBUG_LOG(D_ERROR, "Failed to register GPT.ini parser.\r\n");
		return FALSE;
	}

	// Add parser for AAS file
	RegisterAasParser(&pParserID);
	if (pParserID)
		pParserTable[3] = pParserID;
	else
	{
		DEBUG_LOG(D_ERROR, "Failed to register AAS parser.\r\n");
		return FALSE;
	}

	// Add parser for scripts.ini file
	RegisterScriptsIniParser(&pParserID);
	if (pParserID)
		pParserTable[4] = pParserID;
	else
	{
		DEBUG_LOG(D_ERROR, "Failed to register SCRIPTS parser.\r\n");
		return FALSE;
	}

	// Add parser for GPE.ini file
	RegisterGpeIniParser(&pParserID);
	if (pParserID)
		pParserTable[5] = pParserID;
	else
	{
		DEBUG_LOG(D_ERROR, "Failed to register GPE.ini parser.\r\n");
		return FALSE;
	}

	// Add parser for IEAK folder
	RegisterIeakParser(&pParserID);
	if (pParserID)
		pParserTable[6] = pParserID;
	else
	{
		DEBUG_LOG(D_ERROR, "Failed to register IEAK folder handler.\r\n");
		return FALSE;
	}

	// Add parser for PREFERENCES folder
	RegisterPreferencesParser(&pParserID);
	if (pParserID)
		pParserTable[7] = pParserID;
	else
	{
		DEBUG_LOG(D_ERROR, "Failed to register PREFERENCES folder handler.\r\n");
		return FALSE;
	}

	// Add parser for ADMFILES.ini file
	RegisterAdmFilesIniParser(&pParserID);
	if (pParserID)
		pParserTable[8] = pParserID;
	else
	{
		DEBUG_LOG(D_ERROR, "Failed to register ADMFILES.ini parser.\r\n");
		return FALSE;
	}

	// Add parser for FDEPLOY.ini file
	RegisterFdeployIniParser(&pParserID);
	if (pParserID)
		pParserTable[9] = pParserID;
	else
	{
		DEBUG_LOG(D_ERROR, "Failed to register FDEPLOY.ini parser.\r\n");
		return FALSE;
	}

	// Add parser for ADM file
	RegisterAdmParser(&pParserID);
	if (pParserID)
		pParserTable[10] = pParserID;
	else
	{
		DEBUG_LOG(D_ERROR, "Failed to register ADM parser.\r\n");
		return FALSE;
	}

	//
	//FIXME : Add new parser or folder handler
	//

	// Add DACL parser
	RegisterDaclParser(&pParserID);
	if (pParserID)
		pParserTable[DACL_PARSER_ID] = pParserID;
	else
	{
		DEBUG_LOG(D_ERROR, "Failed to register DACL parser.\r\n");
		return FALSE;
	}

	// Add ACCESS_DENIED parser
	RegisterDeniedParser(&pParserID);
	if (pParserID)
		pParserTable[DENIED_PARSER_ID] = pParserID;
	else
	{
		DEBUG_LOG(D_ERROR, "Failed to register DENIED parser.\r\n");
		return FALSE;
	}

	// Add parser for MISC file
	RegisterMiscParser(&pParserID);
	if (pParserID)
		pParserTable[MISC_PARSER_ID] = pParserID;
	else
	{
		DEBUG_LOG(D_ERROR, "Failed to register MISC parser.\r\n");
		return FALSE;
	}

	return TRUE;
}

BOOL FreeDispatcher()
{
	for (DWORD i = 0; i < MAX_PARSER; ++i)
	{
		if (pParserTable[i] != NULL)
		{
			HeapFree(hCrawlerHeap, NULL, pParserTable[i]);
			pParserTable[i] = NULL;
		}
	}
	return TRUE;
}

BOOL BrowseAndDispatch(_In_ TCHAR *tCurrentPath, _In_ DWORD depth)
{
	DWORD dwPathLen = 0;
	TCHAR tFindPath[MAX_PATH];
	TCHAR tFullNamePath[MAX_PATH];
	HANDLE hNode = INVALID_HANDLE_VALUE;
	WIN32_FIND_DATA sFindDataMask;
	DWORD dwLastError;

	DEBUG_LOG(D_INFO, "Target directory is now: %ws\r\n", tCurrentPath);

	// Format the string which will be pass to regexp engine
	StringCchCopy(tFindPath, MAX_PATH, tCurrentPath);
	StringCchCat(tFindPath, MAX_PATH, TEXT("\\*"));

	hNode = FindFirstFile(tFindPath, &sFindDataMask);
	if (hNode == INVALID_HANDLE_VALUE)
	{
		dwLastError = GetLastError();
		if ((dwLastError == ERROR_ACCESS_DENIED) || (dwLastError == ERROR_SHARING_VIOLATION) || (dwLastError == ERROR_UNEXP_NET_ERR))
			goto parsingerror;

		DEBUG_LOG(D_ERROR, "Folder node invalid. Error code %d\r\nExiting now...", dwLastError);
		DoExit(1);
	}

	// following the file type we dispatch the content to the right parser

	do
	{
		if (sFindDataMask.cFileName == NULL)
		{
			DEBUG_LOG(D_WARNING, "Folder node with no name found !\r\n");
			continue;
		}
		else if (!_tcscmp(sFindDataMask.cFileName, TEXT(".")) 
			|| !_tcscmp(sFindDataMask.cFileName, TEXT("..")))
			continue;

		StringCchCopy(tFullNamePath, MAX_PATH, tCurrentPath);
		StringCchCat(tFullNamePath, MAX_PATH, TEXT("\\"));
		StringCchCat(tFullNamePath, MAX_PATH, sFindDataMask.cFileName);

		if (sFindDataMask.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
		{			
			pParserTable[DACL_PARSER_ID]->pParserEntryPoint(tFullNamePath);
			BrowseAndDispatch(tFullNamePath, ++depth);			
		}			
		else
			DispatchFile(sFindDataMask.cFileName, tFullNamePath);
	}
	while (FindNextFile(hNode, &sFindDataMask) != 0);

	if (GetLastError() != ERROR_NO_MORE_FILES) 
	{
		DEBUG_LOG(D_ERROR, "Cannot parse all the folder node.\r\n Exiting now...");
		DoExit(1);
	}

	FindClose(hNode);

	return TRUE;
parsingerror:
	if ((dwLastError == ERROR_ACCESS_DENIED) || (dwLastError == ERROR_SHARING_VIOLATION) || (dwLastError == ERROR_UNEXP_NET_ERR)) // Rattrappe les fichiers en ACCESS_DENIED, ERROR_SHARING_VIOLATION ou ERROR_UNEXP_NET_ERR
	{
		DEBUG_LOG(D_WARNING, "Folder %ws isn't readable. Sending to %ws.\r\n", tCurrentPath, pParserTable[DENIED_PARSER_ID]->tParserName);
		pParserTable[DENIED_PARSER_ID]->pParserEntryPoint(tCurrentPath);
	}
	else
	{
		DEBUG_LOG(D_ERROR, "Unable to open folder %ws.\r\nExiting now...", tCurrentPath);
		DoExit(D_ERROR);				
	}
	return FALSE;
}

BOOL DispatchFile(_In_ PTCHAR tFileName, _In_ PTCHAR tFilePath)
{
	PPARSER_IDENTIFIER pCurrentParserId = NULL;
	DWORD dwCpt = 0;
	BOOL isFileParsed = FALSE;

	while (pParserTable[dwCpt] != NULL)
	{
		pCurrentParserId = pParserTable[dwCpt];

		// If the path doesnt match a global folder (eg: IEAK, PREFERENCES, etc.) we handle it in batch
		if ((pCurrentParserId->tFolderMatchingRegExp) && (wildcmp(pCurrentParserId->tFolderMatchingRegExp, tFilePath)))
		{
			if (pCurrentParserId->pParserEntryPoint(tFilePath) == FALSE)
				goto parsingerror;
			isFileParsed = TRUE;
			break;
		}

		// If the file need to be parsed, we call the right parser
		if ((pCurrentParserId->tFileMatchingRegExp) && (wildcmp(pCurrentParserId->tFileMatchingRegExp, tFileName)))
		{
			if (pCurrentParserId->pParserEntryPoint(tFilePath) == FALSE)
				goto parsingerror;
			isFileParsed = TRUE;
			break;
		}
		++dwCpt;
	}

	// Send to generic parser in case of lake of specific parser
	if (isFileParsed == FALSE)
	{
		DEBUG_LOG(D_WARNING, "File %ws with path: %ws isn't a classical SYSVOL file. Sending to %ws.\r\n", tFileName, tFilePath, pParserTable[MISC_PARSER_ID]->tParserName);
		if (pParserTable[MISC_PARSER_ID]->pParserEntryPoint(tFilePath) == FALSE)
			goto parsingerror;
	}

	// In all case we call the dacl parser
	if (pParserTable[DACL_PARSER_ID]->pParserEntryPoint(tFilePath) == FALSE)
		goto parsingerror;

	return TRUE;
parsingerror:
	DWORD dwLastError = GetLastError();
	if (dwLastError == ERROR_ACCESS_DENIED) // Catch ACCESS_DENIED file and send it to ACCESS_DENIED parser
	{
		DEBUG_LOG(D_WARNING, "File %ws with path: %ws isn't readable. Sending to %ws.\r\n", tFileName, tFilePath, pParserTable[DENIED_PARSER_ID]->tParserName);
		pParserTable[DENIED_PARSER_ID]->pParserEntryPoint(tFilePath);
	}
	else
	{
		DEBUG_LOG(D_ERROR, "Unable to parse %ws.\r\nExiting now...", tFilePath);
		DoExit(D_ERROR);				
	}
	return FALSE;
}

// Fast, lightweight, and simple pattern matching function
// Written by Jack Handy - improved for sysvolcrawler projet
BOOL wildcmp(_In_ TCHAR* wild, _In_ TCHAR* string)
{
  TCHAR *cp = NULL, *mp = NULL;

  while ((*string) && (*wild != '*')) {
    if ((towlower(*wild) != towlower(*string)) && (*wild != '?')) {
      return 0;
    }
    wild++;
    string++;
  }

  while (*string) {
    if (*wild == '*') {
      if (!*++wild) {
        return 1;
      }
      mp = wild;
      cp = string+1;
    } else if ((towlower(*wild) == towlower(*string)) || (*wild == '?')) {
      wild++;
      string++;
    } else {
      wild = mp;
      string = cp++;
    }
  }

  while (*wild == '*') {
    wild++;
  }

  return !*wild;
}