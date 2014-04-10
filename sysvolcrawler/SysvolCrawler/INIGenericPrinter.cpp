/**************************************************
* SysvolCrawler - INIGenericPrinter.h
* AUTHOR: Luc Delsalle	
*
* Generic INI file printer
*
* ANSSI/COSSI/DTO/BAI - 2014
***************************************************/

#include "INIGenericPrinter.h"

BOOL PrintData(_In_ PINI_FILE_DATA pIniFileData, _In_ HANDLE hXMLFile, _In_ HANDLE hCSVFile)
{
	BOOL bRes = TRUE;

	if (pIniFileData == NULL)
	{
		DEBUG_LOG(D_ERROR, "pIniFileData pointer invalid.\r\nExiting now...");
		DoExit(D_ERROR);
	}

	// Call every printer
	if ((bRes) && (pSyscrwlrOptions->bShouldPrintXML))
		bRes = PrintXMLData(pIniFileData, hXMLFile);

	if ((bRes) && (pSyscrwlrOptions->bShouldPrintCSV))
		bRes = PrintCSVData(pIniFileData, hCSVFile);

	if ((bRes) && (pSyscrwlrOptions->bShouldPrintSTDOUT))
		bRes = PrintSTDOUTData(pIniFileData);

	return bRes;
}

BOOL PrintIniDataHeader(_In_ PTCHAR tFilePath, _In_ HANDLE hXMLFile, _In_ HANDLE hCSVFile)
{
	DWORD dwDataRead = 0;

	if (!tFilePath)
	{
		DEBUG_LOG(D_WARNING, "tFilePath is invalid.\r\nExiting now...");
		DoExit(D_ERROR);
	}

	// Hack for closing xml document. Ugly.
	if (pSyscrwlrOptions->bShouldPrintXML && hXMLFile)
	{
		if ((WriteFile(hXMLFile, TEXT("<IniFile Filename=\""), (DWORD)(_tcslen(TEXT("<GPTiniFile Filename=\"")) * sizeof (TCHAR)), &dwDataRead, NULL) == FALSE)
		|| (WriteFile(hXMLFile, tFilePath, (DWORD)(_tcslen(tFilePath) * sizeof (TCHAR)), &dwDataRead, NULL) == FALSE)
		|| (WriteFile(hXMLFile, TEXT("\">\r\n"), (DWORD)(_tcslen(TEXT("\">\r\n")) * sizeof (TCHAR)), &dwDataRead, NULL) == FALSE))
			goto writerror;
	}

	if (pSyscrwlrOptions->bShouldPrintCSV  && hCSVFile)
	{
		if (WriteFile(hCSVFile, TEXT("File;Section;Name;Value\r\n"), (DWORD)(_tcslen(TEXT("File;Section;Name;Value\r\n")) * sizeof (TCHAR)), &dwDataRead, NULL) == FALSE)
			goto writerror;
	}
	return TRUE;

writerror:
	DEBUG_LOG(D_WARNING, "Unable to write DATA HEADER for generic ini printer.\r\nExiting now...");
	DoExit(D_ERROR);
	return FALSE;
}

BOOL PrintIniDataFooter(_In_ PTCHAR tFilePath)
{
	// Nothing to do in case of generic parser
	return TRUE;
}

BOOL PrintXMLData(_In_ PINI_FILE_DATA pIniFileData, _In_ HANDLE hXMLFile)
{
	DWORD dwDataRead = 0;
	
	if (!pIniFileData || !(pIniFileData->tFilePath))
	{
		DEBUG_LOG(D_WARNING, "PINI_FILE_DATA invalid for INI file.\r\n");
		DoExit(D_WARNING);
	}

	if (hXMLFile == INVALID_HANDLE_VALUE)
	{
		DEBUG_LOG(D_WARNING, "Handle to hXMLFile is invalid.\r\nExiting now...");
		DoExit(D_ERROR);
	}

	for (DWORD i = 0; i < pIniFileData->iNumberOfSection; ++i)
	{
		PINI_SECTION_DATA pCurrSectionData = pIniFileData->pSections[i];

		if (!pCurrSectionData->tSectionName)
			continue;

		if ((WriteFile(hXMLFile, TEXT("\t\t<Section name=\""), (DWORD)(_tcslen(TEXT("\t\t<Section name=\"")) * sizeof (TCHAR)), &dwDataRead, NULL) == FALSE)
			|| (WriteFile(hXMLFile, pCurrSectionData->tSectionName, (DWORD)(_tcslen(pCurrSectionData->tSectionName) * sizeof (WCHAR)), &dwDataRead, NULL) == FALSE)
			|| (WriteFile(hXMLFile, TEXT("\">\r\n"), (DWORD)(_tcslen(TEXT("\">\r\n")) * sizeof (TCHAR)), &dwDataRead, NULL) == FALSE))
			goto writerror;

		PrintXMLSectionData(pCurrSectionData, hXMLFile);

		if (WriteFile(hXMLFile, TEXT("\t\t</Section>\r\n"), (DWORD)(_tcslen(TEXT("\t\t</Section>\r\n")) * sizeof (TCHAR)), &dwDataRead, NULL) == FALSE)
			goto writerror;
	}

	return TRUE;

writerror:
	DEBUG_LOG(D_WARNING, "Unable to write XML DATA.\r\nExiting now...");
	DoExit(D_ERROR);
	return FALSE;
}

