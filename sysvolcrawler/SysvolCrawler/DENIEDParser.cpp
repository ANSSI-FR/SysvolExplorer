/**************************************************
* SysvolCrawler - DENIEDParser.c
* AUTHOR: Luc Delsalle	
*
* Parsing engine for file which couldn't be
* opened during CreateFile attempt
*	
* ANSSI/COSSI/DTO/BAI - 2014
***************************************************/

#include "DENIEDParser.h"

VOID RegisterDeniedParser(_Inout_ PPARSER_IDENTIFIER *pParserID)
{
	*pParserID = (PPARSER_IDENTIFIER) HeapAlloc(hCrawlerHeap, HEAP_ZERO_MEMORY, sizeof (PARSER_IDENTIFIER));
	if (!pParserID)
	{
		DEBUG_LOG(D_ERROR, "Unable to allocate PARSER_IDENTIFIER structure.\r\nExiting now...");
		DoExit(D_ERROR);
	}

	(*pParserID)->tParserName = DENIED_PARSER_NAME;
	(*pParserID)->tFileMatchingRegExp = DENIED_MATCHING_FILE_REGEXP;
	(*pParserID)->tFolderMatchingRegExp = NULL;
	(*pParserID)->pParserEntryPoint = ParseDeniedFile;
}

BOOL ParseDeniedFile(_In_ PTCHAR tFilePath)
{
	PDENIED_FILE_DATA pDeniedData = NULL;
	HANDLE hMiscFile = INVALID_HANDLE_VALUE;
	DWORD dwFileSize = 0, dwNumberOfBytesRead = 0;
	PBYTE pbMISCRawDATA = NULL;
	DWORD dwFileAttributes = 0;

	if (tFilePath == NULL)
	{
		DEBUG_LOG(D_ERROR, "FILEPATH pointer invalid.\r\nExiting now...");
		DoExit(D_ERROR);
	}
	DEBUG_LOG(D_MISC, "[DENIED] Now handling %ws\r\n", tFilePath);

	pDeniedData = (PDENIED_FILE_DATA) HeapAlloc(hCrawlerHeap, HEAP_ZERO_MEMORY, sizeof (DENIED_FILE_DATA));
	if (!pDeniedData)
	{
		DEBUG_LOG(D_ERROR, "Unable to allocate DENIED_FILE_DATA structure.\r\nExiting now...");
		DoExit(D_ERROR);
	}
	pDeniedData->tFilePath = tFilePath;

	dwFileAttributes = GetFileAttributes(tFilePath);
	if (dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
		pDeniedData->bIsADirectory = TRUE;
	else
		pDeniedData->bIsADirectory = FALSE;

	// Call printers
	PrintDeniedDataHeader(pDeniedData->tFilePath);
	PrintData(pDeniedData);
	PrintDeniedDataFooter(pDeniedData->tFilePath);
	
	// Release data
	HeapFree(hCrawlerHeap, NULL, pDeniedData);
	return TRUE;
}