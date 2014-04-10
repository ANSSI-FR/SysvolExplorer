/**************************************************
* SysvolCrawler - GPTiniPrinter.cpp
* AUTHOR: Luc Delsalle	
*
* Display or export GPT.ini file data
*	
* ANSSI/COSSI/DTO/BAI - 2014
***************************************************/

#include "GPTiniPrinter.h"

BOOL PrintData(_In_ PGPTINI_FILE_DATA pGptIniData)
{
	BOOL bRes = TRUE;

	if (pGptIniData == NULL)
	{
		DEBUG_LOG(D_ERROR, "pGptIniData pointer invalid.\r\nExiting now...");
		DoExit(D_ERROR);
	}

	// Call every printer
	if ((bRes) && (pSyscrwlrOptions->bShouldPrintXML))
		bRes = PrintXMLData(pGptIniData);

	if ((bRes) && (pSyscrwlrOptions->bShouldPrintCSV))
		bRes = PrintCSVData(pGptIniData);

	if ((bRes) && (pSyscrwlrOptions->bShouldPrintSTDOUT))
		bRes = PrintSTDOUTData(pGptIniData);

	return bRes;
}

BOOL PrintGptIniDataHeader(_In_ PTCHAR tFilePath)
{
	DWORD dwDataRead = 0;
	LARGE_INTEGER liFileSize;

	if (!tFilePath)
	{
		DEBUG_LOG(D_WARNING, "tFilePath is invalid.\r\nExiting now...");
		DoExit(D_ERROR);
	}

	// Hack for closing xml document. Ugly.
	if (pSyscrwlrOptions->bShouldPrintXML)
	{
		HANDLE hXMLFile = GetFileHandle(OUTPUT_FILE_XML, OUTPUT_DIRECTORY_GPT_INI, OUTPUT_NAME_GPT_INI);

		if (!GetFileSizeEx(hXMLFile, &liFileSize))
		{
			DEBUG_LOG(D_WARNING, "Unable to determine file size.\r\nExiting now...");
			DoExit(D_ERROR);
		}

		if ((liFileSize.HighPart == 0) && (liFileSize.LowPart == 0))
		{
			// New file, we need to add xml header
			if (WriteFile(hXMLFile, TEXT("<?xml version=\"1.0\"?>\r\n"), (DWORD)(_tcslen(TEXT("<?xml version=\"1.0\"?>\r\n")) * sizeof (TCHAR)), &dwDataRead, NULL) == FALSE)
				goto writerror;

			if ((WriteFile(hXMLFile, TEXT("<"), (DWORD)(_tcslen(TEXT("<")) * sizeof (TCHAR)), &dwDataRead, NULL) == FALSE)
			|| (WriteFile(hXMLFile, OUTPUT_NAME_GPT_INI, (DWORD)(_tcslen(OUTPUT_NAME_GPT_INI) * sizeof (TCHAR)), &dwDataRead, NULL) == FALSE)
			|| (WriteFile(hXMLFile, TEXT(".xml>\r\n"), (DWORD)(_tcslen(TEXT(".xml>\r\n")) * sizeof (TCHAR)), &dwDataRead, NULL) == FALSE))
				goto writerror;
		}

		if ((WriteFile(hXMLFile, TEXT("<GPTiniFile Filename=\""), (DWORD)(_tcslen(TEXT("<GPTiniFile Filename=\"")) * sizeof (TCHAR)), &dwDataRead, NULL) == FALSE)
		|| (WriteFile(hXMLFile, tFilePath, (DWORD)(_tcslen(tFilePath) * sizeof (TCHAR)), &dwDataRead, NULL) == FALSE)
		|| (WriteFile(hXMLFile, TEXT("\">\r\n"), (DWORD)(_tcslen(TEXT("\">\r\n")) * sizeof (TCHAR)), &dwDataRead, NULL) == FALSE))
			goto writerror;

		CloseHandle(hXMLFile);
	}

	if (pSyscrwlrOptions->bShouldPrintCSV)
	{
		HANDLE hCSVFile = GetFileHandle(OUTPUT_FILE_CSV, OUTPUT_DIRECTORY_GPT_INI, OUTPUT_NAME_GPT_INI);
		LARGE_INTEGER liFileSize;

		if (!GetFileSizeEx(hCSVFile, &liFileSize))
		{
			DEBUG_LOG(D_WARNING, "Unable to determine file size.\r\nExiting now...");
			DoExit(D_ERROR);
		}

		if ((liFileSize.HighPart == 0) && (liFileSize.LowPart == 0))
		{
			if (WriteFile(hCSVFile, TEXT("File;Version;DisplayName;Unreferrenced\r\n"), (DWORD)(_tcslen(TEXT("File;Version;DisplayName;Unreferrenced\r\n")) * sizeof (TCHAR)), &dwDataRead, NULL) == FALSE)
				goto writerror;
		}
		CloseHandle(hCSVFile);
	}
	return TRUE;

writerror:
	DEBUG_LOG(D_WARNING, "Unable to write DATA HEADER for GPT.ini printer.\r\nExiting now...");
	DoExit(D_ERROR);
	return FALSE;
}

