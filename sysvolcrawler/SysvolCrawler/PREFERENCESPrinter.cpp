/**************************************************
* SysvolCrawler - PREFERENCESPrinter.cpp
* AUTHOR: Luc Delsalle	
*
* Display or export preferences GPO file data
*
* ANSSI/COSSI/DTO/BAI - 2014
***************************************************/

#include "PREFERENCESPrinter.h"

BOOL PrintData(_In_ PPREFERENCES_FILE_DATA pPreferencesFileData)
{
	BOOL bRes = TRUE;

	if (pPreferencesFileData == NULL)
	{
		DEBUG_LOG(D_ERROR, "pPreferencesFileData pointer invalid.\r\nExiting now...");
		DoExit(D_ERROR);
	}

	// Call every printer
	if ((bRes) && (pSyscrwlrOptions->bShouldPrintXML))
		bRes = PrintXMLData(pPreferencesFileData);

	if ((bRes) && (pSyscrwlrOptions->bShouldPrintCSV))
		bRes = PrintCSVData(pPreferencesFileData);

	if ((bRes) && (pSyscrwlrOptions->bShouldPrintSTDOUT))
		bRes = PrintSTDOUTData(pPreferencesFileData);

	return bRes;
}

BOOL PrintPreferencesDataHeader(_In_ PTCHAR tFilePath)
{
	PTCHAR tOutputDirectoryPREFERENCESFile = OUTPUT_DIRECTORY_PREFERENCES_FOLDER;
	GPO_FILTER_TARGET dwTarget = GPO_FILTER_UNKNOWN;
	LARGE_INTEGER liFileSize;
	DWORD dwDataRead = 0;

	if (!tFilePath)
	{
		DEBUG_LOG(D_WARNING, "tFilePath is invalid.\r\nExiting now...");
		DoExit(D_ERROR);
	}

	// Determine if we need to create user or computer file in case of '[Machine||User]' 
	dwTarget = GetTargetGPO(tFilePath);
	if (dwTarget == GPO_FILTER_TARGET_MACHINE)
		tOutputDirectoryPREFERENCESFile = TEXT(".\\Machine\\");
	else if (dwTarget == GPO_FILTER_TARGET_USER)
		tOutputDirectoryPREFERENCESFile = TEXT(".\\User\\");

	// Hack for closing xml document. Ugly.
	if (pSyscrwlrOptions->bShouldPrintXML)
	{
		HANDLE hXMLFile = GetFileHandle(OUTPUT_FILE_XML, tOutputDirectoryPREFERENCESFile, OUTPUT_NAME_PREFERENCES_FOLDER);

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
			|| (WriteFile(hXMLFile, OUTPUT_NAME_PREFERENCES_FOLDER, (DWORD)(_tcslen(OUTPUT_NAME_PREFERENCES_FOLDER) * sizeof (TCHAR)), &dwDataRead, NULL) == FALSE)
			|| (WriteFile(hXMLFile, TEXT(".xml>\r\n"), (DWORD)(_tcslen(TEXT(".xml>\r\n")) * sizeof (TCHAR)), &dwDataRead, NULL) == FALSE))
				goto writerror;
		}

		if ((WriteFile(hXMLFile, TEXT("<PREFERENCESFile Filename=\""), (DWORD)(_tcslen(TEXT("<PREFERENCESFile Filename=\"")) * sizeof (TCHAR)), &dwDataRead, NULL) == FALSE)
		|| (WriteFile(hXMLFile, tFilePath, (DWORD)(_tcslen(tFilePath) * sizeof (TCHAR)), &dwDataRead, NULL) == FALSE)
		|| (WriteFile(hXMLFile, TEXT("\">\r\n"), (DWORD)(_tcslen(TEXT("\">\r\n")) * sizeof (TCHAR)), &dwDataRead, NULL) == FALSE))
			goto writerror;

		CloseHandle(hXMLFile);
	}
	return TRUE;

writerror:
	DEBUG_LOG(D_WARNING, "Unable to write DATA HEADER for PREFERENCES file printer.\r\nExiting now...");
	DoExit(D_ERROR);
	return FALSE;
}

