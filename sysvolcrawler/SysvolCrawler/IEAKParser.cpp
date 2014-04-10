/**************************************************
* SysvolCrawler - IEAKParser.c
* AUTHOR: Luc Delsalle	
*
* Parsing engine for Internet Explorer file
* (store in IEAK folder)
*	
* ANSSI/COSSI/DTO/BAI - 2014
***************************************************/

#include "IEAKParser.h"

VOID RegisterIeakParser(_Inout_ PPARSER_IDENTIFIER *pParserID)
{
	*pParserID = (PPARSER_IDENTIFIER) HeapAlloc(hCrawlerHeap, HEAP_ZERO_MEMORY, sizeof (PARSER_IDENTIFIER));
	if (!pParserID)
	{
		DEBUG_LOG(D_ERROR, "Unable to allocate PARSER_IDENTIFIER structure.\r\nExiting now...");
		DoExit(D_ERROR);
	}

	(*pParserID)->tParserName = IEAK_PARSER_NAME;
	(*pParserID)->tFileMatchingRegExp = NULL;
	(*pParserID)->tFolderMatchingRegExp = IEAK_MATCHING_FOLDER_REGEXP;
	(*pParserID)->pParserEntryPoint = ParseIeakFile;
}

BOOL ParseIeakFile(_In_ PTCHAR tFilePath)
{
	PIEAK_FILE_DATA pIeakFileData = NULL;
	HANDLE hIeakFile = INVALID_HANDLE_VALUE;
	DWORD dwFileSize = 0, dwNumberOfBytesRead = 0;
	PBYTE pbIeakFileRawDATA = NULL;

	if (tFilePath == NULL)
	{
		DEBUG_LOG(D_ERROR, "FILEPATH pointer invalid.\r\nExiting now...");
		DoExit(D_ERROR);
	}
	DEBUG_LOG(D_INFO, "[IEAK] Now handling %ws\r\n", tFilePath);

	pIeakFileData = (PIEAK_FILE_DATA) HeapAlloc(hCrawlerHeap, HEAP_ZERO_MEMORY, sizeof (IEAK_FILE_DATA));
	if (!pIeakFileData)
	{
		DEBUG_LOG(D_ERROR, "Unable to allocate IEAK_FILE_DATA structure.\r\nExiting now...");
		DoExit(D_ERROR);
	}
	pIeakFileData->dwDataSize = 0;
	pIeakFileData->pvData = NULL;
	pIeakFileData->tFilePath = tFilePath;
	pIeakFileData->dwFileType = IEAK_UNHANDLE_FILE;

	hIeakFile = CreateFile_s(tFilePath, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
	if (hIeakFile == INVALID_HANDLE_VALUE)
	{
		DEBUG_LOG(D_ERROR, "Unable to open file %ws. ErrorCode: %d.\r\n", tFilePath, GetLastError());
		SetLastError(ERROR_ACCESS_DENIED);
		return FALSE;
	}

	dwFileSize = GetFileSize(hIeakFile, NULL);
	if (dwFileSize == INVALID_FILE_SIZE)
	{
		DEBUG_LOG(D_ERROR, "Error during reading FileSize for %ws.\r\nExiting now...", tFilePath);
		DoExit(D_ERROR);
	}
	pIeakFileData->dwDataSize = dwFileSize;

	pbIeakFileRawDATA = (PBYTE) HeapAlloc(hCrawlerHeap, HEAP_ZERO_MEMORY, sizeof (DWORD) * dwFileSize);
	if (pbIeakFileRawDATA == NULL)
	{
		DEBUG_LOG(D_ERROR, "pbIeakFileRawDATA pointer invalid.\r\nExiting now...");
		DoExit(D_ERROR);
	}

	if (!ReadFile(hIeakFile, pbIeakFileRawDATA, dwFileSize, &dwNumberOfBytesRead, NULL))
	{
		DEBUG_LOG(D_ERROR, "Unable to read file %ws. ErrorCode: %d.\r\n", tFilePath, GetLastError());
		return FALSE;
	}
	CloseHandle(hIeakFile);

	pIeakFileData->dwFileType = GetIEAKFileExtensionID(pIeakFileData->tFilePath);
	
	if (FillIeakDataContent(pIeakFileData, pbIeakFileRawDATA, dwNumberOfBytesRead) == FALSE)
	{
		DEBUG_LOG(D_ERROR, "Unable to fill data structure for %ws.\r\nExiting now...", tFilePath);
		DoExit(D_ERROR);
	}
	HeapFree(hCrawlerHeap, NULL, pbIeakFileRawDATA);

	// Call printers
	PrintIeakDataHeader(pIeakFileData->tFilePath);
	PrintData(pIeakFileData);
	PrintIeakDataFooter(pIeakFileData->tFilePath);

	// Cleanup
	FreeIeakFileData(pIeakFileData);
	return TRUE;
}

BOOL FreeIeakFileData(_Inout_ PIEAK_FILE_DATA pIeakFileData)
{
	if (!pIeakFileData)
	{
		DEBUG_LOG(D_ERROR, "IEAK_FILE_DATA pointer is invalid.\r\nExiting now...");
		DoExit(D_ERROR);
	}

	switch(pIeakFileData->dwFileType)
	{
	case IEAK_INI_FILE:
		FreeIniFileData((PINI_FILE_DATA) pIeakFileData->pvData);
		break;
	case IEAK_INF_FILE: // Parse INF as an INI
		FreeIniFileData((PINI_FILE_DATA) pIeakFileData->pvData);
		break;
	default:
		HeapFree(hCrawlerHeap, NULL, pIeakFileData->pvData);
		break;
	}

	if (pIeakFileData)
		HeapFree(hCrawlerHeap, NULL, pIeakFileData);
	return TRUE;
}

IEAK_FILE_EXTENSION GetIEAKFileExtensionID(_In_ PTCHAR tFilePath)
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

	if (!_tcscmp(tFileExtension, IEAK_INI_FILE_EXTENSION))
		return IEAK_INI_FILE;
	else if (!_tcscmp(tFileExtension, IEAK_INF_FILE_EXTENSION))
		return IEAK_INF_FILE;
	else
		return IEAK_UNHANDLE_FILE;
}

