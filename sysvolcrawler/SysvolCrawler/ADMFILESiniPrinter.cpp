/**************************************************
* SysvolCrawler - ADMFILESiniPrinter.cpp
* AUTHOR: Luc Delsalle	
*
* Display or export administrative template data
*	
* ANSSI/COSSI/DTO/BAI - 2014
***************************************************/

#include "ADMFILESiniPrinter.h"

BOOL PrintData(_In_ PADMFILESINI_FILE_DATA pAdmFilesIniData)
{
	BOOL bRes = TRUE;

	if (pAdmFilesIniData == NULL)
	{
		DEBUG_LOG(D_ERROR, "ADMFILESINI_FILE_DATA pointer invalid.\r\nExiting now...");
		DoExit(D_ERROR);
	}

	// Call every printer
	if ((bRes) && (pSyscrwlrOptions->bShouldPrintXML))
		bRes = PrintXMLData(pAdmFilesIniData);

	if ((bRes) && (pSyscrwlrOptions->bShouldPrintCSV))
		bRes = PrintCSVData(pAdmFilesIniData);

	if ((bRes) && (pSyscrwlrOptions->bShouldPrintSTDOUT))
		bRes = PrintSTDOUTData(pAdmFilesIniData);

	return bRes;
}

BOOL PrintAdmFilesIniDataHeader(_In_ PTCHAR tFilePath)
{
	DWORD dwDataRead = 0;
	LARGE_INTEGER liFileSize;
	GPO_FILTER_TARGET dwTarget = GPO_FILTER_UNKNOWN;
	PTCHAR tOutputDirectoryAdmFile = OUTPUT_DIRECTORY_ADMFILES_INI;

	if (!tFilePath)
	{
		DEBUG_LOG(D_WARNING, "tFilePath is invalid.\r\nExiting now...");
		DoExit(D_ERROR);
	}

	// Hack for closing xml document. Ugly.
	if (pSyscrwlrOptions->bShouldPrintXML)
	{
		HANDLE hXMLFile = GetFileHandle(OUTPUT_FILE_XML, tOutputDirectoryAdmFile, OUTPUT_NAME_ADMFILES_INI);

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
			|| (WriteFile(hXMLFile, OUTPUT_NAME_ADMFILES_INI, (DWORD)(_tcslen(OUTPUT_NAME_ADMFILES_INI) * sizeof (TCHAR)), &dwDataRead, NULL) == FALSE)
			|| (WriteFile(hXMLFile, TEXT(".xml>\r\n"), (DWORD)(_tcslen(TEXT(".xml>\r\n")) * sizeof (TCHAR)), &dwDataRead, NULL) == FALSE))
				goto writerror;
		}

		if ((WriteFile(hXMLFile, TEXT("<ADMFILESiniFile Filename=\""), (DWORD)(_tcslen(TEXT("<ADMFILESiniFile Filename=\"")) * sizeof (TCHAR)), &dwDataRead, NULL) == FALSE)
		|| (WriteFile(hXMLFile, tFilePath, (DWORD)(_tcslen(tFilePath) * sizeof (TCHAR)), &dwDataRead, NULL) == FALSE)
		|| (WriteFile(hXMLFile, TEXT("\">\r\n"), (DWORD)(_tcslen(TEXT("\">\r\n")) * sizeof (TCHAR)), &dwDataRead, NULL) == FALSE))
			goto writerror;

		CloseHandle(hXMLFile);
	}

	if (pSyscrwlrOptions->bShouldPrintCSV)
	{
		HANDLE hCSVFile = GetFileHandle(OUTPUT_FILE_CSV, tOutputDirectoryAdmFile, OUTPUT_NAME_ADMFILES_INI);
		LARGE_INTEGER liFileSize;

		if (!GetFileSizeEx(hCSVFile, &liFileSize))
		{
			DEBUG_LOG(D_WARNING, "Unable to determine file size.\r\nExiting now...");
			DoExit(D_ERROR);
		}

		if ((liFileSize.HighPart == 0) && (liFileSize.LowPart == 0))
		{
			if (WriteFile(hCSVFile, TEXT("File;Adm;Unreferrenced\r\n"), (DWORD)(_tcslen(TEXT("File;Adm;Unreferrenced\r\n")) * sizeof (TCHAR)), &dwDataRead, NULL) == FALSE)
				goto writerror;
		}
		CloseHandle(hCSVFile);
	}
	return TRUE;

