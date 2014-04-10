/**************************************************
* SysvolCrawler - INFParser.c
* AUTHOR: Luc Delsalle	
*
* Parsing engine for .inf file like GptTmpl.inf
*	
* ANSSI/COSSI/DTO/BAI - 2014
***************************************************/

#include "INFParser.h"

VOID RegisterInfParser(_Inout_ PPARSER_IDENTIFIER *pParserID)
{
	*pParserID = (PPARSER_IDENTIFIER) HeapAlloc(hCrawlerHeap, HEAP_ZERO_MEMORY, sizeof (PARSER_IDENTIFIER));
	if (!pParserID)
	{
		DEBUG_LOG(D_ERROR, "Unable to allocate PARSER_IDENTIFIER structure.\r\nExiting now...");
		DoExit(D_ERROR);
	}

	(*pParserID)->tParserName = INF_PARSER_NAME;
	(*pParserID)->tFileMatchingRegExp = INF_MATCHING_FILE_REGEXP;
	(*pParserID)->tFolderMatchingRegExp = NULL;
	(*pParserID)->pParserEntryPoint = ParseInfFile;
}

BOOL ParseInfFile(_In_ PTCHAR tFilePath)
{
	PINF_FILE_DATA pInfData = NULL;
	PINF_SECTION_DATA pCurrentSection = NULL;
	HANDLE hINFFile = INVALID_HANDLE_VALUE;
	DWORD dwFileSize = 0, dwNumberOfBytesRead = 0;
	PBYTE pbINFRawDATA = NULL;
	BYTE bFileEncoding[2];
	BOOL bIsAsciiFile = FALSE;

	if (tFilePath == NULL)
	{
		DEBUG_LOG(D_ERROR, "FILEPATH pointer invalid.\r\nExiting now...");
		DoExit(D_ERROR);
	}
	DEBUG_LOG(D_MISC, "[INF] Now parsing %ws\r\n", tFilePath);

	pInfData = (PINF_FILE_DATA) HeapAlloc(hCrawlerHeap, HEAP_ZERO_MEMORY, sizeof (INF_FILE_DATA));
	if (!pInfData)
	{
		DEBUG_LOG(D_ERROR, "Unable to allocate INF_FILE_DATA structure.\r\nExiting now...");
		DoExit(D_ERROR);
	}
	pInfData->iNumberOfSection = 0;
	pInfData->tFilePath = tFilePath;

	hINFFile = CreateFile_s(tFilePath, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
	if (hINFFile == INVALID_HANDLE_VALUE)
	{
		DEBUG_LOG(D_ERROR, "Unable to open file %ws. ErrorCode: %d.\r\n", tFilePath, GetLastError());
		SetLastError(ERROR_ACCESS_DENIED);
		return FALSE;
	}

	// Guess file type encoding
	if (!ReadFile(hINFFile, bFileEncoding, 2, &dwNumberOfBytesRead, NULL) || (dwNumberOfBytesRead != 2))
	{
		DEBUG_LOG(D_ERROR, "Unable to read file encoding for %ws. ErrorCode: %d.\r\n", tFilePath, GetLastError());
		return FALSE;
	}
	if ((bFileEncoding[0] == 0xff) && (bFileEncoding[1] == 0xfe))
		DEBUG_LOG(D_MISC, "[INF] Found valid INF file encododed in UTF-16 (Unicode).\r\n", tFilePath);
	else if ((bFileEncoding[0] == '\xFE') && (bFileEncoding[1] == '\xFF'))
	{
		DEBUG_LOG(D_MISC, "[INF] Found valid INF file encododed in UTF-8 (ASCII).\r\n", tFilePath);
		bIsAsciiFile = TRUE;
	}
	else
	{
		DEBUG_LOG(D_MISC, "[INF] Invalid encoding for INF file %ws. Considering this file as ASCII.\r\n", tFilePath);
		SetFilePointer(hINFFile, 0, NULL, FILE_BEGIN);
		bIsAsciiFile = TRUE;
	}

	dwFileSize = GetFileSize(hINFFile, NULL);
	if (dwFileSize == INVALID_FILE_SIZE)
	{
		DEBUG_LOG(D_ERROR, "Error during reading FileSize for %ws.\r\nExiting now...", tFilePath);
		DoExit(D_ERROR);
	}

	pbINFRawDATA = (PBYTE) HeapAlloc(hCrawlerHeap, HEAP_ZERO_MEMORY, sizeof (DWORD) * dwFileSize);
	if (pbINFRawDATA == NULL)
	{
		DEBUG_LOG(D_ERROR, "pbINFRawDATA pointer invalid.\r\nExiting now...");
		DoExit(D_ERROR);
	}

	if (!ReadFile(hINFFile, pbINFRawDATA, dwFileSize, &dwNumberOfBytesRead, NULL))
	{
		DEBUG_LOG(D_ERROR, "Unable to read file %ws. ErrorCode: %d.\r\n", tFilePath, GetLastError());
		return FALSE;
	}
	CloseHandle(hINFFile);

	if (bIsAsciiFile)
	{
		PBYTE pbOLD = pbINFRawDATA;
		pbINFRawDATA = (PBYTE) CStrToPtchar(pbINFRawDATA, dwNumberOfBytesRead);
		dwNumberOfBytesRead *= (sizeof (WCHAR));
		if (pbOLD)
			HeapFree(hCrawlerHeap, NULL, pbOLD);
	}

	// Browse file and build pInfData structure
	for (DWORD i = 0; i < dwNumberOfBytesRead; i += 2)
	{
		PWCHAR tCurrentLine = NULL;
		
		// Get a new line line
		if (GetLine(&i, dwNumberOfBytesRead, &pbINFRawDATA, &tCurrentLine) == FALSE)
			break;

		if (IsLineEmpty(tCurrentLine) == TRUE)
			goto freeAndContinue;

		if (IsLineComment(tCurrentLine) == TRUE)
			goto freeAndContinue;

		// Determine if the line is a new property or a new section
		PTCHAR tNewSectionName = NULL;
		if (IsLineContainASection(tCurrentLine, &tNewSectionName) == TRUE)
		{
			PINF_SECTION_DATA pNewSection = NULL;

			DEBUG_LOG(D_MISC, "[INF] New section found [%ws] for %ws\r\n", tNewSectionName, tFilePath);
			
			if (AddNewSection(pInfData, tNewSectionName, &pNewSection) == FALSE)
			{
				DEBUG_LOG(D_ERROR, "Unable to add new section.\r\nExiting now...");
				DoExit(D_ERROR);
			}
			else
			{
				pCurrentSection = pNewSection;
			}
		}
		else
		{
			if (!pCurrentSection)
			{
				DEBUG_LOG(D_ERROR, "[INF] File invalid, property (%ws) found before any section.\r\n", tCurrentLine);
				return FALSE;
			}
			else
				DEBUG_LOG(D_MISC, "[INF] New property found [%ws] for section %ws\r\n", tCurrentLine, pCurrentSection->tSectionName);
			
			AddNewProperty(pCurrentSection, tCurrentLine);
		}

	freeAndContinue:
		if (tCurrentLine) HeapFree(hCrawlerHeap, NULL, tCurrentLine);
	}
	PrintInfDataHeader(pInfData->tFilePath);
	PrintData(pInfData);
	PrintInfDataFooter(pInfData->tFilePath);

	// Cleanup
	FreeInfFileData(pInfData);
	HeapFree(hCrawlerHeap, NULL, pbINFRawDATA);
	
	return TRUE;
}

BOOL AddNewSection(_Inout_ PINF_FILE_DATA pInfData, _In_ PWCHAR pSectionName, _In_ PINF_SECTION_DATA *pOutNewSection)
{
	*pOutNewSection = NULL;

	// Create and fill new section
	*pOutNewSection = (PINF_SECTION_DATA) HeapAlloc(hCrawlerHeap, HEAP_ZERO_MEMORY, sizeof (INF_SECTION_DATA));
	if ((*pOutNewSection) == NULL)
	{
		DEBUG_LOG(D_ERROR, "pOutNewSection pointer invalid.\r\nExiting now...");
		DoExit(D_ERROR);
	}
	(*pOutNewSection)->iNumberOfProperty = 0;
	(*pOutNewSection)->tSectionName = pSectionName;

	if ((pInfData->iNumberOfSection + 1) == MAX_INF_SECTIONS)
	{
		DEBUG_LOG(D_ERROR, "[INF] Unable to add new section, reached the maximum number of section for one Ini file\r\n");
		return FALSE;
	}

	// Add created section to global INF structure
	pInfData->pSections[pInfData->iNumberOfSection] = (*pOutNewSection);
	pInfData->iNumberOfSection += 1;

	return TRUE;
}

BOOL AddNewProperty(_Inout_ PINF_SECTION_DATA pSectionData, _In_ PWCHAR tRawValue)
{
	DWORD i = 0;
	BOOL bValueFound = FALSE;
	DWORD dwNameBegin = 0, dwNameLen = 0, dwValueBegin = 0, dwValueLen = 0;
	PINF_PROPERTY_DATA pInfPropertyData = NULL;

	if (!pSectionData || ! tRawValue)
	{
		DEBUG_LOG(D_ERROR, "INF_SECTION_DATA or CHAR pointer invalid.\r\nExiting now...");
		DoExit(D_ERROR);
	}

	pInfPropertyData = (PINF_PROPERTY_DATA) HeapAlloc(hCrawlerHeap, HEAP_ZERO_MEMORY, sizeof (INF_PROPERTY_DATA));
	if (!pInfPropertyData)
	{
		DEBUG_LOG(D_ERROR, "Unable to alloc pInfPropertyData.\r\nExiting now...");
		DoExit(D_ERROR);
	}
	pInfPropertyData->tName = NULL;
	pInfPropertyData->tValue = NULL;

	while (tRawValue[i] != TEXT('\0'))
	{
		if ((tRawValue[i] == TEXT(INF_PROPERTY_SEPARATOR_SYMBOL))
			&& (i > 0)
			&& (tRawValue[i - 1] != TEXT(INF_ESCAPE_SYMBOL)))
		{
			bValueFound = TRUE;
			dwNameLen = i;
			dwValueBegin = i + 1;
		}
		++i;
	}

	if (bValueFound == FALSE)
		dwNameLen = i;
	else
		dwValueLen = i - dwValueBegin;

	// Extract property name
	pInfPropertyData->tName = (PTCHAR) HeapAlloc(hCrawlerHeap, HEAP_ZERO_MEMORY, sizeof (WCHAR) * (dwNameLen + 1));
	if (!(pInfPropertyData->tName))
	{
		DEBUG_LOG(D_ERROR, "Unable to alloc tName.\r\nExiting now...");
		DoExit(D_ERROR);
	}
	if (memcpy_s((pInfPropertyData->tName), sizeof (WCHAR) * dwNameLen, (tRawValue + dwNameBegin), sizeof (WCHAR) * dwNameLen))
	{
		DEBUG_LOG(D_ERROR, "Unable to extract property name.\r\nExiting now...");
		DoExit(D_ERROR);
	}
	pInfPropertyData->tName[dwNameLen] = '\0';
	TrimWhiteSpace(&(pInfPropertyData->tName));


	// Extract property value if exists
	if (bValueFound == FALSE)
	{
		pInfPropertyData->tValue = (PTCHAR) HeapAlloc(hCrawlerHeap, HEAP_ZERO_MEMORY, sizeof (WCHAR) * 1);
		pInfPropertyData->tValue[0] = TEXT('\0');
		goto AddPropertyTosection;
	}

	pInfPropertyData->tValue = (PTCHAR) HeapAlloc(hCrawlerHeap, HEAP_ZERO_MEMORY, sizeof (WCHAR) * (dwValueLen + 1));
	if (!(pInfPropertyData->tValue))
	{
		DEBUG_LOG(D_ERROR, "Unable to alloc tValue.\r\nExiting now...");
		DoExit(D_ERROR);
	}

	if (memcpy_s((pInfPropertyData->tValue), sizeof (WCHAR) * dwValueLen, (tRawValue + dwValueBegin), sizeof (WCHAR) * dwValueLen))
	{
		DEBUG_LOG(D_ERROR, "Unable to extract property value.\r\nExiting now...");
		DoExit(D_ERROR);
	}
	pInfPropertyData->tValue[dwValueLen] = '\0';
	TrimWhiteSpace(&(pInfPropertyData->tValue));


	// Add the new property to current section
AddPropertyTosection:
	if (pSectionData->iNumberOfProperty < MAX_INF_PROPERTIES)
	{
		pSectionData->pProperties[pSectionData->iNumberOfProperty] = pInfPropertyData;
		pSectionData->iNumberOfProperty += 1;
	}
	else
	{
		DEBUG_LOG(D_ERROR, "[INF] Unable to add new property, reached the maximum number of section for one Inf file\r\n");
		return FALSE;
	}

	return TRUE;
}

BOOL FreeInfFileData(_Inout_ PINF_FILE_DATA pInfData)
{
	PINF_SECTION_DATA pCurrentSection = NULL;
	PINF_PROPERTY_DATA pCurrentProperty = NULL;

	if (pInfData == NULL)
	{
		DEBUG_LOG(D_ERROR, "INF_FILE_DATA pointer invalid.\r\nExiting now...");
		DoExit(D_ERROR);
	}

	for (DWORD i = 0; i < pInfData->iNumberOfSection; ++i)
	{
		pCurrentSection = pInfData->pSections[i];
		if (!pCurrentSection)
			continue;

		for (DWORD j = 0; j < pCurrentSection->iNumberOfProperty; ++j)
		{
			pCurrentProperty = pCurrentSection->pProperties[j];
			if (!pCurrentProperty)
				continue;

			if (pCurrentProperty->tName)
				HeapFree(hCrawlerHeap, NULL, pCurrentProperty->tName);
			if (pCurrentProperty->tValue)
				HeapFree(hCrawlerHeap, NULL, pCurrentProperty->tValue);

			HeapFree(hCrawlerHeap, NULL, pCurrentProperty);
		}

		if (pCurrentSection->tSectionName)
			HeapFree(hCrawlerHeap, NULL, pCurrentSection->tSectionName);
		HeapFree(hCrawlerHeap, NULL, pCurrentSection);
	}

	HeapFree(hCrawlerHeap, NULL, pInfData);
	return TRUE;
}

BOOL IsLineContainASection(_In_ PWCHAR tLine, _In_ PWCHAR *pSectionName)
{
	DWORD dwSectionNameStart = 0, dwSectionNameLen = 0;
	BOOL isCharFound = FALSE;
	
	*pSectionName = NULL;

	for (DWORD i = 0; tLine != '\0'; ++i)
	{
		WCHAR cCurrChar = (WCHAR) *(tLine + i);
		WCHAR cPrevChar = (WCHAR) *(tLine + i - 2);

		if ((isCharFound == FALSE) && (cCurrChar == TEXT(' ')))
			continue;
		else if ((isCharFound == FALSE) && (cCurrChar == TEXT('[')))
		{
			isCharFound = TRUE;
			dwSectionNameStart = i + 1;
			continue;
		}
		else if (isCharFound == FALSE)
			break;
		
		if ((isCharFound == TRUE) && (cCurrChar == TEXT(']')) && (cPrevChar != TEXT(INF_ESCAPE_SYMBOL)))
		{
			dwSectionNameLen = i - dwSectionNameStart;
			*pSectionName = (PTCHAR) HeapAlloc(hCrawlerHeap, HEAP_ZERO_MEMORY, (dwSectionNameLen + 1) * sizeof(WCHAR));

			if (memcpy_s((*pSectionName), dwSectionNameLen * sizeof(WCHAR), (tLine + dwSectionNameStart), dwSectionNameLen * sizeof(WCHAR)))
			{
				DEBUG_LOG(D_ERROR, "Unable to extract section name.\r\nExiting now...");
				DoExit(D_ERROR);
			}
			(*pSectionName)[dwSectionNameLen] = TEXT('\0');
			return TRUE;
		}
	}
	return FALSE;
}

BOOL IsLineComment(_In_ PWCHAR tLine)
{
	DWORD i = 0;
	PWCHAR wCurrentChar;

	if (!tLine)
		return FALSE;
	wCurrentChar = tLine;

	while ((wCurrentChar)
		&& (*wCurrentChar != '\0')
		&& (*wCurrentChar != ';')
		&& ((*wCurrentChar == ' ') || (*wCurrentChar == '\r') || (*wCurrentChar == '\n')))
		wCurrentChar++;

	if ((wCurrentChar) && (*wCurrentChar == ';'))
		return TRUE;
	else
		return FALSE;
}