BOOL PrintPreferencesDataFooter(_In_ PTCHAR tFilePath)
{
	DWORD dwDataRead = 0;
	GPO_FILTER_TARGET dwTarget = GPO_FILTER_UNKNOWN;
	PTCHAR tOutputDirectoryPreferencesFolder = OUTPUT_DIRECTORY_PREFERENCES_FOLDER;

	// Determine if we need to create user or computer file in case of '[Machine||User]' 
	dwTarget = GetTargetGPO(tFilePath);
	if (dwTarget == GPO_FILTER_TARGET_MACHINE)
		tOutputDirectoryPreferencesFolder = TEXT(".\\Machine\\");
	else if (dwTarget == GPO_FILTER_TARGET_USER)
		tOutputDirectoryPreferencesFolder = TEXT(".\\User\\");

	// Hack for closing xml document. Ugly.
	if (pSyscrwlrOptions->bShouldPrintXML)
	{
		HANDLE hXMLFile = GetFileHandle(OUTPUT_FILE_XML, tOutputDirectoryPreferencesFolder, OUTPUT_NAME_PREFERENCES_FOLDER);
		if (WriteFile(hXMLFile, TEXT("</PREFERENCESFile>\r\n"), (DWORD)(_tcslen(TEXT("</PREFERENCESFile>\r\n")) * sizeof (TCHAR)), &dwDataRead, NULL) == FALSE)
			goto writerror;
		CloseHandle(hXMLFile);
	}
	return TRUE;

writerror:
	DEBUG_LOG(D_WARNING, "Unable to write DATA FOOTER for SCRIPTSini printer.\r\nExiting now...");
	DoExit(D_ERROR);
	return FALSE;
}

BOOL PrintXMLData(_In_ PPREFERENCES_FILE_DATA pPreferencesFileData)
{
	PTCHAR tOutputDirectoryPREFERENCESFile = OUTPUT_DIRECTORY_PREFERENCES_FOLDER;
	HANDLE hXMLFile = INVALID_HANDLE_VALUE;
	GPO_FILTER_TARGET dwTarget = GPO_FILTER_UNKNOWN;
	BOOL bRes = FALSE;

	if (!pPreferencesFileData || !(pPreferencesFileData->pvData) || !(pPreferencesFileData->tFilePath))
	{
		DEBUG_LOG(D_WARNING, "PIEAK_FILE_DATA invalid.\r\nExiting now...");
		DoExit(D_ERROR);
	}

	// Determine if we need to create user or computer file in case of '[Machine||User]' 
	dwTarget = GetTargetGPO(pPreferencesFileData->tFilePath);
	if (dwTarget == GPO_FILTER_TARGET_MACHINE)
		tOutputDirectoryPREFERENCESFile = TEXT(".\\Machine\\");
	else if (dwTarget == GPO_FILTER_TARGET_USER)
		tOutputDirectoryPREFERENCESFile = TEXT(".\\User\\");

	hXMLFile = GetFileHandle(OUTPUT_FILE_XML, tOutputDirectoryPREFERENCESFile, OUTPUT_NAME_PREFERENCES_FOLDER);
	if (hXMLFile == INVALID_HANDLE_VALUE)
	{
		DEBUG_LOG(D_WARNING, "Handle to hXMLFile is invalid.\r\nExiting now...");
		DoExit(D_ERROR);
	}

	switch (pPreferencesFileData->dwFileType)
	{
	case PREFERENCES_INI_FILE:
		bRes = PrintXMLIniData(pPreferencesFileData, hXMLFile);
		break;
	case PREFERENCES_INF_FILE: // parse INF file like INI file
		bRes = PrintXMLIniData(pPreferencesFileData, hXMLFile);
		break;
	default:
		bRes = PrintXMLRawData(pPreferencesFileData, hXMLFile);
		break;
	}

	CloseHandle(hXMLFile);
	return TRUE;
}