writerror:
	DEBUG_LOG(D_WARNING, "Unable to write DATA HEADER for ADMFILESini printer.\r\nExiting now...");
	DoExit(D_ERROR);
	return FALSE;
}

BOOL PrintAdmFilesIniDataFooter(_In_ PTCHAR tFilePath)
{
	DWORD dwDataRead = 0;
	GPO_FILTER_TARGET dwTarget = GPO_FILTER_UNKNOWN;
	PTCHAR tOutputDirectoryAdmFile = OUTPUT_DIRECTORY_ADMFILES_INI;

	// Hack for closing xml document. Ugly.
	if (pSyscrwlrOptions->bShouldPrintXML)
	{
		HANDLE hXMLFile = GetFileHandle(OUTPUT_FILE_XML, tOutputDirectoryAdmFile, OUTPUT_NAME_ADMFILES_INI);
		if (WriteFile(hXMLFile, TEXT("</ADMFILESiniFile>\r\n"), (DWORD)(_tcslen(TEXT("</ADMFILESiniFile>\r\n")) * sizeof (TCHAR)), &dwDataRead, NULL) == FALSE)
			goto writerror;
		CloseHandle(hXMLFile);
	}
	return TRUE;

writerror:
	DEBUG_LOG(D_WARNING, "Unable to write DATA FOOTER for ADMFILESini printer.\r\nExiting now...");
	DoExit(D_ERROR);
	return FALSE;
}