BOOL PrintGptIniDataFooter(_In_ PTCHAR tFilePath)
{
	DWORD dwDataRead = 0;

	// Hack for closing xml document. Ugly.
	if (pSyscrwlrOptions->bShouldPrintXML)
	{
		HANDLE hXMLFile = GetFileHandle(OUTPUT_FILE_XML, OUTPUT_DIRECTORY_GPT_INI, OUTPUT_NAME_GPT_INI);
		if (WriteFile(hXMLFile, TEXT("</GPTiniFile>\r\n"), (DWORD)(_tcslen(TEXT("</GPTiniFile>\r\n")) * sizeof (TCHAR)), &dwDataRead, NULL) == FALSE)
			goto writerror;
		CloseHandle(hXMLFile);
	}
	return TRUE;

writerror:
	DEBUG_LOG(D_WARNING, "Unable to write DATA FOOTER for GPT.ini printer.\r\nExiting now...");
	DoExit(D_ERROR);
	return FALSE;
}

BOOL PrintXMLData(_In_ PGPTINI_FILE_DATA pGptIniData)
{
	DWORD dwDataRead = 0;
	HANDLE hXMLFile = GetFileHandle(OUTPUT_FILE_XML, OUTPUT_DIRECTORY_GPT_INI, OUTPUT_NAME_GPT_INI);
	PTCHAR tVersion = NULL;
	PTCHAR tDisplayName = NULL;	

	if (!pGptIniData || !(pGptIniData->tFilePath))
	{
		DEBUG_LOG(D_WARNING, "PGPTINI_FILE_DATA invalid for GPT.ini file.\r\n");
		DoExit(D_WARNING);
	}

	if (hXMLFile == INVALID_HANDLE_VALUE)
	{
		DEBUG_LOG(D_WARNING, "Handle to hXMLFile is invalid.\r\nExiting now...");
		DoExit(D_ERROR);
	}

	if (!pGptIniData->tVersion)
		tVersion = TEXT("NOT FOUND");
	else
		tVersion = pGptIniData->tVersion;

	if (!pGptIniData->tDisplayName)
		tDisplayName = TEXT("NOT FOUND");
	else
		tDisplayName = pGptIniData->tDisplayName;

	if ((WriteFile(hXMLFile, TEXT("\t<GPO version=\""), (DWORD)(_tcslen(TEXT("\t<GPO version=\"")) * sizeof (TCHAR)), &dwDataRead, NULL) == FALSE)
		|| (WriteFile(hXMLFile, tVersion, (DWORD)(_tcslen(tVersion) * sizeof (WCHAR)), &dwDataRead, NULL) == FALSE)
		|| (WriteFile(hXMLFile, TEXT("\" displayname=\""), (DWORD)(_tcslen(TEXT("\" displayname=\"")) * sizeof (TCHAR)), &dwDataRead, NULL) == FALSE)
		|| (WriteFile(hXMLFile, tDisplayName, (DWORD)(_tcslen(tDisplayName) * sizeof (WCHAR)), &dwDataRead, NULL) == FALSE)
		|| (WriteFile(hXMLFile, TEXT("\"/>\r\n"), (DWORD)(_tcslen(TEXT("\"/>\r\n")) * sizeof (TCHAR)), &dwDataRead, NULL) == FALSE))
		goto writerror;

	for (DWORD i = 0; i < pGptIniData->iNumberOfUnReferrencedSections; ++i)
	{
		PINI_SECTION_DATA pCurrSectionData = pGptIniData->pUnReferrencedSections[i];

		if (!pCurrSectionData->tSectionName)
			continue;

		PrintXMLUnreferrencedSectionDataInGPT(pCurrSectionData, hXMLFile);
	}	

	CloseHandle(hXMLFile);
	return TRUE;
writerror:
	DEBUG_LOG(D_WARNING, "Unable to write XML DATA.\r\nExiting now...");
	DoExit(D_ERROR);
	return FALSE;
}

