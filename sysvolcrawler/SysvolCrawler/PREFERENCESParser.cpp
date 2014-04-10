/**************************************************
* SysvolCrawler - PREFERENCESParser.c
* AUTHOR: Luc Delsalle	
*
* Parsing engine for preferences GPO file store
* in PREFERENCES folder
*	
* ANSSI/COSSI/DTO/BAI - 2014
***************************************************/

#include "PREFERENCESParser.h"

VOID RegisterPreferencesParser(_Inout_ PPARSER_IDENTIFIER *pParserID)
{
	*pParserID = (PPARSER_IDENTIFIER) HeapAlloc(hCrawlerHeap, HEAP_ZERO_MEMORY, sizeof (PARSER_IDENTIFIER));
	if (!pParserID)
	{
		DEBUG_LOG(D_ERROR, "Unable to allocate PARSER_IDENTIFIER structure.\r\nExiting now...");
		DoExit(D_ERROR);
	}

	(*pParserID)->tParserName = PREFERENCES_PARSER_NAME;
	(*pParserID)->tFileMatchingRegExp = NULL;
	(*pParserID)->tFolderMatchingRegExp = PREFERENCES_MATCHING_FOLDER_REGEXP;
	(*pParserID)->pParserEntryPoint = ParsePreferencesFile;
}