BOOL PrintXMLSectionData(_In_ PINI_SECTION_DATA pSectionData, _In_ HANDLE hXMLFile)
{
	DWORD dwDataRead = 0;

	if (!pSectionData || !(pSectionData->tSectionName))
	{
		DEBUG_LOG(D_WARNING, "PINI_SECTION_DATA invalid for INI file.\r\n");
		DoExit(D_WARNING);
	}

	if (hXMLFile == INVALID_HANDLE_VALUE)
	{
		DEBUG_LOG(D_WARNING, "Handle to hXMLFile is invalid.\r\nExiting now...");
		DoExit(D_ERROR);
	}

	for (DWORD i = 0; i < pSectionData->iNumberOfProperty; ++i)
	{
		PINI_PROPERTY_DATA pCurrPropertyData = pSectionData->pProperties[i];
		PTCHAR tEscapedValue = EscapeXMLString(pCurrPropertyData->tValue);
		PTCHAR tEscapedName = EscapeXMLString(pCurrPropertyData->tName);

		if (!pCurrPropertyData->tName)
			continue;

		if ((WriteFile(hXMLFile, TEXT("\t\t\t<Property name=\""), (DWORD)(_tcslen(TEXT("\t\t\t<Property name=\"")) * sizeof (TCHAR)), &dwDataRead, NULL) == FALSE)
			|| (WriteFile(hXMLFile, tEscapedName, (DWORD)(_tcslen(tEscapedName) * sizeof (WCHAR)), &dwDataRead, NULL) == FALSE)
			|| (WriteFile(hXMLFile, TEXT("\" value=\""), (DWORD)(_tcslen(TEXT("\" value=\"")) * sizeof (TCHAR)), &dwDataRead, NULL) == FALSE)
			|| (WriteFile(hXMLFile, tEscapedValue, (DWORD)(_tcslen(tEscapedValue) * sizeof (WCHAR)), &dwDataRead, NULL) == FALSE)
			|| (WriteFile(hXMLFile, TEXT("\"/>\r\n"), (DWORD)(_tcslen(TEXT("\"/>\r\n")) * sizeof (TCHAR)), &dwDataRead, NULL) == FALSE))
			goto writerror;
		if (tEscapedName)
			HeapFree(hCrawlerHeap, NULL, tEscapedName);
		if (tEscapedValue)
			HeapFree(hCrawlerHeap, NULL, tEscapedValue);
	}

	return TRUE;
writerror:
	DEBUG_LOG(D_WARNING, "Unable to write XML DATA.\r\nExiting now...");
	DoExit(D_ERROR);
	return FALSE;
}

BOOL PrintCSVData(_In_ PINI_FILE_DATA pIniFileData, _In_ HANDLE hCSVFile)
{
	DWORD dwDataRead = 0;
	
	if (!pIniFileData || !(pIniFileData->tFilePath))
	{
		DEBUG_LOG(D_WARNING, "PINI_FILE_DATA invalid for INI file.\r\n");
		DoExit(D_WARNING);
	}

	if (hCSVFile == INVALID_HANDLE_VALUE)
	{
		DEBUG_LOG(D_WARNING, "Handle to hCSVFile is invalid.\r\nExiting now...");
		DoExit(D_ERROR);
	}

	for (DWORD i = 0; i < pIniFileData->iNumberOfSection; ++i)
	{
		PINI_SECTION_DATA pCurrSectionData = pIniFileData->pSections[i];

		if (!pCurrSectionData->tSectionName)
			continue;

		PrintCSVSectionData(pIniFileData, pCurrSectionData, hCSVFile);
	}

	return TRUE;
}

