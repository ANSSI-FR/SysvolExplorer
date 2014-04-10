/**************************************************
* SysvolCrawler - MISCParser.c
* AUTHOR: Luc Delsalle	
*
* Parsing engine for odd file which should not be
* on a Sysvol folder
*	
* ANSSI/COSSI/DTO/BAI - 2014
***************************************************/

#include "MISCParser.h"

VOID RegisterMiscParser(_Inout_ PPARSER_IDENTIFIER *pParserID)
{
	*pParserID = (PPARSER_IDENTIFIER) HeapAlloc(hCrawlerHeap, HEAP_ZERO_MEMORY, sizeof (PARSER_IDENTIFIER));
	if (!pParserID)
	{
		DEBUG_LOG(D_ERROR, "Unable to allocate PARSER_IDENTIFIER structure.\r\nExiting now...");
		DoExit(D_ERROR);
	}

	(*pParserID)->tParserName = MISC_PARSER_NAME;
	(*pParserID)->tFileMatchingRegExp = MISC_MATCHING_FILE_REGEXP;
	(*pParserID)->tFolderMatchingRegExp = NULL;
	(*pParserID)->pParserEntryPoint = ParseMiscFile;
}

BOOL ParseMiscFile(_In_ PTCHAR tFilePath)
{
	PMISC_FILE_DATA pMiscData = NULL;
	HANDLE hMiscFile = INVALID_HANDLE_VALUE;
	DWORD dwFileSize = 0, dwNumberOfBytesRead = 0;
	PBYTE pbMISCRawDATA = NULL;

	if (tFilePath == NULL)
	{
		DEBUG_LOG(D_ERROR, "FILEPATH pointer invalid.\r\nExiting now...");
		DoExit(D_ERROR);
	}
	DEBUG_LOG(D_MISC, "[MISC] Now parsing %ws\r\n", tFilePath);

	pMiscData = (PMISC_FILE_DATA) HeapAlloc(hCrawlerHeap, HEAP_ZERO_MEMORY, sizeof (MISC_FILE_DATA));
	if (!pMiscData)
	{
		DEBUG_LOG(D_ERROR, "Unable to allocate MISC_FILE_DATA structure.\r\nExiting now...");
		DoExit(D_ERROR);
	}
	pMiscData->dwDataSize = 0;
	pMiscData->pbData = NULL;
	pMiscData->tFilePath = tFilePath;

	hMiscFile = CreateFile_s(tFilePath, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
	if (hMiscFile == INVALID_HANDLE_VALUE)
	{
		DEBUG_LOG(D_ERROR, "Unable to open file %ws. ErrorCode: %d.\r\n", tFilePath, GetLastError());
		SetLastError(ERROR_ACCESS_DENIED);
		return FALSE;
	}

	dwFileSize = GetFileSize(hMiscFile, NULL);
	if (dwFileSize == INVALID_FILE_SIZE)
	{
		DEBUG_LOG(D_ERROR, "Error during reading FileSize for %ws.\r\nExiting now...", tFilePath);
		DoExit(D_ERROR);
	}
	pMiscData->dwDataSize = dwFileSize;

	// Chack that file isnt to big to be handled by printers
	if (dwFileSize < MISC_MAX_FILE_SIZE) 
	{
		pbMISCRawDATA = (PBYTE) HeapAlloc(hCrawlerHeap, HEAP_ZERO_MEMORY, sizeof (DWORD) * dwFileSize);
		if (pbMISCRawDATA == NULL)
		{
			DEBUG_LOG(D_ERROR, "Unable to allocate pbMISCRawDATA.\r\nExiting now...");
			DoExit(D_ERROR);
		}

		if (!ReadFile(hMiscFile, pbMISCRawDATA, dwFileSize, &dwNumberOfBytesRead, NULL))
		{
			DEBUG_LOG(D_ERROR, "Unable to read file %ws. ErrorCode: %d.\r\n", tFilePath, GetLastError());
			return FALSE;
		}
	}
	else // in case of heavy file, we replace it by error message
	{
		PTCHAR ptMsg = MISC_MAX_FILE_ERR_MSG;
		DWORD dwMsgLen = (DWORD) _tcslen(ptMsg);

		DEBUG_LOG(D_WARNING, "The file is %ws too big to be collected. Please save it manually\r\n.", tFilePath);
		pbMISCRawDATA = (PBYTE) HeapAlloc(hCrawlerHeap, HEAP_ZERO_MEMORY, sizeof(TCHAR) * (dwMsgLen + 1));
		if (!pbMISCRawDATA)
		{
			DEBUG_LOG(D_ERROR, "Unable to allocate memory (ErrCode=%d).\r\n.", GetLastError());
			DoExit(D_ERROR);
		}
		if (memcpy_s(pbMISCRawDATA, (dwMsgLen + 1) * sizeof (TCHAR), ptMsg, sizeof(TCHAR) * dwMsgLen))
		{
			DEBUG_LOG(D_ERROR, "Unable to copy message.\r\nExiting now...");
			DoExit(D_ERROR);
		}
		pMiscData->dwDataSize = dwMsgLen;
	}
	pMiscData->pbData = pbMISCRawDATA;
	CloseHandle(hMiscFile);

	// Call printers
	PrintMiscDataHeader(pMiscData->tFilePath);
	PrintData(pMiscData);
	PrintMiscDataFooter(pMiscData->tFilePath);
	
	// Release data
	HeapFree(hCrawlerHeap, NULL, pMiscData->pbData);
	HeapFree(hCrawlerHeap, NULL, pMiscData);
	return TRUE;
}