BOOL ParsePreferencesFile(_In_ PTCHAR tFilePath)
{
	PPREFERENCES_FILE_DATA pPreferencesFileData = NULL;
	HANDLE hPreferencesFile = INVALID_HANDLE_VALUE;
	DWORD dwFileSize = 0, dwNumberOfBytesRead = 0;
	PBYTE pbPreferencesFileRawDATA = NULL;

	if (tFilePath == NULL)
	{
		DEBUG_LOG(D_ERROR, "FILEPATH pointer invalid.\r\nExiting now...");
		DoExit(D_ERROR);
	}
	DEBUG_LOG(D_INFO, "[PREFERENCES] Now handling %ws\r\n", tFilePath);

	pPreferencesFileData = (PPREFERENCES_FILE_DATA) HeapAlloc(hCrawlerHeap, HEAP_ZERO_MEMORY, sizeof (PREFERENCES_FILE_DATA));
	if (!pPreferencesFileData)
	{
		DEBUG_LOG(D_ERROR, "Unable to allocate IEAK_FILE_DATA structure.\r\nExiting now...");
		DoExit(D_ERROR);
	}
	pPreferencesFileData->dwDataSize = 0;
	pPreferencesFileData->pvData = NULL;
	pPreferencesFileData->tFilePath = tFilePath;
	pPreferencesFileData->dwFileType = PREFERENCES_UNHANDLE_FILE;

	hPreferencesFile = CreateFile_s(tFilePath, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
	if (hPreferencesFile == INVALID_HANDLE_VALUE)
	{
		DEBUG_LOG(D_ERROR, "Unable to open file %ws. ErrorCode: %d.\r\n", tFilePath, GetLastError());
		SetLastError(ERROR_ACCESS_DENIED);
		return FALSE;
	}

	dwFileSize = GetFileSize(hPreferencesFile, NULL);
	if (dwFileSize == INVALID_FILE_SIZE)
	{
		DEBUG_LOG(D_ERROR, "Error during reading FileSize for %ws.\r\nExiting now...", tFilePath);
		DoExit(D_ERROR);
	}
	pPreferencesFileData->dwDataSize = dwFileSize;

	pbPreferencesFileRawDATA = (PBYTE) HeapAlloc(hCrawlerHeap, HEAP_ZERO_MEMORY, sizeof (DWORD) * dwFileSize);
	if (pbPreferencesFileRawDATA == NULL)
	{
		DEBUG_LOG(D_ERROR, "pbPreferencesFileRawDATA pointer invalid.\r\nExiting now...");
		DoExit(D_ERROR);
	}

	if (!ReadFile(hPreferencesFile, pbPreferencesFileRawDATA, dwFileSize, &dwNumberOfBytesRead, NULL))
	{
		DEBUG_LOG(D_ERROR, "Unable to read file %ws. ErrorCode: %d.\r\n", tFilePath, GetLastError());
		return FALSE;
	}
	CloseHandle(hPreferencesFile);

	pPreferencesFileData->dwFileType = GetPreferenceFileExtensionID(pPreferencesFileData->tFilePath);
	
	if (FillPreferencesDataContent(pPreferencesFileData, pbPreferencesFileRawDATA, dwNumberOfBytesRead) == FALSE)
	{
		DEBUG_LOG(D_ERROR, "Unable to fill data structure for %ws.\r\nExiting now...", tFilePath);
		DoExit(D_ERROR);
	}
	HeapFree(hCrawlerHeap, NULL, pbPreferencesFileRawDATA);

	// Call printers
	PrintPreferencesDataHeader(pPreferencesFileData->tFilePath);
	PrintData(pPreferencesFileData);
	PrintPreferencesDataFooter(pPreferencesFileData->tFilePath);

	// Cleanup
	FreePreferencesFileData(pPreferencesFileData);
	return TRUE;
}

BOOL FreePreferencesFileData(_Inout_ PPREFERENCES_FILE_DATA pPreferencesFileData)
{
	if (!pPreferencesFileData)
	{
		DEBUG_LOG(D_ERROR, "PREFERENCES_FILE_DATA pointer is invalid.\r\nExiting now...");
		DoExit(D_ERROR);
	}

	switch(pPreferencesFileData->dwFileType)
	{
	case PREFERENCES_INI_FILE:
		FreeIniFileData((PINI_FILE_DATA) pPreferencesFileData->pvData);
		break;
	case PREFERENCES_INF_FILE: // Consider INF file like an INI file
		FreeIniFileData((PINI_FILE_DATA) pPreferencesFileData->pvData);
		break;
	default:
		HeapFree(hCrawlerHeap, NULL, pPreferencesFileData->pvData);
		break;
	}

	if (pPreferencesFileData)
		HeapFree(hCrawlerHeap, NULL, pPreferencesFileData);
	return TRUE;
}

PREFERENCES_FILE_EXTENSION GetPreferenceFileExtensionID(_In_ PTCHAR tFilePath)
{
	PTCHAR tFileName = NULL;
	PTCHAR tFileExtension = NULL;

	if (!tFilePath)
	{
		DEBUG_LOG(D_ERROR, "FILEPATH pointer invalid.\r\nExiting now...");
		DoExit(D_ERROR);
	}

	tFileName = rstrstr(tFilePath, TEXT("\\"));
	if (!tFileName)
	{
		DEBUG_LOG(D_WARNING, "The file %ws doesn't seems to be hosted in a proper sysvol folder.\r\n", tFilePath);
		tFileName = tFilePath;
	}
	else
		tFileName++;

	tFileExtension = rstrstr(tFileName, TEXT("."));
	if (!tFileExtension)
	{
		DEBUG_LOG(D_WARNING, "The filename %ws doesn't seems to have a well-kwnown extension.\r\n", tFileName);
		tFileExtension = tFileName;
	}
	else
		tFileExtension++;

	if (!_tcscmp(tFileExtension, PREFERENCES_INI_FILE_EXTENSION))
		return PREFERENCES_INI_FILE;
	else if (!_tcscmp(tFileExtension, PREFERENCES_INF_FILE_EXTENSION))
		return PREFERENCES_INF_FILE;
	else
		return PREFERENCES_UNHANDLE_FILE;
}

BOOL FillPreferencesDataContent(_Inout_ PPREFERENCES_FILE_DATA pPreferencesFileData, _In_ PBYTE pbPreferencesFileRawDATA, _In_ DWORD dwPreferencesFileRawDATALen)
{
	if (!pPreferencesFileData || !pbPreferencesFileRawDATA)
	{
		DEBUG_LOG(D_ERROR, "PREFERENCES_FILE_DATA pointer or raw data invalid.\r\nExiting now...");
		DoExit(D_ERROR);
	}

	switch(pPreferencesFileData->dwFileType)
	{
	case PREFERENCES_INI_FILE:
		return FillIniDataContent(pPreferencesFileData, pbPreferencesFileRawDATA, dwPreferencesFileRawDATALen);
		break;
	case PREFERENCES_INF_FILE: // Consider INF file like an INI file
		return FillIniDataContent(pPreferencesFileData, pbPreferencesFileRawDATA, dwPreferencesFileRawDATALen);
		break;
	default:
		return FillDefaultDataContent(pPreferencesFileData, pbPreferencesFileRawDATA, dwPreferencesFileRawDATALen);
		break;
	}

	return TRUE;
}

BOOL FillIniDataContent(_Inout_ PPREFERENCES_FILE_DATA pPreferencesFileData, _In_ PBYTE pbPreferencesFileRawDATA, _In_ DWORD dwPreferencesFileRawDATALen)
{
	PINI_FILE_DATA pGenericIniFileData = NULL;

	if (!pPreferencesFileData || !pbPreferencesFileRawDATA)
	{
		DEBUG_LOG(D_ERROR, "IEAK_FILE_DATA pointer or raw data invalid.\r\nExiting now...");
		DoExit(D_ERROR);
	}

	if (IsIniFileWcharEncoded(pbPreferencesFileRawDATA, dwPreferencesFileRawDATALen) == FALSE)
	{
		// in case of ANSI file, we need to convert it in WCHAR
		pbPreferencesFileRawDATA = (PBYTE) CStrToPtchar(pbPreferencesFileRawDATA, dwPreferencesFileRawDATALen);
		dwPreferencesFileRawDATALen *= sizeof (WCHAR);
		if (!pbPreferencesFileRawDATA)
		{
			DEBUG_LOG(D_ERROR, "Unable to convert file %ws to WideChar.\r\n", pPreferencesFileData->tFilePath);
			return FALSE;
		}
	}
	else
		// In case of WHAR file, we simply skip the BOM
		pbPreferencesFileRawDATA +=2;

	pGenericIniFileData = ParseIniFile((PWCHAR) pbPreferencesFileRawDATA, dwPreferencesFileRawDATALen, pPreferencesFileData->tFilePath);
	if (!pGenericIniFileData)
	{
		DEBUG_LOG(D_ERROR, "Unable to parse generic PREFERENCES file : %ws.\r\nExiting now...", pPreferencesFileData->tFilePath);
		DoExit(D_ERROR);
	}
	pPreferencesFileData->pvData = (PVOID) pGenericIniFileData;
	pPreferencesFileData->dwDataSize = sizeof(INI_FILE_DATA);

	return TRUE;
}

BOOL FillDefaultDataContent(_Inout_ PPREFERENCES_FILE_DATA pPreferencesFileData, _In_ PBYTE pbPreferencesFileRawDATA, _In_ DWORD dwPreferencesFileRawDATALen)
{
	PBYTE pbRawData = NULL;

	if (!pPreferencesFileData || !pbPreferencesFileRawDATA)
	{
		DEBUG_LOG(D_ERROR, "IEAK_FILE_DATA pointer or raw data invalid.\r\nExiting now...");
		DoExit(D_ERROR);
	}

	pbRawData = (PBYTE) HeapAlloc(hCrawlerHeap, NULL, (dwPreferencesFileRawDATALen) * sizeof(BYTE));
	if (!pbRawData)
	{
		DEBUG_LOG(D_ERROR, "pbRawData pointer invalid.\r\nExiting now...");
		DoExit(D_ERROR);
	}

	if (memcpy_s(pbRawData, sizeof (BYTE) * dwPreferencesFileRawDATALen, pbPreferencesFileRawDATA, sizeof (BYTE) * dwPreferencesFileRawDATALen))
	{
		DEBUG_LOG(D_ERROR, "Unable to extract ID.\r\nExiting now...");
		DoExit(D_ERROR);
	}
	pPreferencesFileData->pvData = pbRawData;
	pPreferencesFileData->dwDataSize = dwPreferencesFileRawDATALen;

	return TRUE;
}