BOOL PrintCSVSectionData(_In_ PINI_FILE_DATA pIniData, _In_ PINI_SECTION_DATA pSectionData, _In_ HANDLE hCSVFile)
{
	DWORD dwDataRead = 0;

	if (!pSectionData || !(pSectionData->tSectionName))
	{
		DEBUG_LOG(D_WARNING, "PINI_SECTION_DATA invalid for INI file.\r\n");
		DoExit(D_WARNING);
	}

	if (hCSVFile == INVALID_HANDLE_VALUE)
	{
		DEBUG_LOG(D_WARNING, "Handle to hCSVFile is invalid.\r\nExiting now...");
		DoExit(D_ERROR);
	}

	for (DWORD i = 0; i < pSectionData->iNumberOfProperty; ++i)
	{
		PINI_PROPERTY_DATA pCurrPropertyData = pSectionData->pProperties[i];

		if (!pCurrPropertyData->tName)
			continue;

		if ((WriteFile(hCSVFile, pIniData->tFilePath, (DWORD)(_tcslen(pIniData->tFilePath) * sizeof (WCHAR)), &dwDataRead, NULL) == FALSE)
			|| (WriteFile(hCSVFile, TEXT(";"), (DWORD)(_tcslen(TEXT(";")) * sizeof (TCHAR)), &dwDataRead, NULL) == FALSE)
			|| (WriteFile(hCSVFile, pSectionData->tSectionName, (DWORD)(_tcslen(pSectionData->tSectionName) * sizeof (WCHAR)), &dwDataRead, NULL) == FALSE)
			|| (WriteFile(hCSVFile, TEXT(";"), (DWORD)(_tcslen(TEXT(";")) * sizeof (TCHAR)), &dwDataRead, NULL) == FALSE)
			|| (WriteFile(hCSVFile, pCurrPropertyData->tName, (DWORD)(_tcslen(pCurrPropertyData->tName) * sizeof (WCHAR)), &dwDataRead, NULL) == FALSE)
			|| (WriteFile(hCSVFile, TEXT(";"), (DWORD)(_tcslen(TEXT(";")) * sizeof (TCHAR)), &dwDataRead, NULL) == FALSE)
			|| (WriteFile(hCSVFile, pCurrPropertyData->tValue, (DWORD)(_tcslen(pCurrPropertyData->tValue) * sizeof (WCHAR)), &dwDataRead, NULL) == FALSE)
			|| (WriteFile(hCSVFile, TEXT("\r\n"), (DWORD)(_tcslen(TEXT("\r\n")) * sizeof (TCHAR)), &dwDataRead, NULL) == FALSE))
			goto writerror;		
	}

	return TRUE;
writerror:
	DEBUG_LOG(D_WARNING, "Unable to write CSV DATA.\r\nExiting now...");
	DoExit(D_ERROR);
	return FALSE;
}

BOOL PrintSTDOUTData(_In_ PINI_FILE_DATA pIniFileData)
{
	DWORD dwDataRead = 0;

	if (!pIniFileData || !(pIniFileData->tFilePath))
	{
		DEBUG_LOG(D_WARNING, "PINI_FILE_DATA invalid for INI file.\r\n");
		DoExit(D_WARNING);
	}

	for (DWORD i = 0; i < pIniFileData->iNumberOfSection; ++i)
	{
		PINI_SECTION_DATA pCurrSectionData = pIniFileData->pSections[i];

		if (!pCurrSectionData->tSectionName)
			continue;

		PrintSTDOUTSectionData(pIniFileData, pCurrSectionData);
	}
	return TRUE;
}

BOOL PrintSTDOUTSectionData(_In_ PINI_FILE_DATA pInfData, _In_ PINI_SECTION_DATA pSectionData)
{
	DWORD dwDataRead = 0;
	
	if (!pSectionData || !(pSectionData->tSectionName))
	{
		DEBUG_LOG(D_WARNING, "PINI_FILE_DATA invalid for INI file.\r\n");
		DoExit(D_WARNING);
	}

	for (DWORD i = 0; i < pSectionData->iNumberOfProperty; ++i)
	{
		PINI_PROPERTY_DATA pCurrPropertyData = pSectionData->pProperties[i];

		if (!pCurrPropertyData->tName)
			continue;

		printf("\t[INI] File=%ws SectionName=%ws PropertyName=%ws Value=%ws\r\n", pInfData->tFilePath, pSectionData->tSectionName, pCurrPropertyData->tName, pCurrPropertyData->tValue);	
	}

	return TRUE;
}