BOOL PrintXMLRawData(_In_ PPREFERENCES_FILE_DATA pPreferencesFileData, _In_ HANDLE hXMLFile)
{
	DWORD dwDataRead = 0, dwSizeLength = 0;
	TCHAR tSize[100];
	PTCHAR tData = NULL;

	if (!pPreferencesFileData || !hXMLFile)
	{
		DEBUG_LOG(D_WARNING, "PREFERENCES_FILE_DATA or HANDLE invalid.\r\nExiting now...");
		DoExit(D_ERROR);
	}

	tData = GetBase64FromByte((PBYTE) pPreferencesFileData->pvData, pPreferencesFileData->dwDataSize);
	dwSizeLength = _stprintf_s(tSize, 100, TEXT("%d"), (pPreferencesFileData->dwDataSize));

	if ((WriteFile(hXMLFile, TEXT("\t<RawData size=\""), (DWORD)(_tcslen(TEXT("\t<RawData size=\"")) * sizeof (TCHAR)), &dwDataRead, NULL) == FALSE)
		|| (WriteFile(hXMLFile, tSize, (DWORD)(_tcslen(tSize) * sizeof (WCHAR)), &dwDataRead, NULL) == FALSE)
		|| (WriteFile(hXMLFile, TEXT("\" data=\""), (DWORD)(_tcslen(TEXT("\" data=\"")) * sizeof (TCHAR)), &dwDataRead, NULL) == FALSE)
		|| (WriteFile(hXMLFile, tData, (DWORD)(_tcslen(tData) * sizeof (WCHAR)), &dwDataRead, NULL) == FALSE)
		|| (WriteFile(hXMLFile, TEXT("\"/>\r\n"), (DWORD)(_tcslen(TEXT("\"/>\r\n")) * sizeof (TCHAR)), &dwDataRead, NULL) == FALSE))
		goto writerror;

	HeapFree(hCrawlerHeap, NULL, tData);
	return TRUE;
writerror:
	DEBUG_LOG(D_WARNING, "Unable to write XML DATA.\r\nExiting now...");
	DoExit(D_ERROR);
	return FALSE;
}

BOOL PrintXMLIniData(_In_ PPREFERENCES_FILE_DATA pPreferencesFileData, _In_ HANDLE hXMLFile)
{
	DWORD dwDataRead = 0;
	PINI_FILE_DATA pIniFileData = NULL;

	if (!pPreferencesFileData || !hXMLFile || !(pPreferencesFileData->pvData))
	{
		DEBUG_LOG(D_WARNING, "PREFERENCES_FILE_DATA or HANDLE invalid.\r\nExiting now...");
		DoExit(D_ERROR);
	}
	pIniFileData = (PINI_FILE_DATA) pPreferencesFileData->pvData;

	if ((WriteFile(hXMLFile, TEXT("\t<InfData>\r\n"), (DWORD)(_tcslen(TEXT("\t<InfData>\r\n")) * sizeof (TCHAR)), &dwDataRead, NULL) == FALSE))
		goto writerror;

	if (PrintXMLData(pIniFileData, hXMLFile) == FALSE)
		goto writerror;

	if ((WriteFile(hXMLFile, TEXT("\t</InfData>\r\n"), (DWORD)(_tcslen(TEXT("\t</InfData>\r\n")) * sizeof (TCHAR)), &dwDataRead, NULL) == FALSE))
		goto writerror;

	return TRUE;
writerror:
	DEBUG_LOG(D_WARNING, "Unable to write XML DATA.\r\nExiting now...");
	DoExit(D_ERROR);
	return FALSE;
}