BOOL PrintXMLData(_In_ PADMFILESINI_FILE_DATA pAdmFilesIniData)
{
	DWORD dwDataRead = 0;
	HANDLE hXMLFile = INVALID_HANDLE_VALUE;
	GPO_FILTER_TARGET dwTarget = GPO_FILTER_UNKNOWN;
	PTCHAR tOutputDirectoryAdmFile = OUTPUT_DIRECTORY_ADMFILES_INI;
	TCHAR tNumOfAdm[11];
	DWORD dwNumOfAdmLen = 0;
	
	hXMLFile = GetFileHandle(OUTPUT_FILE_XML, tOutputDirectoryAdmFile, OUTPUT_NAME_ADMFILES_INI);

	if (!pAdmFilesIniData || !(pAdmFilesIniData->tFilePath))
	{
		DEBUG_LOG(D_WARNING, "PADMFILESINI_FILE_DATA invalid for admfiles.ini file.\r\n");
		DoExit(D_WARNING);
	}

	if (hXMLFile == INVALID_HANDLE_VALUE)
	{
		DEBUG_LOG(D_WARNING, "Handle to hXMLFile is invalid.\r\nExiting now...");
		DoExit(D_ERROR);
	}

	dwNumOfAdmLen = _stprintf_s(tNumOfAdm, 11, TEXT("%d"), pAdmFilesIniData->dwAdmFileListNum);
	if ((WriteFile(hXMLFile, TEXT("\t<Adm numberofadmfiles=\""), (DWORD)(_tcslen(TEXT("\t<Adm numberofadmfiles=\"")) * sizeof (TCHAR)), &dwDataRead, NULL) == FALSE)
		|| (WriteFile(hXMLFile, tNumOfAdm, (DWORD)(dwNumOfAdmLen * sizeof (WCHAR)), &dwDataRead, NULL) == FALSE)
		|| (WriteFile(hXMLFile, TEXT("\">\r\n"), (DWORD)(_tcslen(TEXT("\">\r\n")) * sizeof (TCHAR)), &dwDataRead, NULL) == FALSE))
		goto writerror;
	for (DWORD i = 0; i < pAdmFilesIniData->dwAdmFileListNum; ++i)
	{
		PADMFILESINI_ADM_DATA pCurrAdmData = pAdmFilesIniData->pAdmFileList[i];
		TCHAR tAdmVersion[11];
		DWORD dwAdmversionLen = 0;

		if (!pCurrAdmData)
			continue;
		dwAdmversionLen = _stprintf_s(tAdmVersion, 11, TEXT("%d"), pCurrAdmData->dwAdmVersion);

		if ((WriteFile(hXMLFile, TEXT("\t\t<Admfile name=\""), (DWORD)(_tcslen(TEXT("\t\t<Admfile name=\"")) * sizeof (TCHAR)), &dwDataRead, NULL) == FALSE)
			|| (WriteFile(hXMLFile, pCurrAdmData->tAdmName, (DWORD)(_tcslen(pCurrAdmData->tAdmName) * sizeof (WCHAR)), &dwDataRead, NULL) == FALSE)
			|| (WriteFile(hXMLFile, TEXT("\" version=\""), (DWORD)(_tcslen(TEXT("\" version=\"")) * sizeof (TCHAR)), &dwDataRead, NULL) == FALSE)
			|| (WriteFile(hXMLFile, tAdmVersion, (DWORD)(dwAdmversionLen * sizeof (WCHAR)), &dwDataRead, NULL) == FALSE)
			|| (WriteFile(hXMLFile, TEXT("\"/>\r\n"), (DWORD)(_tcslen(TEXT("\"/>\r\n")) * sizeof (TCHAR)), &dwDataRead, NULL) == FALSE))
			goto writerror;
	}
	if (WriteFile(hXMLFile, TEXT("\t</Adm>\r\n"), (DWORD)(_tcslen(TEXT("\t</Adm>\r\n")) * sizeof (TCHAR)), &dwDataRead, NULL) == FALSE)
		goto writerror;

	for (DWORD i = 0; i < pAdmFilesIniData->dwNumberOfUnReferrencedSections; ++i)
	{
		PINI_SECTION_DATA pCurrSectionData = pAdmFilesIniData->pUnReferrencedSections[i];

		if (!pCurrSectionData->tSectionName)
			continue;

		PrintXMLUnreferencedSectionDataInAdmFiles(pCurrSectionData, hXMLFile);
	}	

	CloseHandle(hXMLFile);
	return TRUE;
writerror:
	DEBUG_LOG(D_WARNING, "Unable to write XML DATA.\r\nExiting now...");
	DoExit(D_ERROR);
	return FALSE;
}

BOOL PrintXMLUnreferencedSectionDataInAdmFiles(_In_ PINI_SECTION_DATA pSectionData, _In_ HANDLE hXMLFile)
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

