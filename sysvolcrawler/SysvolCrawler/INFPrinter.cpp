/**************************************************
* SysvolCrawler - INFPrinter.cpp
* AUTHOR: Luc Delsalle	
*
* Affiche le contenu d'un fichier INF 
* sous différents formats
*	
* ANSSI/COSSI/DTO/BAI - 2014
***************************************************/

#include "INFPrinter.h"

BOOL PrintData(_In_ PINF_FILE_DATA pInfData)
{
	BOOL bRes = TRUE;

	if (pInfData == NULL)
	{
		DEBUG_LOG(D_ERROR, "INF_FILE_DATA pointer invalid.\r\nExiting now...");
		DoExit(D_ERROR);
	}

	// Call every printer
	if ((bRes) && (pSyscrwlrOptions->bShouldPrintXML))
		bRes = PrintXMLData(pInfData);

	if ((bRes) && (pSyscrwlrOptions->bShouldPrintCSV))
		bRes = PrintCSVData(pInfData);

	if ((bRes) && (pSyscrwlrOptions->bShouldPrintSTDOUT))
		bRes = PrintSTDOUTData(pInfData);

	return bRes;
}

BOOL PrintInfDataHeader(_In_ PTCHAR tFilePath)
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
		HANDLE hXMLFile = GetFileHandle(OUTPUT_FILE_XML, OUTPUT_DIRECTORY_INF_FILE, OUTPUT_NAME_INF_FILE);

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
			|| (WriteFile(hXMLFile, OUTPUT_NAME_INF_FILE, (DWORD)(_tcslen(OUTPUT_NAME_INF_FILE) * sizeof (TCHAR)), &dwDataRead, NULL) == FALSE)
			|| (WriteFile(hXMLFile, TEXT(".xml>\r\n"), (DWORD)(_tcslen(TEXT(".xml>\r\n")) * sizeof (TCHAR)), &dwDataRead, NULL) == FALSE))
				goto writerror;
		}

		if ((WriteFile(hXMLFile, TEXT("<INFormationFile Filename=\""), (DWORD)(_tcslen(TEXT("<INFormationFile Filename=\"")) * sizeof (TCHAR)), &dwDataRead, NULL) == FALSE)
		|| (WriteFile(hXMLFile, tFilePath, (DWORD)(_tcslen(tFilePath) * sizeof (TCHAR)), &dwDataRead, NULL) == FALSE)
		|| (WriteFile(hXMLFile, TEXT("\">\r\n"), (DWORD)(_tcslen(TEXT("\">\r\n")) * sizeof (TCHAR)), &dwDataRead, NULL) == FALSE))
			goto writerror;

		CloseHandle(hXMLFile);
	}

	if (pSyscrwlrOptions->bShouldPrintCSV)
	{
		HANDLE hCSVFile = GetFileHandle(OUTPUT_FILE_CSV, OUTPUT_DIRECTORY_INF_FILE, OUTPUT_NAME_INF_FILE);
		LARGE_INTEGER liFileSize;

		if (!GetFileSizeEx(hCSVFile, &liFileSize))
		{
			DEBUG_LOG(D_WARNING, "Unable to determine file size.\r\nExiting now...");
			DoExit(D_ERROR);
		}

		if ((liFileSize.HighPart == 0) && (liFileSize.LowPart == 0))
		{
			if (WriteFile(hCSVFile, TEXT("File;Section;Name;Value\r\n"), (DWORD)(_tcslen(TEXT("File;Section;Name;Value\r\n")) * sizeof (TCHAR)), &dwDataRead, NULL) == FALSE)
				goto writerror;
		}
		CloseHandle(hCSVFile);
	}
	return TRUE;

writerror:
	DEBUG_LOG(D_WARNING, "Unable to write DATA HEADER for INF printer.\r\nExiting now...");
	DoExit(D_ERROR);
	return FALSE;
}

