/**************************************************
* SysvolCrawler - IEAKPrinter.c
* AUTHOR: Luc Delsalle	

* Display or export content of Internet Explorer file
* (store in IEAK folder)
*	
* ANSSI/COSSI/DTO/BAI - 2014
***************************************************/

#include "IEAKPrinter.h"

BOOL PrintData(_In_ PIEAK_FILE_DATA pIeakFileData)
{
	BOOL bRes = TRUE;

	if (pIeakFileData == NULL)
	{
		DEBUG_LOG(D_ERROR, "pIeakFileData pointer invalid.\r\nExiting now...");
		DoExit(D_ERROR);
	}

	// Call every printer
	if ((bRes) && (pSyscrwlrOptions->bShouldPrintXML))
		bRes = PrintXMLData(pIeakFileData);

	if ((bRes) && (pSyscrwlrOptions->bShouldPrintCSV))
		bRes = PrintCSVData(pIeakFileData);

	if ((bRes) && (pSyscrwlrOptions->bShouldPrintSTDOUT))
		bRes = PrintSTDOUTData(pIeakFileData);

	return bRes;
}

BOOL PrintIeakDataHeader(_In_ PTCHAR tFilePath)
{
	PTCHAR tOutputDirectoryIEAKFile = OUTPUT_DIRECTORY_IEAK_FOLDER;
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
		tOutputDirectoryIEAKFile = TEXT(".\\Machine\\");
	else if (dwTarget == GPO_FILTER_TARGET_USER)
		tOutputDirectoryIEAKFile = TEXT(".\\User\\");

	// Hack for closing xml document. Ugly.
	if (pSyscrwlrOptions->bShouldPrintXML)
	{
		HANDLE hXMLFile = GetFileHandle(OUTPUT_FILE_XML, tOutputDirectoryIEAKFile, OUTPUT_NAME_IEAK_FOLDER);

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
			|| (WriteFile(hXMLFile, OUTPUT_NAME_IEAK_FOLDER, (DWORD)(_tcslen(OUTPUT_NAME_IEAK_FOLDER) * sizeof (TCHAR)), &dwDataRead, NULL) == FALSE)
			|| (WriteFile(hXMLFile, TEXT(".xml>\r\n"), (DWORD)(_tcslen(TEXT(".xml>\r\n")) * sizeof (TCHAR)), &dwDataRead, NULL) == FALSE))
				goto writerror;
		}

		if ((WriteFile(hXMLFile, TEXT("<IEAKFile Filename=\""), (DWORD)(_tcslen(TEXT("<IEAKFile Filename=\"")) * sizeof (TCHAR)), &dwDataRead, NULL) == FALSE)
		|| (WriteFile(hXMLFile, tFilePath, (DWORD)(_tcslen(tFilePath) * sizeof (TCHAR)), &dwDataRead, NULL) == FALSE)
		|| (WriteFile(hXMLFile, TEXT("\">\r\n"), (DWORD)(_tcslen(TEXT("\">\r\n")) * sizeof (TCHAR)), &dwDataRead, NULL) == FALSE))
			goto writerror;

		CloseHandle(hXMLFile);
	}
	return TRUE;

writerror:
	DEBUG_LOG(D_WARNING, "Unable to write DATA HEADER for IEAK file printer.\r\nExiting now...");
	DoExit(D_ERROR);
	return FALSE;
}