BOOL PrintCSVData(_In_ PPREFERENCES_FILE_DATA pPreferencesFileData)
{
	PTCHAR tOutputDirectoryPREFERENCESFile = OUTPUT_DIRECTORY_PREFERENCES_FOLDER;
	HANDLE hCSVFile = INVALID_HANDLE_VALUE;
	GPO_FILTER_TARGET dwTarget = GPO_FILTER_UNKNOWN;
	BOOL bRes = FALSE;
	DWORD dwDataRead = 0;

	if (!pPreferencesFileData || !(pPreferencesFileData->pvData) || !(pPreferencesFileData->tFilePath))
	{
		DEBUG_LOG(D_WARNING, "PREFERENCES_FILE_DATA invalid.\r\nExiting now...");
		DoExit(D_ERROR);
	}

	// Determine if we need to create user or computer file in case of '[Machine||User]' 
	dwTarget = GetTargetGPO(pPreferencesFileData->tFilePath);
	if (dwTarget == GPO_FILTER_TARGET_MACHINE)
		tOutputDirectoryPREFERENCESFile = TEXT(".\\Machine\\");
	else if (dwTarget == GPO_FILTER_TARGET_USER)
		tOutputDirectoryPREFERENCESFile = TEXT(".\\User\\");

	hCSVFile = GetFileHandle(OUTPUT_FILE_CSV, tOutputDirectoryPREFERENCESFile, OUTPUT_NAME_PREFERENCES_FOLDER);
	if (hCSVFile == INVALID_HANDLE_VALUE)
	{
		DEBUG_LOG(D_WARNING, "Handle to hCSVFile is invalid.\r\nExiting now...");
		DoExit(D_ERROR);
	}

	switch (pPreferencesFileData->dwFileType)
	{
	case PREFERENCES_INI_FILE:
		if (WriteFile(hCSVFile, TEXT("File;Section;Name;Value\r\n"), (DWORD)(_tcslen(TEXT("File;Section;Name;Value\r\n")) * sizeof (TCHAR)), &dwDataRead, NULL) == FALSE)
			goto writerror;
		bRes = PrintCSVIniData(pPreferencesFileData, hCSVFile);
		break;
	case PREFERENCES_INF_FILE: // parse INF file like INI file
		if (WriteFile(hCSVFile, TEXT("File;Section;Name;Value\r\n"), (DWORD)(_tcslen(TEXT("File;Section;Name;Value\r\n")) * sizeof (TCHAR)), &dwDataRead, NULL) == FALSE)
			goto writerror;
		bRes = PrintCSVIniData(pPreferencesFileData, hCSVFile);
		break;
	default:
		if (WriteFile(hCSVFile, TEXT("File;DataSize;Data\r\n"), (DWORD)(_tcslen(TEXT("File;DataSize;Data\r\n")) * sizeof (TCHAR)), &dwDataRead, NULL) == FALSE)
			goto writerror;
		bRes = PrintCSVRawData(pPreferencesFileData, hCSVFile);
		break;
	}

	CloseHandle(hCSVFile);
	return TRUE;
writerror:
	DEBUG_LOG(D_WARNING, "Unable to write CSV DATA.\r\nExiting now...");
	DoExit(D_ERROR);
	return FALSE;
}

BOOL PrintCSVRawData(_In_ PPREFERENCES_FILE_DATA pPreferencesFileData, _In_ HANDLE hCSVFile)
{
	DWORD dwDataRead = 0, dwSizeLength = 0;
	TCHAR tSize[100];
	PTCHAR tData = NULL;

	if (!pPreferencesFileData || !hCSVFile)
	{
		DEBUG_LOG(D_WARNING, "PREFERENCES_FILE_DATA or HANDLE invalid.\r\nExiting now...");
		DoExit(D_ERROR);
	}

	tData = GetBase64FromByte((PBYTE) (pPreferencesFileData->pvData), pPreferencesFileData->dwDataSize);
	dwSizeLength = _stprintf_s(tSize, 100, TEXT("%d"), (pPreferencesFileData->dwDataSize));

	if ((WriteFile(hCSVFile, pPreferencesFileData->tFilePath, (DWORD)(_tcslen(pPreferencesFileData->tFilePath) * sizeof (WCHAR)), &dwDataRead, NULL) == FALSE)
		|| (WriteFile(hCSVFile, TEXT(";"), (DWORD)(_tcslen(TEXT(";")) * sizeof (TCHAR)), &dwDataRead, NULL) == FALSE)
		|| (WriteFile(hCSVFile, tSize, (DWORD)(_tcslen(tSize) * sizeof (WCHAR)), &dwDataRead, NULL) == FALSE)
		|| (WriteFile(hCSVFile, TEXT(";"), (DWORD)(_tcslen(TEXT(";")) * sizeof (TCHAR)), &dwDataRead, NULL) == FALSE)
		|| (WriteFile(hCSVFile, tData, (DWORD)(_tcslen(tData) * sizeof (WCHAR)), &dwDataRead, NULL) == FALSE)
		|| (WriteFile(hCSVFile, TEXT("\r\n"), (DWORD)(_tcslen(TEXT("\r\n")) * sizeof (TCHAR)), &dwDataRead, NULL) == FALSE))
		goto writerror;

	HeapFree(hCrawlerHeap, NULL, tData);
	return TRUE;
writerror:
	DEBUG_LOG(D_WARNING, "Unable to write CSV DATA.\r\nExiting now...");
	DoExit(D_ERROR);
	return FALSE;
}