BOOL PrintInfDataFooter(_In_ PTCHAR tFilePath)
{
	DWORD dwDataRead = 0;

	// Hack for closing xml document. Ugly.
	if (pSyscrwlrOptions->bShouldPrintXML)
	{
		HANDLE hXMLFile = GetFileHandle(OUTPUT_FILE_XML, OUTPUT_DIRECTORY_INF_FILE, OUTPUT_NAME_INF_FILE);
		if (WriteFile(hXMLFile, TEXT("</INFormationFile>\r\n"), (DWORD)(_tcslen(TEXT("</INFormationFile>\r\n")) * sizeof (TCHAR)), &dwDataRead, NULL) == FALSE)
			goto writerror;
		CloseHandle(hXMLFile);
	}
	return TRUE;

writerror:
	DEBUG_LOG(D_WARNING, "Unable to write DATA FOOTER for INF printer.\r\nExiting now...");
	DoExit(D_ERROR);
	return FALSE;
}

BOOL PrintXMLData(_In_ PINF_FILE_DATA pInfData)
{
	DWORD dwDataRead = 0;
	HANDLE hXMLFile = GetFileHandle(OUTPUT_FILE_XML, OUTPUT_DIRECTORY_INF_FILE, OUTPUT_NAME_INF_FILE);
	
	if (!pInfData || !(pInfData->tFilePath))
	{
		DEBUG_LOG(D_WARNING, "PINF_FILE_DATA invalid for INF file.\r\n");
		DoExit(D_WARNING);
	}

	if (hXMLFile == INVALID_HANDLE_VALUE)
	{
		DEBUG_LOG(D_WARNING, "Handle to hXMLFile is invalid.\r\nExiting now...");
		DoExit(D_ERROR);
	}

	for (DWORD i = 0; i < pInfData->iNumberOfSection; ++i)
	{
		PINF_SECTION_DATA pCurrSectionData = pInfData->pSections[i];

		if (!pCurrSectionData->tSectionName)
			continue;

		if ((WriteFile(hXMLFile, TEXT("\t<Section name=\""), (DWORD)(_tcslen(TEXT("\t<Section name=\"")) * sizeof (TCHAR)), &dwDataRead, NULL) == FALSE)
			|| (WriteFile(hXMLFile, pCurrSectionData->tSectionName, (DWORD)(_tcslen(pCurrSectionData->tSectionName) * sizeof (WCHAR)), &dwDataRead, NULL) == FALSE)
			|| (WriteFile(hXMLFile, TEXT("\">\r\n"), (DWORD)(_tcslen(TEXT("\">\r\n")) * sizeof (TCHAR)), &dwDataRead, NULL) == FALSE))
			goto writerror;

		PrintXMLSectionData(pCurrSectionData, hXMLFile);

		if (WriteFile(hXMLFile, TEXT("\t</Section>\r\n"), (DWORD)(_tcslen(TEXT("\t</Section>\r\n")) * sizeof (TCHAR)), &dwDataRead, NULL) == FALSE)
			goto writerror;
	}

	CloseHandle(hXMLFile);
	return TRUE;

writerror:
	DEBUG_LOG(D_WARNING, "Unable to write XML DATA.\r\nExiting now...");
	DoExit(D_ERROR);
	return FALSE;
}