BOOL PrintIeakDataFooter(_In_ PTCHAR tFilePath)
{
	DWORD dwDataRead = 0;
	GPO_FILTER_TARGET dwTarget = GPO_FILTER_UNKNOWN;
	PTCHAR tOutputDirectoryIeakFolder = OUTPUT_DIRECTORY_IEAK_FOLDER;

	// Determine if we need to create user or computer file in case of '[Machine||User]' 
	dwTarget = GetTargetGPO(tFilePath);
	if (dwTarget == GPO_FILTER_TARGET_MACHINE)
		tOutputDirectoryIeakFolder = TEXT(".\\Machine\\");
	else if (dwTarget == GPO_FILTER_TARGET_USER)
		tOutputDirectoryIeakFolder = TEXT(".\\User\\");

	// Hack for closing xml document. Ugly.
	if (pSyscrwlrOptions->bShouldPrintXML)
	{
		HANDLE hXMLFile = GetFileHandle(OUTPUT_FILE_XML, tOutputDirectoryIeakFolder, OUTPUT_NAME_IEAK_FOLDER);
		if (WriteFile(hXMLFile, TEXT("</IEAKFile>\r\n"), (DWORD)(_tcslen(TEXT("</IEAKFile>\r\n")) * sizeof (TCHAR)), &dwDataRead, NULL) == FALSE)
			goto writerror;
		CloseHandle(hXMLFile);
	}
	return TRUE;

writerror:
	DEBUG_LOG(D_WARNING, "Unable to write DATA FOOTER for SCRIPTSini printer.\r\nExiting now...");
	DoExit(D_ERROR);
	return FALSE;
}

BOOL PrintXMLData(_In_ PIEAK_FILE_DATA pIeakFileData)
{
	PTCHAR tOutputDirectoryIEAKFile = OUTPUT_DIRECTORY_IEAK_FOLDER;
	HANDLE hXMLFile = INVALID_HANDLE_VALUE;
	GPO_FILTER_TARGET dwTarget = GPO_FILTER_UNKNOWN;
	BOOL bRes = FALSE;

	if (!pIeakFileData || !(pIeakFileData->pvData) || !(pIeakFileData->tFilePath))
	{
		DEBUG_LOG(D_WARNING, "PIEAK_FILE_DATA invalid.\r\nExiting now...");
		DoExit(D_ERROR);
	}

	// Determine if we need to create user or computer file in case of '[Machine||User]' 
	dwTarget = GetTargetGPO(pIeakFileData->tFilePath);
	if (dwTarget == GPO_FILTER_TARGET_MACHINE)
		tOutputDirectoryIEAKFile = TEXT(".\\Machine\\");
	else if (dwTarget == GPO_FILTER_TARGET_USER)
		tOutputDirectoryIEAKFile = TEXT(".\\User\\");

	hXMLFile = GetFileHandle(OUTPUT_FILE_XML, tOutputDirectoryIEAKFile, OUTPUT_NAME_IEAK_FOLDER);
	if (hXMLFile == INVALID_HANDLE_VALUE)
	{
		DEBUG_LOG(D_WARNING, "Handle to hXMLFile is invalid.\r\nExiting now...");
		DoExit(D_ERROR);
	}

	switch (pIeakFileData->dwFileType)
	{
	case IEAK_INI_FILE:
		bRes = PrintXMLIniData(pIeakFileData, hXMLFile);
		break;
	case IEAK_INF_FILE: // parse INF file like INI file
		bRes = PrintXMLIniData(pIeakFileData, hXMLFile);
		break;
	default:
		bRes = PrintXMLRawData(pIeakFileData, hXMLFile);
		break;
	}

	CloseHandle(hXMLFile);
	return TRUE;
}

