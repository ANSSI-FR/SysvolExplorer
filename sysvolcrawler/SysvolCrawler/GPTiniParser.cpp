/**************************************************
* SysvolCrawler - GPTiniParser.cpp
* AUTHOR: Luc Delsalle	
*
* Parsing engine for GPT.ini file
*
* ANSSI/COSSI/DTO/BAI - 2014
***************************************************/

#include "GPTiniParser.h"

VOID RegisterGptIniParser(_Inout_ PPARSER_IDENTIFIER *pParserID)
{
	*pParserID = (PPARSER_IDENTIFIER) HeapAlloc(hCrawlerHeap, HEAP_ZERO_MEMORY, sizeof (PARSER_IDENTIFIER));
	if (!pParserID)
	{
		DEBUG_LOG(D_ERROR, "Unable to allocate PARSER_IDENTIFIER structure.\r\nExiting now...");
		DoExit(D_ERROR);
	}

	(*pParserID)->tParserName = GPTINI_PARSER_NAME;
	(*pParserID)->tFileMatchingRegExp = GPTINI_MATCHING_FILE_REGEXP;
	(*pParserID)->tFolderMatchingRegExp = NULL;
	(*pParserID)->pParserEntryPoint = ParseGptIniFile;
}

BOOL ParseGptIniFile(_In_ PTCHAR tFilePath)
{
	HANDLE hGptIniFile = INVALID_HANDLE_VALUE;
	PINI_FILE_DATA pGenericIniFileData = NULL;
	PGPTINI_FILE_DATA pGptIniFileData = NULL;
	PINI_SECTION_DATA pGeneralSection = NULL;
	PINI_PROPERTY_DATA pVersionProperty = NULL, pDisplayNameProperty = NULL;
	DWORD dwFileSize = 0, dwNumberOfBytesRead = 0;
	PBYTE pbINIRawDATA = NULL;
	BOOL bMemoryAreaMoved = FALSE;

	if (tFilePath == NULL)
	{
		DEBUG_LOG(D_ERROR, "FILEPATH pointer invalid.\r\nExiting now...");
		DoExit(D_ERROR);
	}
	DEBUG_LOG(D_MISC, "[GPT.INI] Now parsing %ws\r\n", tFilePath);

	hGptIniFile = CreateFile_s(tFilePath, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
	if (hGptIniFile == INVALID_HANDLE_VALUE)
	{
		DEBUG_LOG(D_ERROR, "Unable to open file %ws. ErrorCode: %d.\r\n", tFilePath, GetLastError());
		SetLastError(ERROR_ACCESS_DENIED);
		return FALSE;
	}

	dwFileSize = GetFileSize(hGptIniFile, NULL);
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

	if (!ReadFile(hGptIniFile, pbINIRawDATA, dwFileSize, &dwNumberOfBytesRead, NULL))
	{
		DEBUG_LOG(D_ERROR, "Unable to read file %ws. ErrorCode: %d.\r\n", tFilePath, GetLastError());
		return FALSE;
	}
	CloseHandle(hGptIniFile);

	if (IsIniFileWcharEncoded(pbINIRawDATA, dwNumberOfBytesRead) == FALSE)
	{
		PBYTE pbINIRawDATATmp = pbINIRawDATA;

		// GPT.ini is an ANSI file, we need to convert it to WCHAR
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
		// GPT.ini if a WCHAR file, we just need to skip BOM
		bMemoryAreaMoved = TRUE;
		pbINIRawDATA +=2;
	}

	pGenericIniFileData = ParseIniFile((PWCHAR) pbINIRawDATA, dwNumberOfBytesRead, tFilePath);
	if (!pGenericIniFileData)
	{
		DEBUG_LOG(D_ERROR, "Unable to parse generic ini file : %ws.\r\nExiting now...", tFilePath);
		DoExit(D_ERROR);
	}

	pGptIniFileData = (PGPTINI_FILE_DATA) HeapAlloc(hCrawlerHeap, HEAP_ZERO_MEMORY, sizeof (GPTINI_FILE_DATA));
	if (pGptIniFileData == NULL)
	{
		DEBUG_LOG(D_ERROR, "pGptIniFileData pointer invalid.\r\nExiting now...");
		DoExit(D_ERROR);
	}
	pGptIniFileData->tFilePath = tFilePath;
	pGptIniFileData->iNumberOfUnReferrencedSections = 0;
	
	pGeneralSection = GetSectionByName(pGenericIniFileData, TEXT(GPT_GENERAL_SECTION));
	if (!pGeneralSection)
	{
		DEBUG_LOG(D_ERROR, "Unable to retrieve General section for GPT File.\r\nExiting now...");
		DoExit(D_ERROR);
	}

	pVersionProperty = GetPropertyByName(pGeneralSection, TEXT(GPT_GENERAL_VERSION));
	pDisplayNameProperty = GetPropertyByName(pGeneralSection, TEXT(GPT_GENERAL_DISPLAYNAME));
	if (pVersionProperty)
	{
		PWCHAR  tmp = (PWCHAR) HeapAlloc(hCrawlerHeap, HEAP_ZERO_MEMORY, sizeof (PWCHAR) * (_tcslen(pVersionProperty->tValue) + 1));
		_tcscpy_s(tmp, _tcslen(pVersionProperty->tValue) + 1, pVersionProperty->tValue);
		pGptIniFileData->tVersion = tmp;

		if (RemovePropertyInSection(pGeneralSection, pVersionProperty) != TRUE) 
		{
			DEBUG_LOG(D_ERROR, "Unable to delete General properties.\r\nExiting now...");
			DoExit(D_ERROR);
		}
	}
	if (pDisplayNameProperty)
	{
		PWCHAR  tmp = (PWCHAR) HeapAlloc(hCrawlerHeap, HEAP_ZERO_MEMORY, sizeof (PWCHAR) * (_tcslen(pDisplayNameProperty->tValue) + 1));
		_tcscpy_s(tmp, _tcslen(pDisplayNameProperty->tValue) + 1, pDisplayNameProperty->tValue);
		pGptIniFileData->tDisplayName = tmp;

		if (RemovePropertyInSection(pGeneralSection, pDisplayNameProperty) != TRUE)
		{
			DEBUG_LOG(D_ERROR, "Unable to delete General properties.\r\nExiting now...");
			DoExit(D_ERROR);
		}
	}

	if (IsSectionEmpty(pGeneralSection))
	{
		if (RemoveSectionInIniData(pGenericIniFileData, pGeneralSection) != TRUE)
		{
			DEBUG_LOG(D_ERROR, "Unable to delete General section.\r\nExiting now...");
			DoExit(D_ERROR);
		}
	}

	if (pGenericIniFileData->iNumberOfSection)
	{
		pGptIniFileData->iNumberOfUnReferrencedSections = pGenericIniFileData->iNumberOfSection;
		for (DWORD i = 0; i < pGptIniFileData->iNumberOfUnReferrencedSections; ++i)
			pGptIniFileData->pUnReferrencedSections[i] = pGenericIniFileData->pSections[i]; 
		pGenericIniFileData->iNumberOfSection = 0; 
	}

	PrintGptIniDataHeader(pGptIniFileData->tFilePath);
	PrintData(pGptIniFileData);
	PrintGptIniDataFooter(pGptIniFileData->tFilePath);

	if (pbINIRawDATA)
	{
		if (bMemoryAreaMoved == TRUE)
			pbINIRawDATA -=2;
		HeapFree(hCrawlerHeap, NULL, pbINIRawDATA);
	}
	FreeGptIniFileData(pGptIniFileData);
	FreeIniFileData(pGenericIniFileData);
	return TRUE;
}

BOOL FreeGptIniFileData(_Inout_ PGPTINI_FILE_DATA pGptIniFileData)
{
	PINI_SECTION_DATA pCurrentSection = NULL;
	PINI_PROPERTY_DATA pCurrentProperty = NULL;
	
	if (pGptIniFileData == NULL)
	{
		DEBUG_LOG(D_ERROR, "GPTINI_FILE_DATA pointer invalid.\r\nExiting now...");
		DoExit(D_ERROR);
	}

	for (DWORD i = 0; i < pGptIniFileData->iNumberOfUnReferrencedSections; ++i)
	{
		pCurrentSection = pGptIniFileData->pUnReferrencedSections[i];
		if (!pCurrentSection)
			continue;

		for (DWORD j = 0; j < pCurrentSection->iNumberOfProperty; ++j)
		{
			pCurrentProperty = pCurrentSection->pProperties[j];
			if (!pCurrentProperty)
				continue;

			if ((pCurrentProperty->tName) && (_tcscmp(pCurrentProperty->tValue, TEXT(""))))
				HeapFree(hCrawlerHeap, NULL, pCurrentProperty->tName);
			if ((pCurrentProperty->tValue) && (_tcscmp(pCurrentProperty->tValue, TEXT(""))))
				HeapFree(hCrawlerHeap, NULL, pCurrentProperty->tValue);

			HeapFree(hCrawlerHeap, NULL, pCurrentProperty);
		}

		if (pCurrentSection->tSectionName)
			HeapFree(hCrawlerHeap, NULL, pCurrentSection->tSectionName);
		HeapFree(hCrawlerHeap, NULL, pCurrentSection);
	}

	if (pGptIniFileData->tVersion != NULL)	
		HeapFree(hCrawlerHeap, NULL, pGptIniFileData->tVersion);
	if (pGptIniFileData->tDisplayName)
		HeapFree(hCrawlerHeap, NULL, pGptIniFileData->tDisplayName);
	HeapFree(hCrawlerHeap, NULL, pGptIniFileData);
	return TRUE;
}