BOOL PrintXMLSectionData(_In_ PINF_SECTION_DATA pSectionData, _In_ HANDLE hXMLFile)
{
	DWORD dwDataRead = 0;

	if (!pSectionData || !(pSectionData->tSectionName))
	{
		DEBUG_LOG(D_WARNING, "PINF_SECTION_DATA invalid for INF file.\r\n");
		DoExit(D_WARNING);
	}

	if (hXMLFile == INVALID_HANDLE_VALUE)
	{
		DEBUG_LOG(D_WARNING, "Handle to hXMLFile is invalid.\r\nExiting now...");
		DoExit(D_ERROR);
	}

	for (DWORD i = 0; i < pSectionData->iNumberOfProperty; ++i)
	{
		PINF_PROPERTY_DATA pCurrPropertyData = pSectionData->pProperties[i];
		PTCHAR tEscapedValue = EscapeXMLString(pCurrPropertyData->tValue);
		PTCHAR tEscapedName = EscapeXMLString(pCurrPropertyData->tName);

		if (!pCurrPropertyData->tName)
			continue;

		if ((WriteFile(hXMLFile, TEXT("\t\t<Property name=\""), (DWORD)(_tcslen(TEXT("\t\t<Property name=\"")) * sizeof (TCHAR)), &dwDataRead, NULL) == FALSE)
			|| (WriteFile(hXMLFile, tEscapedName, (DWORD)(_tcslen(tEscapedName) * sizeof (WCHAR)), &dwDataRead, NULL) == FALSE)
			|| (WriteFile(hXMLFile, TEXT("\" value=\""), (DWORD)(_tcslen(TEXT("\" value=\"")) * sizeof (TCHAR)), &dwDataRead, NULL) == FALSE)
			|| (WriteFile(hXMLFile, tEscapedValue, (DWORD)(_tcslen(tEscapedValue) * sizeof (WCHAR)), &dwDataRead, NULL) == FALSE)
			|| (WriteFile(hXMLFile, TEXT("\"/>\r\n"), (DWORD)(_tcslen(TEXT("\"/>\r\n")) * sizeof (TCHAR)), &dwDataRead, NULL) == FALSE))
			goto writerror;

		if (tEscapedValue)
			HeapFree(hCrawlerHeap, NULL, tEscapedValue);
		if (tEscapedName)
			HeapFree(hCrawlerHeap, NULL, tEscapedName);
	}

	return TRUE;
writerror:
	DEBUG_LOG(D_WARNING, "Unable to write XML DATA.\r\nExiting now...");
	DoExit(D_ERROR);
	return FALSE;
}

BOOL PrintCSVData(_In_ PINF_FILE_DATA pInfData)
{
	DWORD dwDataRead = 0;
	HANDLE hCSVFile = GetFileHandle(OUTPUT_FILE_CSV, OUTPUT_DIRECTORY_INF_FILE, OUTPUT_NAME_INF_FILE);
	
	if (!pInfData || !(pInfData->tFilePath))
	{
		DEBUG_LOG(D_WARNING, "PINF_FILE_DATA invalid for INF file.\r\n");
		DoExit(D_WARNING);
	}

	if (hCSVFile == INVALID_HANDLE_VALUE)
	{
		DEBUG_LOG(D_WARNING, "Handle to hCSVFile is invalid.\r\nExiting now...");
		DoExit(D_ERROR);
	}

	for (DWORD i = 0; i < pInfData->iNumberOfSection; ++i)
	{
		PINF_SECTION_DATA pCurrSectionData = pInfData->pSections[i];

		if (!pCurrSectionData->tSectionName)
			continue;

		PrintCSVSectionData(pInfData, pCurrSectionData, hCSVFile);
	}

	CloseHandle(hCSVFile);
	return TRUE;
}

