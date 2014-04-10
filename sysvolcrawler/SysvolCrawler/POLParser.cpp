/**************************************************
* SysvolCrawler - POLParser.c
* AUTHOR: Luc Delsalle	
*
* Parsing engine for .pol file (eg. Registry.pol)
*	
* ANSSI/COSSI/DTO/BAI - 2014
***************************************************/
#include "POLParser.h"

VOID RegisterPOLParser(_Inout_ PPARSER_IDENTIFIER *pParserID)
{
	*pParserID = (PPARSER_IDENTIFIER) HeapAlloc(hCrawlerHeap, HEAP_ZERO_MEMORY, sizeof (PARSER_IDENTIFIER));
	if (!pParserID)
	{
		DEBUG_LOG(D_ERROR, "Unable to allocate PARSER_IDENTIFIER structure.\r\nExiting now...");
		DoExit(D_ERROR);
	}

	(*pParserID)->tParserName = POL_PARSER_NAME;
	(*pParserID)->tFileMatchingRegExp = POL_MATCHING_FILE_REGEXP;
	(*pParserID)->tFolderMatchingRegExp = NULL;
	(*pParserID)->pParserEntryPoint = ParsePolFile;
}

BOOL ParsePolFile(_In_ PTCHAR tFilePath)
{
	HANDLE hPOLFile = INVALID_HANDLE_VALUE;
	DWORD dwPOLMagic[2];
	DWORD dwNumberOfBytesRead = 0, dwFileSize = 0;

	if (tFilePath == NULL)
	{
		DEBUG_LOG(D_ERROR, "FILEPATH pointer invalid.\r\nExiting now...");
		DoExit(D_ERROR);
	}
	DEBUG_LOG(D_MISC, "[POL] Now parsing %ws\r\n", tFilePath);

	hPOLFile = CreateFile_s(tFilePath, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
	if (hPOLFile == INVALID_HANDLE_VALUE)
	{
		DEBUG_LOG(D_ERROR, "Unable to open file %ws. ErrorCode: %d.\r\n", tFilePath, GetLastError());
		SetLastError(ERROR_ACCESS_DENIED);
		return FALSE;
	}

	dwFileSize = GetFileSize(hPOLFile, NULL);
	if (dwFileSize == INVALID_FILE_SIZE)
	{
		DEBUG_LOG(D_ERROR, "Error during reading FileSize for %ws.\r\nExiting now...", tFilePath);
		DoExit(D_ERROR);
	}
	else if (dwFileSize == 0)
	{
		return TRUE;
	}

	// Check format magic in pol file
	if (!ReadFile(hPOLFile, dwPOLMagic, 8, &dwNumberOfBytesRead, NULL) || (dwNumberOfBytesRead != 8))
	{
		DEBUG_LOG(D_ERROR, "Unable to read file %ws. ErrorCode: %d.\r\n", tFilePath, GetLastError());
		return FALSE;
	}

	if (dwPOLMagic[0] != 0x67655250)
	{
		DEBUG_LOG(D_ERROR, "The file %ws doesn't seems to be a real POL file: MagicNumber error with value: 0x%8x.\r\n", tFilePath, dwPOLMagic[0]);
		return FALSE;
	}
	else
	{
		DEBUG_LOG(D_MISC, "Valid POL file found [version:%d] for %ws.\r\n", dwPOLMagic[1], tFilePath);
		if (ParseBodyRegisteryValues(&hPOLFile, tFilePath) == FALSE)
			return FALSE;
	}
	CloseHandle(hPOLFile);
	return TRUE;
}

BOOL ParseBodyRegisteryValues(_In_ PHANDLE hPOLFile, _In_ PTCHAR tFilePath)
{
	DWORD dwFileSize = 0;
	PBYTE pbPOLDATA = NULL;
	PPOL_DATA pPolDATA = NULL;
	DWORD dwNumberOfBytesRead = 0, dwSubTokenIndex = 0;
	INT iSubTokenStartPos = -1, iSubTokenLen = 0;
	BOOL bIsTokenFound = FALSE, bIsDataSizeExtracted = FALSE;

	if ((hPOLFile == INVALID_HANDLE_VALUE) || (tFilePath == NULL))
	{
		DEBUG_LOG(D_ERROR, "POLFILE or FILEPATH pointer invalid for %ws.\r\nExiting now...", tFilePath);
		DoExit(D_ERROR);
	}

	dwFileSize = GetFileSize(*hPOLFile, NULL);
	if (dwFileSize == INVALID_FILE_SIZE)
	{
		DEBUG_LOG(D_ERROR, "Error during reading FileSize for %ws.\r\nExiting now...", tFilePath);
		DoExit(D_ERROR);
	}

	pbPOLDATA = (PBYTE) HeapAlloc(hCrawlerHeap, HEAP_ZERO_MEMORY, sizeof (DWORD) * dwFileSize);
	if (pbPOLDATA == NULL)
	{
		DEBUG_LOG(D_ERROR, "pbPOLDATA pointer invalid.\r\nExiting now...");
		DoExit(D_ERROR);
	}

	if (!ReadFile(*hPOLFile, pbPOLDATA, dwFileSize, &dwNumberOfBytesRead, NULL))
	{
		DEBUG_LOG(D_ERROR, "Unable to read file %ws. ErrorCode: %d.\r\n", tFilePath, GetLastError());
		return FALSE;
	}

	// Extract every tokens in file(registry keys like [key;value;type;size;data]) and send them to parsing
	PrintPolDataHeader(tFilePath);
	for (DWORD i = 0; i < dwNumberOfBytesRead; ++i)
	{
		WCHAR cCurrentVal = (TCHAR) *(pbPOLDATA + i);

		// Get token beginning
		if ((cCurrentVal == TEXT('[')) 
			&&  (bIsTokenFound == FALSE))	// we didnt find a token yet
		{
			pPolDATA = (PPOL_DATA) HeapAlloc(hCrawlerHeap, HEAP_ZERO_MEMORY, sizeof (POL_DATA));
			if (pPolDATA == NULL)
			{
				DEBUG_LOG(D_ERROR, "pPolDATA pointer invalid.\r\nExiting now...");
				DoExit(D_ERROR);
			}
			else
				pPolDATA->tFilePath = tFilePath;

			// Reset aprsing indexes
			bIsTokenFound = TRUE;
			iSubTokenStartPos = i + sizeof(TCHAR);
			iSubTokenLen = -1;
		}

		// We need to determine new token size when we capture it
		if (bIsTokenFound)
		{
			iSubTokenLen++;
		}

		// Detect separators
		if ((bIsTokenFound == TRUE)				// Token found
			&& (bIsDataSizeExtracted == FALSE)  // could determine size yet
			&& (cCurrentVal == TEXT(';')))		// In case of ';': we need to extract subtoken
		{
			ExtractSubToken(pPolDATA, pbPOLDATA, iSubTokenStartPos, iSubTokenLen, dwSubTokenIndex);
			
			dwSubTokenIndex++; // increase subtoken index
			if (dwSubTokenIndex == 4) // found token size
				bIsDataSizeExtracted = TRUE;

			//Reset token indexes
			iSubTokenStartPos = i  + sizeof(TCHAR);
			iSubTokenLen = -1;
		}
		
		if ((bIsTokenFound)
			&& (bIsDataSizeExtracted == TRUE)
			&& (dwSubTokenIndex == 4)
			&& (pPolDATA->pwdSize))
		{
			ExtractSubToken(pPolDATA, pbPOLDATA, iSubTokenStartPos, (*pPolDATA->pwdSize), dwSubTokenIndex);
			dwSubTokenIndex++;
			i = iSubTokenStartPos + (*pPolDATA->pwdSize) - 1;
			continue;
		}

		// Detect token end
		if ((bIsTokenFound)
			&& (bIsDataSizeExtracted == TRUE)
			&& (dwSubTokenIndex == 5)
			&& (cCurrentVal == TEXT(']')))
		{
			BOOL bRes = FALSE;

			if (!pPolDATA->pwKey || !pPolDATA->pbValue || !pPolDATA->pwdSize || !pPolDATA->pwdType || !pPolDATA->pbData)
			{
				DEBUG_LOG(D_ERROR, "[POL] Token Invalid, must be skipped.\r\n.");
				return FALSE;
			}
			else
			{
				// Send to printer extracted token
				DEBUG_LOG(D_MISC, "[POL] Found one token ending at pos [%d].\r\n", i);
				bRes = PrintData(pPolDATA);
			}

			if (bRes == FALSE)
				DEBUG_LOG(D_ERROR, "[POL] unable to print token ending at pos [%d].\r\n", i);

			// Release data
			HeapFree(hCrawlerHeap, NULL, pPolDATA->pbData);
			HeapFree(hCrawlerHeap, NULL, pPolDATA->pbValue);
			HeapFree(hCrawlerHeap, NULL, pPolDATA->pwdSize);
			HeapFree(hCrawlerHeap, NULL, pPolDATA->pwdType);
			HeapFree(hCrawlerHeap, NULL, pPolDATA->pwKey);
			HeapFree(hCrawlerHeap, NULL, pPolDATA);

			bIsTokenFound = FALSE;
			bIsDataSizeExtracted = FALSE;
			dwSubTokenIndex = 0;
		}
	}
	PrintPolDataFooter(tFilePath);
	HeapFree(hCrawlerHeap, NULL, pbPOLDATA);
	return TRUE;
}

BOOL ExtractSubToken(_Inout_ PPOL_DATA pPolDATA, _In_ PBYTE pToken, _In_ INT iSubTokenStartPos, _In_ INT iSubTokenLen, _In_ DWORD dwSubTokenIndex)
{
	LPVOID lpDest = NULL;

	lpDest = HeapAlloc(hCrawlerHeap, HEAP_ZERO_MEMORY, sizeof (WCHAR) * (iSubTokenLen));

	if (memcpy_s(lpDest, iSubTokenLen, (pToken + iSubTokenStartPos), iSubTokenLen))
	{
		DEBUG_LOG(D_ERROR, "Unable to extract token.\r\nExiting now...");
		DoExit(D_ERROR);
	}

	switch (dwSubTokenIndex)
	{
	case 0:
		pPolDATA->pwKey = (PWCHAR) lpDest;
		break;
	case 1:
		pPolDATA->pbValue = (PBYTE) lpDest;
		pPolDATA->dwValueSize = iSubTokenLen;
		break;
	case 2:
		pPolDATA->pwdType = (PDWORD) lpDest;
		*pPolDATA->pwdType = (*pPolDATA->pwdType) / 256; 
		break;
	case 3:
		pPolDATA->pwdSize = (PDWORD) lpDest;
		break;
	case 4:
		pPolDATA->pbData = (PBYTE) lpDest;
		break;
	}
	return TRUE;
}