BOOL PrintXMLUnreferrencedSectionDataInGPT(_In_ PINI_SECTION_DATA pSectionData, _In_ HANDLE hXMLFile)
{
	DWORD dwDataRead = 0;

	if (!pSectionData || !(pSectionData->tSectionName))
	{
		DEBUG_LOG(D_WARNING, "PINI_SECTION_DATA invalid for GPT.ini file.\r\n");
		DoExit(D_WARNING);
	}

	if (hXMLFile == INVALID_HANDLE_VALUE)
	{
		DEBUG_LOG(D_WARNING, "Handle to hXMLFile is invalid.\r\nExiting now...");
		DoExit(D_ERROR);
	}

	if ((WriteFile(hXMLFile, TEXT("\t<Unreferrenced name=\""), (DWORD)(_tcslen(TEXT("\t<Unreferrenced name=\"")) * sizeof (TCHAR)), &dwDataRead, NULL) == FALSE)
		|| (WriteFile(hXMLFile, pSectionData->tSectionName, (DWORD)(_tcslen(pSectionData->tSectionName) * sizeof (WCHAR)), &dwDataRead, NULL) == FALSE)
		|| (WriteFile(hXMLFile, TEXT("\">\r\n"), (DWORD)(_tcslen(TEXT("\">\r\n")) * sizeof (TCHAR)), &dwDataRead, NULL) == FALSE))
		goto writerror;

	for (DWORD i = 0; i < pSectionData->iNumberOfProperty; ++i)
	{
		PINI_PROPERTY_DATA pCurrPropertyData = pSectionData->pProperties[i];

		if (!pCurrPropertyData->tName)
			continue;

		if ((WriteFile(hXMLFile, TEXT("\t\t<Property name=\""), (DWORD)(_tcslen(TEXT("\t\t<Property name=\"")) * sizeof (TCHAR)), &dwDataRead, NULL) == FALSE)
			|| (WriteFile(hXMLFile, pCurrPropertyData->tName, (DWORD)(_tcslen(pCurrPropertyData->tName) * sizeof (WCHAR)), &dwDataRead, NULL) == FALSE)
			|| (WriteFile(hXMLFile, TEXT("\" value=\""), (DWORD)(_tcslen(TEXT("\" value=\"")) * sizeof (TCHAR)), &dwDataRead, NULL) == FALSE)
			|| (WriteFile(hXMLFile, pCurrPropertyData->tValue, (DWORD)(_tcslen(pCurrPropertyData->tValue) * sizeof (WCHAR)), &dwDataRead, NULL) == FALSE)
			|| (WriteFile(hXMLFile, TEXT("\"/>\r\n"), (DWORD)(_tcslen(TEXT("\"/>\r\n")) * sizeof (TCHAR)), &dwDataRead, NULL) == FALSE))
			goto writerror;
	}

	if (WriteFile(hXMLFile, TEXT("\t</Unreferrenced>\r\n"), (DWORD)(_tcslen(TEXT("\t</Unreferrenced>\r\n")) * sizeof (TCHAR)), &dwDataRead, NULL) == FALSE)
		goto writerror;


	return TRUE;
writerror:
	DEBUG_LOG(D_WARNING, "Unable to write XML DATA.\r\nExiting now...");
	DoExit(D_ERROR);
	return FALSE;
}