BOOL FillIeakDataContent(_Inout_ PIEAK_FILE_DATA pIeakFileData, _In_ PBYTE pbIeakFileRawDATA, _In_ DWORD dwIeakFileRawDATALen)
{
	if (!pIeakFileData || !pbIeakFileRawDATA)
	{
		DEBUG_LOG(D_ERROR, "IEAK_FILE_DATA pointer or raw data invalid.\r\nExiting now...");
		DoExit(D_ERROR);
	}

	switch(pIeakFileData->dwFileType)
	{
	case IEAK_INI_FILE:
		return FillIniDataContent(pIeakFileData, pbIeakFileRawDATA, dwIeakFileRawDATALen);
		break;
	case IEAK_INF_FILE: // Parse INF file like an INI
		return FillIniDataContent(pIeakFileData, pbIeakFileRawDATA, dwIeakFileRawDATALen);
		break;
	default:
		return FillDefaultDataContent(pIeakFileData, pbIeakFileRawDATA, dwIeakFileRawDATALen);
		break;
	}

	return TRUE;
}

BOOL FillIniDataContent(_Inout_ PIEAK_FILE_DATA pIeakFileData, _In_ PBYTE pbIeakFileRawDATA, _In_ DWORD dwIeakFileRawDATALen)
{
	PINI_FILE_DATA pGenericIniFileData = NULL;
	PBYTE pbIeakFileRawDATANew = NULL;

	if (!pIeakFileData || !pbIeakFileRawDATA)
	{
		DEBUG_LOG(D_ERROR, "IEAK_FILE_DATA pointer or raw datainvalid.\r\nExiting now...");
		DoExit(D_ERROR);
	}

	if (IsIniFileWcharEncoded(pbIeakFileRawDATA, dwIeakFileRawDATALen) == FALSE)
	{
		// In case of ANSI file, we convert it to WCHAR
		pbIeakFileRawDATANew = (PBYTE) CStrToPtchar(pbIeakFileRawDATA, dwIeakFileRawDATALen);
		pbIeakFileRawDATA = pbIeakFileRawDATANew;
		dwIeakFileRawDATALen *= sizeof (WCHAR);
		if (!pbIeakFileRawDATA)
		{
			DEBUG_LOG(D_ERROR, "Unable to convert file %ws to WideChar.\r\n", pIeakFileData->tFilePath);
			return FALSE;
		}
	}
	else
		// In case of WCHAR, we simply skip the BOM
		pbIeakFileRawDATA +=2;

	pGenericIniFileData = ParseIniFile((PWCHAR) pbIeakFileRawDATA, dwIeakFileRawDATALen, pIeakFileData->tFilePath);
	if (!pGenericIniFileData)
	{
		DEBUG_LOG(D_ERROR, "Unable to parse generic IEAK file : %ws.\r\nExiting now...", pIeakFileData->tFilePath);
		DoExit(D_ERROR);
	}
	pIeakFileData->pvData = (PVOID) pGenericIniFileData;
	pIeakFileData->dwDataSize = sizeof(INI_FILE_DATA);

	if (pbIeakFileRawDATANew)
		HeapFree(hCrawlerHeap, NULL, pbIeakFileRawDATANew);

	return TRUE;
}

BOOL FillDefaultDataContent(_Inout_ PIEAK_FILE_DATA pIeakFileData, _In_ PBYTE pbIeakFileRawDATA, _In_ DWORD dwIeakFileRawDATALen)
{
	PBYTE pbRawData = NULL;

	if (!pIeakFileData || !pbIeakFileRawDATA)
	{
		DEBUG_LOG(D_ERROR, "IEAK_FILE_DATA pointer or raw datainvalid.\r\nExiting now...");
		DoExit(D_ERROR);
	}

	pbRawData = (PBYTE) HeapAlloc(hCrawlerHeap, NULL, (dwIeakFileRawDATALen) * sizeof(BYTE));
	if (!pbRawData)
	{
		DEBUG_LOG(D_ERROR, "pbRawData pointer invalid.\r\nExiting now...");
		DoExit(D_ERROR);
	}

	if (memcpy_s(pbRawData, sizeof (BYTE) * dwIeakFileRawDATALen, pbIeakFileRawDATA, sizeof (BYTE) * dwIeakFileRawDATALen))
	{
		DEBUG_LOG(D_ERROR, "Unable to extract ID.\r\nExiting now...");
		DoExit(D_ERROR);
	}
	pIeakFileData->pvData = pbRawData;
	pIeakFileData->dwDataSize = dwIeakFileRawDATALen;

	return TRUE;
}