BOOL PrintCSVData(_In_ PADMFILESINI_FILE_DATA pAdmFilesIniData)
{
	DWORD dwDataRead = 0;
	HANDLE hCSVFile = INVALID_HANDLE_VALUE;
	GPO_FILTER_TARGET dwTarget = GPO_FILTER_UNKNOWN;
	PTCHAR tOutputDirectoryAdmFile = OUTPUT_DIRECTORY_ADMFILES_INI;
	
	hCSVFile = GetFileHandle(OUTPUT_FILE_CSV, tOutputDirectoryAdmFile, OUTPUT_NAME_ADMFILES_INI);

	if (!pAdmFilesIniData || !(pAdmFilesIniData->tFilePath))
	{
		DEBUG_LOG(D_WARNING, "PADMFILESINI_FILE_DATA invalid for admfiles.ini file.\r\n");
		DoExit(D_WARNING);
	}

	if (hCSVFile == INVALID_HANDLE_VALUE)
	{
		DEBUG_LOG(D_WARNING, "Handle to hCSVFile is invalid.\r\nExiting now...");
		DoExit(D_ERROR);
	}

	if ((WriteFile(hCSVFile, pAdmFilesIniData->tFilePath, (DWORD)(_tcslen(pAdmFilesIniData->tFilePath) * sizeof (WCHAR)), &dwDataRead, NULL) == FALSE)
		|| (WriteFile(hCSVFile, TEXT(";"), (DWORD)(_tcslen(TEXT(";")) * sizeof (TCHAR)), &dwDataRead, NULL) == FALSE))
		goto writerror;	

	for (DWORD i = 0; i < pAdmFilesIniData->dwAdmFileListNum; ++i)
	{
		PADMFILESINI_ADM_DATA pCurrAdmData = pAdmFilesIniData->pAdmFileList[i];
		TCHAR tAdmVersion[11];
		DWORD dwAdmversionLen = 0;

		if (!pCurrAdmData)
			continue;
		dwAdmversionLen = _stprintf_s(tAdmVersion, 11, TEXT("%d"), pCurrAdmData->dwAdmVersion);

		if ((WriteFile(hCSVFile, TEXT("(Admfile name:"), (DWORD)(_tcslen(TEXT("(Admfile name:")) * sizeof (TCHAR)), &dwDataRead, NULL) == FALSE)
			|| (WriteFile(hCSVFile, pCurrAdmData->tAdmName, (DWORD)(_tcslen(pCurrAdmData->tAdmName) * sizeof (WCHAR)), &dwDataRead, NULL) == FALSE)
			|| (WriteFile(hCSVFile, TEXT(", version:"), (DWORD)(_tcslen(TEXT(", version:")) * sizeof (TCHAR)), &dwDataRead, NULL) == FALSE)
			|| (WriteFile(hCSVFile, tAdmVersion, (DWORD)(dwAdmversionLen * sizeof (WCHAR)), &dwDataRead, NULL) == FALSE)
			|| (WriteFile(hCSVFile, TEXT(")"), (DWORD)(_tcslen(TEXT(")")) * sizeof (TCHAR)), &dwDataRead, NULL) == FALSE))
			goto writerror;	
	}
	if (!pAdmFilesIniData->dwAdmFileListNum)
	{
		if (WriteFile(hCSVFile, TEXT("(missing);"), (DWORD)(_tcslen(TEXT("(missing);")) * sizeof (TCHAR)), &dwDataRead, NULL) == FALSE)
			goto writerror;	
	}
	else
	{
		if (WriteFile(hCSVFile, TEXT(";"), (DWORD)(_tcslen(TEXT(";")) * sizeof (TCHAR)), &dwDataRead, NULL) == FALSE)
			goto writerror;	
	}

	for (DWORD i = 0; i < pAdmFilesIniData->dwNumberOfUnReferrencedSections; ++i)
	{
		PINI_SECTION_DATA pCurrSectionData = pAdmFilesIniData->pUnReferrencedSections[i];

		printf("[SectionName=%ws Properties=", pCurrSectionData->tSectionName);
		if ((WriteFile(hCSVFile, TEXT("[SectionName="), (DWORD)(_tcslen(TEXT("[SectionName=")) * sizeof (TCHAR)), &dwDataRead, NULL) == FALSE)
			|| (WriteFile(hCSVFile, pCurrSectionData->tSectionName, (DWORD)(_tcslen(pCurrSectionData->tSectionName) * sizeof (WCHAR)), &dwDataRead, NULL) == FALSE)
			|| (WriteFile(hCSVFile, TEXT(" Properties="), (DWORD)(_tcslen(TEXT(" Properties=")) * sizeof (TCHAR)), &dwDataRead, NULL) == FALSE))
			goto writerror;	

		for (DWORD j = 0; j < pCurrSectionData->iNumberOfProperty; ++j)
		{
			PINI_PROPERTY_DATA pCurrPropertyData = pCurrSectionData->pProperties[j];
	
			if (!pCurrPropertyData->tName || !pCurrPropertyData->tValue)
				continue;

			if ((WriteFile(hCSVFile, TEXT("("), (DWORD)(_tcslen(TEXT("(")) * sizeof (TCHAR)), &dwDataRead, NULL) == FALSE)
			|| (WriteFile(hCSVFile, pCurrPropertyData->tName, (DWORD)(_tcslen(pCurrPropertyData->tName) * sizeof (WCHAR)), &dwDataRead, NULL) == FALSE)
			|| (WriteFile(hCSVFile, TEXT(", "), (DWORD)(_tcslen(TEXT(", ")) * sizeof (TCHAR)), &dwDataRead, NULL) == FALSE)
			|| (WriteFile(hCSVFile, pCurrPropertyData->tValue, (DWORD)(_tcslen(pCurrPropertyData->tValue) * sizeof (WCHAR)), &dwDataRead, NULL) == FALSE)
			|| (WriteFile(hCSVFile, TEXT(")"), (DWORD)(_tcslen(TEXT(")")) * sizeof (TCHAR)), &dwDataRead, NULL) == FALSE))
				goto writerror;
		}

		if ((WriteFile(hCSVFile, TEXT("]"), (DWORD)(_tcslen(TEXT("]")) * sizeof (TCHAR)), &dwDataRead, NULL) == FALSE))
			goto writerror;
	}

	if ((WriteFile(hCSVFile, TEXT("\r\n"), (DWORD)(_tcslen(TEXT("\r\n")) * sizeof (TCHAR)), &dwDataRead, NULL) == FALSE))
			goto writerror;

	CloseHandle(hCSVFile);
	return TRUE;
writerror:
	DEBUG_LOG(D_WARNING, "Unable to write CSV DATA.\r\nExiting now...");
	DoExit(D_ERROR);
	return FALSE;
}