BOOL PrintCSVSectionData(_In_ PINF_FILE_DATA pInfData, _In_ PINF_SECTION_DATA pSectionData, _In_ HANDLE hCSVFile)
{
	DWORD dwDataRead = 0;

	if (!pSectionData || !(pSectionData->tSectionName))
	{
		DEBUG_LOG(D_WARNING, "PINF_SECTION_DATA invalid for INF file.\r\n");
		DoExit(D_WARNING);
	}

	if (hCSVFile == INVALID_HANDLE_VALUE)
	{
		DEBUG_LOG(D_WARNING, "Handle to hCSVFile is invalid.\r\nExiting now...");
		DoExit(D_ERROR);
	}

	for (DWORD i = 0; i < pSectionData->iNumberOfProperty; ++i)
	{
		PINF_PROPERTY_DATA pCurrPropertyData = pSectionData->pProperties[i];
		PTCHAR tEscapedValue = EscapeCSVString(pCurrPropertyData->tValue);
		PTCHAR tEscapedName = EscapeCSVString(pCurrPropertyData->tName);

		if (!pCurrPropertyData->tName)
			continue;

		if ((WriteFile(hCSVFile, pInfData->tFilePath, (DWORD)(_tcslen(pInfData->tFilePath) * sizeof (WCHAR)), &dwDataRead, NULL) == FALSE)
			|| (WriteFile(hCSVFile, TEXT(";"), (DWORD)(_tcslen(TEXT(";")) * sizeof (TCHAR)), &dwDataRead, NULL) == FALSE)
			|| (WriteFile(hCSVFile, pSectionData->tSectionName, (DWORD)(_tcslen(pSectionData->tSectionName) * sizeof (WCHAR)), &dwDataRead, NULL) == FALSE)
			|| (WriteFile(hCSVFile, TEXT(";"), (DWORD)(_tcslen(TEXT(";")) * sizeof (TCHAR)), &dwDataRead, NULL) == FALSE)
			|| (WriteFile(hCSVFile, tEscapedName, (DWORD)(_tcslen(tEscapedName) * sizeof (WCHAR)), &dwDataRead, NULL) == FALSE)
			|| (WriteFile(hCSVFile, TEXT(";"), (DWORD)(_tcslen(TEXT(";")) * sizeof (TCHAR)), &dwDataRead, NULL) == FALSE)
			|| (WriteFile(hCSVFile, tEscapedValue, (DWORD)(_tcslen(tEscapedValue) * sizeof (WCHAR)), &dwDataRead, NULL) == FALSE)
			|| (WriteFile(hCSVFile, TEXT("\r\n"), (DWORD)(_tcslen(TEXT("\r\n")) * sizeof (TCHAR)), &dwDataRead, NULL) == FALSE))
			goto writerror;

		if (tEscapedValue)
			HeapFree(hCrawlerHeap, NULL, tEscapedValue);
		if (tEscapedName)
			HeapFree(hCrawlerHeap, NULL, tEscapedName);
	}

	return TRUE;
writerror:
	DEBUG_LOG(D_WARNING, "Unable to write CSV DATA.\r\nExiting now...");
	DoExit(D_ERROR);
	return FALSE;
}

BOOL PrintSTDOUTData(_In_ PINF_FILE_DATA pInfData)
{
	DWORD dwDataRead = 0;

	if (!pInfData || !(pInfData->tFilePath))
	{
		DEBUG_LOG(D_WARNING, "PINF_FILE_DATA invalid for INF file.\r\n");
		DoExit(D_WARNING);
	}

	for (DWORD i = 0; i < pInfData->iNumberOfSection; ++i)
	{
		PINF_SECTION_DATA pCurrSectionData = pInfData->pSections[i];

		if (!pCurrSectionData->tSectionName)
			continue;

		PrintSTDOUTSectionData(pInfData, pCurrSectionData);
	}
	return TRUE;
}

BOOL PrintSTDOUTSectionData(_In_ PINF_FILE_DATA pInfData, _In_ PINF_SECTION_DATA pSectionData)
{
	DWORD dwDataRead = 0;
	
	if (!pSectionData || !(pSectionData->tSectionName))
	{
		DEBUG_LOG(D_WARNING, "PINF_SECTION_DATA invalid for INF file.\r\n");
		DoExit(D_WARNING);
	}

	for (DWORD i = 0; i < pSectionData->iNumberOfProperty; ++i)
	{
		PINF_PROPERTY_DATA pCurrPropertyData = pSectionData->pProperties[i];

		if (!pCurrPropertyData->tName)
			continue;

		printf("[INF] File=%ws SectionName=%ws PropertyName=%ws Value=%ws\r\n", pInfData->tFilePath, pSectionData->tSectionName, pCurrPropertyData->tName, pCurrPropertyData->tValue);	
	}

	return TRUE;
}