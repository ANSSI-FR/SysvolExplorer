/**************************************************
* SysvolCrawler - MISCParser.c
* AUTHOR: Luc Delsalle
*
* Parsing engine for ADM file
*
* ANSSI/COSSI/DTO/BAI - 2014
***************************************************/

#include "ADMParser.h"

VOID RegisterAdmParser(_Inout_ PPARSER_IDENTIFIER *pParserID)
{
	*pParserID = (PPARSER_IDENTIFIER)HeapAlloc(hCrawlerHeap, HEAP_ZERO_MEMORY, sizeof (PARSER_IDENTIFIER));
	if (!pParserID)
	{
		DEBUG_LOG(D_ERROR, "Unable to allocate PARSER_IDENTIFIER structure.\r\nExiting now...");
		DoExit(D_ERROR);
	}

	(*pParserID)->tParserName = ADM_PARSER_NAME;
	(*pParserID)->tFileMatchingRegExp = ADM_MATCHING_FILE_REGEXP;
	(*pParserID)->tFolderMatchingRegExp = NULL;
	(*pParserID)->pParserEntryPoint = ParseAdmFile;
}

BOOL ParseAdmFile(_In_ PTCHAR tFilePath)
{
	PADM_FILE_DATA pAdmData = NULL;
	HANDLE hAdmFile = INVALID_HANDLE_VALUE;
	DWORD dwFileSize = 0, dwNumberOfBytesRead = 0;
	PBYTE pbADMRawDATA = NULL;

	if (tFilePath == NULL)
	{
		DEBUG_LOG(D_ERROR, "FILEPATH pointer invalid.\r\nExiting now...");
		DoExit(D_ERROR);
	}
	DEBUG_LOG(D_MISC, "[ADM] Now parsing %ws\r\n", tFilePath);

	pAdmData = (PADM_FILE_DATA)HeapAlloc(hCrawlerHeap, HEAP_ZERO_MEMORY, sizeof (ADM_FILE_DATA));
	if (!pAdmData)
	{
		DEBUG_LOG(D_ERROR, "Unable to allocate ADM_FILE_DATA structure.\r\nExiting now...");
		DoExit(D_ERROR);
	}
	pAdmData->dwDataSize = 0;
	pAdmData->pbData = NULL;
	pAdmData->tFilePath = tFilePath;

	hAdmFile = CreateFile_s(tFilePath, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
	if (hAdmFile == INVALID_HANDLE_VALUE)
	{
		DEBUG_LOG(D_ERROR, "Unable to open file %ws. ErrorCode: %d.\r\n", tFilePath, GetLastError());
		SetLastError(ERROR_ACCESS_DENIED);
		return FALSE;
	}

	dwFileSize = GetFileSize(hAdmFile, NULL);
	if (dwFileSize == INVALID_FILE_SIZE)
	{
		DEBUG_LOG(D_ERROR, "Error during reading FileSize for %ws.\r\nExiting now...", tFilePath);
		DoExit(D_ERROR);
	}
	pAdmData->dwDataSize = dwFileSize;

	// Ensure that the file isnt to heavy for output printer
	if (dwFileSize < MISC_MAX_FILE_SIZE)
	{
		pbADMRawDATA = (PBYTE)HeapAlloc(hCrawlerHeap, HEAP_ZERO_MEMORY, sizeof (DWORD)* dwFileSize);
		if (pbADMRawDATA == NULL)
		{
			DEBUG_LOG(D_ERROR, "Unable to allocate pbMISCRawDATA.\r\nExiting now...");
			DoExit(D_ERROR);
		}

		if (!ReadFile(hAdmFile, pbADMRawDATA, dwFileSize, &dwNumberOfBytesRead, NULL))
		{
			DEBUG_LOG(D_ERROR, "Unable to read file %ws. ErrorCode: %d.\r\n", tFilePath, GetLastError());
			return FALSE;
		}
	}
	else // if the file is too big, we put error message instead
	{
		PTCHAR ptMsg = MISC_MAX_FILE_ERR_MSG;
		DWORD dwMsgLen = (DWORD)_tcslen(ptMsg);

		DEBUG_LOG(D_WARNING, "The file is %ws too big to be collected. Please save it manually\r\n.", tFilePath);
		pbADMRawDATA = (PBYTE)HeapAlloc(hCrawlerHeap, HEAP_ZERO_MEMORY, sizeof(TCHAR)* (dwMsgLen + 1));
		if (!pbADMRawDATA)
		{
			DEBUG_LOG(D_ERROR, "Unable to allocate memory (ErrCode=%d).\r\n.", GetLastError());
			DoExit(D_ERROR);
		}
		if (memcpy_s(pbADMRawDATA, (dwMsgLen + 1) * sizeof (TCHAR), ptMsg, sizeof(TCHAR)* dwMsgLen))
		{
			DEBUG_LOG(D_ERROR, "Unable to copy message.\r\nExiting now...");
			DoExit(D_ERROR);
		}
		pAdmData->dwDataSize = dwMsgLen;
	}
	pAdmData->pbData = pbADMRawDATA;
	CloseHandle(hAdmFile);

	// Call printers
	PrintAdmDataHeader(pAdmData->tFilePath);
	PrintData(pAdmData);
	PrintAdmDataFooter(pAdmData->tFilePath);

	// Release data
	HeapFree(hCrawlerHeap, NULL, pAdmData->pbData);
	HeapFree(hCrawlerHeap, NULL, pAdmData);
	return TRUE;
}