BOOL PrintSTDOUTData(_In_ PADMFILESINI_FILE_DATA pAdmFilesIniData)
{
	if (!pAdmFilesIniData)
	{
		DEBUG_LOG(D_WARNING, "PADMFILESINI_FILE_DATA invalid for admfiles.ini file.\r\n");
		DoExit(D_ERROR);
	}

	printf("[ADMFILESini] File=%ws", pAdmFilesIniData->tFilePath);

	for (DWORD i = 0; i < pAdmFilesIniData->dwAdmFileListNum; ++i)
	{
		PADMFILESINI_ADM_DATA pCurrAdmData = pAdmFilesIniData->pAdmFileList[i];

		if (!pCurrAdmData)
			continue;

		printf(" AdmFile=(name:%ws, version:%d)", pCurrAdmData->tAdmName, pCurrAdmData->dwAdmVersion);
	}
	if (!pAdmFilesIniData->dwAdmFileListNum)
		printf(" AdmFile=(missing)");


	for (DWORD i = 0; i < pAdmFilesIniData->dwNumberOfUnReferrencedSections; ++i)
	{
		PINI_SECTION_DATA pCurrSectionData = pAdmFilesIniData->pUnReferrencedSections[i];

		if (i == 0)
			printf(" UnreferrencedSection=");

		printf("[SectionName=%ws Properties=", pCurrSectionData->tSectionName);

		for (DWORD j = 0; j < pCurrSectionData->iNumberOfProperty; ++j)
		{
			PINI_PROPERTY_DATA pCurrPropertyData = pCurrSectionData->pProperties[j];
	
			if (!pCurrPropertyData->tName || !pCurrPropertyData->tValue)
				continue;

			printf("(%ws, %ws)", pCurrPropertyData->tName, pCurrPropertyData->tValue);
		}
		printf("]");
	}

	printf("\r\n");
	return TRUE;
}