BOOL PrintCSVData(_In_ PGPTINI_FILE_DATA pGptIniData)
{
	DWORD dwDataRead = 0;
	HANDLE hCSVFile = GetFileHandle(OUTPUT_FILE_CSV, OUTPUT_DIRECTORY_GPT_INI, OUTPUT_NAME_GPT_INI);
	PTCHAR tVersion = NULL;
	PTCHAR tDisplayName = NULL;

	if (!pGptIniData || !(pGptIniData->tFilePath))
	{
		DEBUG_LOG(D_WARNING, "PGPTINI_FILE_DATA invalid for CSV file.\r\n");
		DoExit(D_WARNING);
	}

	if (hCSVFile == INVALID_HANDLE_VALUE)
	{
		DEBUG_LOG(D_WARNING, "Handle to hCSVFile is invalid.\r\nExiting now...");
		DoExit(D_ERROR);
	}

	if (!pGptIniData->tVersion)
		tVersion = TEXT("NOT FOUND");
	else
		tVersion = pGptIniData->tVersion;

	if (!pGptIniData->tDisplayName)
		tDisplayName = TEXT("NOT FOUND");
	else
		tDisplayName = pGptIniData->tDisplayName;

	if ((WriteFile(hCSVFile, pGptIniData->tFilePath, (DWORD)(_tcslen(pGptIniData->tFilePath) * sizeof (WCHAR)), &dwDataRead, NULL) == FALSE)
		|| (WriteFile(hCSVFile, TEXT(";"), (DWORD)(_tcslen(TEXT(";")) * sizeof (TCHAR)), &dwDataRead, NULL) == FALSE)
		|| (WriteFile(hCSVFile, tVersion, (DWORD)(_tcslen(tVersion) * sizeof (WCHAR)), &dwDataRead, NULL) == FALSE)
		|| (WriteFile(hCSVFile, TEXT(";"), (DWORD)(_tcslen(TEXT(";")) * sizeof (TCHAR)), &dwDataRead, NULL) == FALSE)
		|| (WriteFile(hCSVFile, tDisplayName, (DWORD)(_tcslen(tDisplayName) * sizeof (WCHAR)), &dwDataRead, NULL) == FALSE))
		goto writerror;	

	for (DWORD i = 0; i < pGptIniData->iNumberOfUnReferrencedSections; ++i)
	{
		PINI_SECTION_DATA pCurrSectionData = pGptIniData->pUnReferrencedSections[i];

		if (!pCurrSectionData->tSectionName)
			continue;

		PrintCSVUnreferrencedSectionDataInGPT(pCurrSectionData, hCSVFile);
	}

	if (WriteFile(hCSVFile, TEXT("\r\n"), (DWORD)(_tcslen(TEXT("\r\n")) * sizeof (TCHAR)), &dwDataRead, NULL) == FALSE)
		goto writerror;	

	CloseHandle(hCSVFile);
	return TRUE;
writerror:
	DEBUG_LOG(D_WARNING, "Unable to write CSV DATA.\r\nExiting now...");
	DoExit(D_ERROR);
	return FALSE;
}