BOOL PrintCSVIniData(_In_ PPREFERENCES_FILE_DATA pPreferencesFileData, _In_ HANDLE hCSVFile)
{
	DWORD dwDataRead = 0;
	PINI_FILE_DATA pIniFileData = NULL;

	if (!pPreferencesFileData || !hCSVFile || !(pPreferencesFileData->pvData))
	{
		DEBUG_LOG(D_WARNING, "PREFERENCES_FILE_DATA or HANDLE invalid.\r\nExiting now...");
		DoExit(D_ERROR);
	}
	pIniFileData = (PINI_FILE_DATA) pPreferencesFileData->pvData;

	if (PrintCSVData(pIniFileData, hCSVFile) == FALSE)
		goto writerror;

	return TRUE;
writerror:
	DEBUG_LOG(D_WARNING, "Unable to write CSV DATA.\r\nExiting now...");
	DoExit(D_ERROR);
	return FALSE;
}

BOOL PrintSTDOUTData(_In_ PPREFERENCES_FILE_DATA pPreferencesFileData)
{
	BOOL bRes = FALSE;
	
	if (!pPreferencesFileData || !(pPreferencesFileData->pvData) || !(pPreferencesFileData->tFilePath))
	{
		DEBUG_LOG(D_WARNING, "PREFERENCES_FILE_DATA invalid.\r\nExiting now...");
		DoExit(D_ERROR);
	}

	printf("[PREFERENCES]\r\n");
	switch (pPreferencesFileData->dwFileType)
	{
	case PREFERENCES_INI_FILE:
		bRes = PrintSTDOUTIniData(pPreferencesFileData);
		break;
	case PREFERENCES_INF_FILE: // parse INF file like INI file
		bRes = PrintSTDOUTIniData(pPreferencesFileData);
		break;
	default:
		bRes = PrintSTDOUTRawData(pPreferencesFileData);
		break;
	}
	printf("[/PREFERENCES]\r\n");
	return TRUE;
}

BOOL PrintSTDOUTRawData(_In_ PPREFERENCES_FILE_DATA pPreferencesFileData)
{
	PTCHAR tData = NULL;
	
	if (!pPreferencesFileData || !(pPreferencesFileData->pvData) || !(pPreferencesFileData->tFilePath))
	{
		DEBUG_LOG(D_WARNING, "PREFERENCES_FILE_DATA invalid for IEAK file.\r\n");
		DoExit(D_WARNING);
	}

	tData = GetBase64FromByte((PBYTE )pPreferencesFileData->pvData, pPreferencesFileData->dwDataSize);

	printf("\t[RAW] File=%ws Size=%d Data=%ws\r\n", pPreferencesFileData->tFilePath, pPreferencesFileData->dwDataSize, tData);

	HeapFree(hCrawlerHeap, NULL, tData);
	return TRUE;
}

BOOL PrintSTDOUTIniData(_In_ PPREFERENCES_FILE_DATA pPreferencesFileData)
{
	DWORD dwDataRead = 0;
	PINI_FILE_DATA pIniFileData = NULL;

	if (!pPreferencesFileData || !(pPreferencesFileData->pvData))
	{
		DEBUG_LOG(D_WARNING, "PREFERENCES_FILE_DATA invalid.\r\nExiting now...");
		DoExit(D_ERROR);
	}
	pIniFileData = (PINI_FILE_DATA) pPreferencesFileData->pvData;

	return (PrintSTDOUTData(pIniFileData));
}