BOOL PrintXMLRawData(_In_ PIEAK_FILE_DATA pIeakFileData, _In_ HANDLE hXMLFile)
{
	DWORD dwDataRead = 0, dwSizeLength = 0;
	TCHAR tSize[100];
	PTCHAR tData = NULL;

	if (!pIeakFileData || !hXMLFile)
	{
		DEBUG_LOG(D_WARNING, "PIEAK_FILE_DATA or HANDLE invalid.\r\nExiting now...");
		DoExit(D_ERROR);
	}

	tData = GetBase64FromByte((PBYTE) pIeakFileData->pvData, pIeakFileData->dwDataSize);
	dwSizeLength = _stprintf_s(tSize, 100, TEXT("%d"), (pIeakFileData->dwDataSize));

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

BOOL PrintXMLIniData(_In_ PIEAK_FILE_DATA pIeakFileData, _In_ HANDLE hXMLFile)
{
	DWORD dwDataRead = 0;
	PINI_FILE_DATA pIniFileData = NULL;

	if (!pIeakFileData || !hXMLFile || !(pIeakFileData->pvData))
	{
		DEBUG_LOG(D_WARNING, "PIEAK_FILE_DATA or HANDLE invalid.\r\nExiting now...");
		DoExit(D_ERROR);
	}
	pIniFileData = (PINI_FILE_DATA) pIeakFileData->pvData;

	if ((WriteFile(hXMLFile, TEXT("\t<InfData>\r\n"), (DWORD)(_tcslen(TEXT("\t<InfData>\r\n")) * sizeof (TCHAR)), &dwDataRead, NULL) == FALSE))
		goto writerror;

	if (PrintXMLData(pIniFileData, hXMLFile) == FALSE)
		goto writerror;

	if ((WriteFile(hXMLFile, TEXT("\t</InfData>\r\n"), (DWORD)(_tcslen(TEXT("\t<InfData>\r\n")) * sizeof (TCHAR)), &dwDataRead, NULL) == FALSE))
		goto writerror;

	return TRUE;
writerror:
	DEBUG_LOG(D_WARNING, "Unable to write XML DATA.\r\nExiting now...");
	DoExit(D_ERROR);
	return FALSE;
}

BOOL PrintCSVData(_In_ PIEAK_FILE_DATA pIeakFileData)
{
	PTCHAR tOutputDirectoryIEAKFile = OUTPUT_DIRECTORY_IEAK_FOLDER;
	HANDLE hCSVFile = INVALID_HANDLE_VALUE;
	GPO_FILTER_TARGET dwTarget = GPO_FILTER_UNKNOWN;
	BOOL bRes = FALSE;
	DWORD dwDataRead = 0;

	if (!pIeakFileData || !(pIeakFileData->pvData) || !(pIeakFileData->tFilePath))
	{
		DEBUG_LOG(D_WARNING, "PIEAK_FILE_DATA invalid.\r\nExiting now...");
		DoExit(D_ERROR);
	}

	// Determine if we need to create user or computer file in case of '[Machine||User]' 
	dwTarget = GetTargetGPO(pIeakFileData->tFilePath);
	if (dwTarget == GPO_FILTER_TARGET_MACHINE)
		tOutputDirectoryIEAKFile = TEXT(".\\Machine\\");
	else if (dwTarget == GPO_FILTER_TARGET_USER)
		tOutputDirectoryIEAKFile = TEXT(".\\User\\");

	hCSVFile = GetFileHandle(OUTPUT_FILE_CSV, tOutputDirectoryIEAKFile, OUTPUT_NAME_IEAK_FOLDER);
	if (hCSVFile == INVALID_HANDLE_VALUE)
	{
		DEBUG_LOG(D_WARNING, "Handle to hCSVFile is invalid.\r\nExiting now...");
		DoExit(D_ERROR);
	}

	switch (pIeakFileData->dwFileType)
	{
	case IEAK_INI_FILE:
		if (WriteFile(hCSVFile, TEXT("File;Section;Name;Value\r\n"), (DWORD)(_tcslen(TEXT("File;Section;Name;Value\r\n")) * sizeof (TCHAR)), &dwDataRead, NULL) == FALSE)
			goto writerror;
		bRes = PrintCSVIniData(pIeakFileData, hCSVFile);
		break;
	case IEAK_INF_FILE: // parse INF file like INI file
		if (WriteFile(hCSVFile, TEXT("File;Section;Name;Value\r\n"), (DWORD)(_tcslen(TEXT("File;Section;Name;Value\r\n")) * sizeof (TCHAR)), &dwDataRead, NULL) == FALSE)
			goto writerror;
		bRes = PrintCSVIniData(pIeakFileData, hCSVFile);
		break;
	default:
		if (WriteFile(hCSVFile, TEXT("File;DataSize;Data\r\n"), (DWORD)(_tcslen(TEXT("File;DataSize;Data\r\n")) * sizeof (TCHAR)), &dwDataRead, NULL) == FALSE)
			goto writerror;
		bRes = PrintCSVRawData(pIeakFileData, hCSVFile);
		break;
	}

	CloseHandle(hCSVFile);
	return TRUE;
writerror:
	DEBUG_LOG(D_WARNING, "Unable to write CSV DATA.\r\nExiting now...");
	DoExit(D_ERROR);
	return FALSE;
}

BOOL PrintCSVRawData(_In_ PIEAK_FILE_DATA pIeakFileData, _In_ HANDLE hCSVFile)
{
	DWORD dwDataRead = 0, dwSizeLength = 0;
	TCHAR tSize[100];
	PTCHAR tData = NULL;

	if (!pIeakFileData || !hCSVFile)
	{
		DEBUG_LOG(D_WARNING, "PIEAK_FILE_DATA or HANDLE invalid.\r\nExiting now...");
		DoExit(D_ERROR);
	}

	tData = GetBase64FromByte((PBYTE) (pIeakFileData->pvData), pIeakFileData->dwDataSize);
	dwSizeLength = _stprintf_s(tSize, 100, TEXT("%d"), (pIeakFileData->dwDataSize));

	if ((WriteFile(hCSVFile, pIeakFileData->tFilePath, (DWORD)(_tcslen(pIeakFileData->tFilePath) * sizeof (WCHAR)), &dwDataRead, NULL) == FALSE)
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

BOOL PrintCSVIniData(_In_ PIEAK_FILE_DATA pIeakFileData, _In_ HANDLE hCSVFile)
{
	DWORD dwDataRead = 0;
	PINI_FILE_DATA pIniFileData = NULL;

	if (!pIeakFileData || !hCSVFile || !(pIeakFileData->pvData))
	{
		DEBUG_LOG(D_WARNING, "PIEAK_FILE_DATA or HANDLE invalid.\r\nExiting now...");
		DoExit(D_ERROR);
	}
	pIniFileData = (PINI_FILE_DATA) pIeakFileData->pvData;

	if (PrintCSVData(pIniFileData, hCSVFile) == FALSE)
		goto writerror;

	return TRUE;
writerror:
	DEBUG_LOG(D_WARNING, "Unable to write CSV DATA.\r\nExiting now...");
	DoExit(D_ERROR);
	return FALSE;
}

BOOL PrintSTDOUTData(_In_ PIEAK_FILE_DATA pIeakFileData)
{
	BOOL bRes = FALSE;
	
	if (!pIeakFileData || !(pIeakFileData->pvData) || !(pIeakFileData->tFilePath))
	{
		DEBUG_LOG(D_WARNING, "PIEAK_FILE_DATA invalid.\r\nExiting now...");
		DoExit(D_ERROR);
	}

	printf("[IEAK]\r\n");
	switch (pIeakFileData->dwFileType)
	{
	case IEAK_INI_FILE:
		bRes = PrintSTDOUTIniData(pIeakFileData);
		break;
	case IEAK_INF_FILE: // parse INF as INI file
		bRes = PrintSTDOUTIniData(pIeakFileData);
		break;
	default:
		bRes = PrintSTDOUTRawData(pIeakFileData);
		break;
	}
	printf("[/IEAK]\r\n");
	return TRUE;
}

BOOL PrintSTDOUTRawData(_In_ PIEAK_FILE_DATA pIeakFileData)
{
	PTCHAR tData = NULL;
	
	if (!pIeakFileData || !(pIeakFileData->pvData) || !(pIeakFileData->tFilePath))
	{
		DEBUG_LOG(D_WARNING, "PIEAK_FILE_DATA invalid for IEAK file.\r\n");
		DoExit(D_WARNING);
	}

	tData = GetBase64FromByte((PBYTE )pIeakFileData->pvData, pIeakFileData->dwDataSize);

	printf("\t[RAW] File=%ws Size=%d Data=%ws\r\n", pIeakFileData->tFilePath, pIeakFileData->dwDataSize, tData);

	HeapFree(hCrawlerHeap, NULL, tData);
	return TRUE;
}

BOOL PrintSTDOUTIniData(_In_ PIEAK_FILE_DATA pIeakFileData)
{
	DWORD dwDataRead = 0;
	PINI_FILE_DATA pIniFileData = NULL;

	if (!pIeakFileData || !(pIeakFileData->pvData))
	{
		DEBUG_LOG(D_WARNING, "PIEAK_FILE_DATA invalid.\r\nExiting now...");
		DoExit(D_ERROR);
	}
	pIniFileData = (PINI_FILE_DATA) pIeakFileData->pvData;

	return (PrintSTDOUTData(pIniFileData));
}