BOOL PrintCSVUnreferrencedSectionDataInGPT(_In_ PINI_SECTION_DATA pSectionData, _In_ HANDLE hCSVFile)
{
	DWORD dwDataRead = 0;

	if (!pSectionData || !(pSectionData->tSectionName))
	{
		DEBUG_LOG(D_WARNING, "PINI_SECTION_DATA invalid for GPT.ini file.\r\n");
		DoExit(D_WARNING);
	}

	if (hCSVFile == INVALID_HANDLE_VALUE)
	{
		DEBUG_LOG(D_WARNING, "Handle to hCSVFile is invalid.\r\nExiting now...");
		DoExit(D_ERROR);
	}

	if ((WriteFile(hCSVFile, TEXT(";["), (DWORD)(_tcslen(TEXT(";[")) * sizeof (TCHAR)), &dwDataRead, NULL) == FALSE)
		|| (WriteFile(hCSVFile, pSectionData->tSectionName, (DWORD)(_tcslen(pSectionData->tSectionName) * sizeof (WCHAR)), &dwDataRead, NULL) == FALSE)
		|| (WriteFile(hCSVFile, TEXT(":"), (DWORD)(_tcslen(TEXT(":")) * sizeof (WCHAR)), &dwDataRead, NULL) == FALSE))
		goto writerror;	

	for (DWORD i = 0; i < pSectionData->iNumberOfProperty; ++i)
	{
		PINI_PROPERTY_DATA pCurrPropertyData = pSectionData->pProperties[i];

		if (!pCurrPropertyData->tName)
			continue;

		if ((WriteFile(hCSVFile, TEXT("("), (DWORD)(_tcslen(TEXT("(")) * sizeof (WCHAR)), &dwDataRead, NULL) == FALSE)
			|| (WriteFile(hCSVFile, pCurrPropertyData->tName, (DWORD)(_tcslen(pCurrPropertyData->tName) * sizeof (WCHAR)), &dwDataRead, NULL) == FALSE)
			|| (WriteFile(hCSVFile, TEXT(","), (DWORD)(_tcslen(TEXT(",")) * sizeof (TCHAR)), &dwDataRead, NULL) == FALSE)
			|| (WriteFile(hCSVFile, pCurrPropertyData->tValue, (DWORD)(_tcslen(pCurrPropertyData->tValue) * sizeof (WCHAR)), &dwDataRead, NULL) == FALSE)
			|| (WriteFile(hCSVFile, TEXT(")"), (DWORD)(_tcslen(TEXT(")")) * sizeof (WCHAR)), &dwDataRead, NULL) == FALSE))
			goto writerror;		
	}

	if ((WriteFile(hCSVFile, TEXT("]"), (DWORD)(_tcslen(TEXT("]")) * sizeof (WCHAR)), &dwDataRead, NULL) == FALSE))
		goto writerror;	

	return TRUE;

writerror:
	DEBUG_LOG(D_WARNING, "Unable to write CSV DATA.\r\nExiting now...");
	DoExit(D_ERROR);
	return FALSE;
}

BOOL PrintSTDOUTData(_In_ PGPTINI_FILE_DATA pGptIniData)
{
	DWORD dwDataRead = 0;
	PTCHAR tVersion = NULL;
	PTCHAR tDisplayName = NULL;
	
	if (!pGptIniData || !(pGptIniData->tFilePath))
	{
		DEBUG_LOG(D_WARNING, "PGPTINI_FILE_DATA invalid for GPT.ini file.\r\n");
		DoExit(D_WARNING);
	}

	if (!pGptIniData->tVersion)
		tVersion = TEXT("0");
	else
		tVersion = pGptIniData->tVersion;

	if (!pGptIniData->tDisplayName)
	{
		tDisplayName = (PTCHAR) HeapAlloc(hCrawlerHeap, HEAP_ZERO_MEMORY, sizeof (WCHAR) * 1);
		tDisplayName[0] = TEXT('\0');
	}
	else
		tDisplayName = pGptIniData->tDisplayName;

	printf("[GPTini] File=%ws Version=%ws DisplayName=%ws", pGptIniData->tFilePath, tVersion, tDisplayName);

	for (DWORD i = 0; i < pGptIniData->iNumberOfUnReferrencedSections; ++i)
	{
		PINI_SECTION_DATA pCurrSectionData = pGptIniData->pUnReferrencedSections[i];

		if (i == 0)
			printf(" UnreferrencedSection=");

		PrintSTDOUTSectionData(pCurrSectionData);
	}
	printf("\r\n");

	return TRUE;
}

BOOL PrintSTDOUTSectionData(_In_ PINI_SECTION_DATA pSectionData)
{
	DWORD dwDataRead = 0;
	
	if (!pSectionData || !(pSectionData->tSectionName))
	{
		DEBUG_LOG(D_WARNING, "PINF_SECTION_DATA invalid for INI file.\r\n");
		DoExit(D_WARNING);
	}

	printf("[SectionName=%ws Properties=", pSectionData->tSectionName);

	for (DWORD i = 0; i < pSectionData->iNumberOfProperty; ++i)
	{
		PINI_PROPERTY_DATA pCurrPropertyData = pSectionData->pProperties[i];

		if (!pCurrPropertyData->tName || !pCurrPropertyData->tValue)
			continue;

		printf("(%ws, %ws)", pCurrPropertyData->tName, pCurrPropertyData->tValue);
	}
	printf("]");

	return TRUE;
}