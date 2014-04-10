/**************************************************
* SysvolCrawler - MISCPrinter.cpp
* AUTHOR: Luc Delsalle
*
* Display or export ADM data
*
* ANSSI/COSSI/DTO/BAI - 2014
***************************************************/

#include "ADMPrinter.h"

BOOL PrintData(_In_ PADM_FILE_DATA pAdmData)
{
	BOOL bRes = TRUE;

	if (pAdmData == NULL)
	{
		DEBUG_LOG(D_ERROR, "pAdmData pointer invalid.\r\nExiting now...");
		DoExit(D_ERROR);
	}

	// Call every printer
	if ((bRes) && (pSyscrwlrOptions->bShouldPrintXML))
		bRes = PrintXMLData(pAdmData);

	if ((bRes) && (pSyscrwlrOptions->bShouldPrintCSV))
		bRes = PrintCSVData(pAdmData);

	if ((bRes) && (pSyscrwlrOptions->bShouldPrintSTDOUT))
		bRes = PrintSTDOUTData(pAdmData);

	return bRes;
}

BOOL PrintAdmDataHeader(_In_ PTCHAR tFilePath)
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
		HANDLE hXMLFile = GetFileHandle(OUTPUT_FILE_XML, OUTPUT_DIRECTORY_ADM_FILE, OUTPUT_NAME_ADM_FILE);

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
				|| (WriteFile(hXMLFile, OUTPUT_NAME_ADM_FILE, (DWORD)(_tcslen(OUTPUT_NAME_ADM_FILE) * sizeof (TCHAR)), &dwDataRead, NULL) == FALSE)
				|| (WriteFile(hXMLFile, TEXT(".xml>\r\n"), (DWORD)(_tcslen(TEXT(".xml>\r\n")) * sizeof (TCHAR)), &dwDataRead, NULL) == FALSE))
				goto writerror;
		}

		if ((WriteFile(hXMLFile, TEXT("<AdmFile Filename=\""), (DWORD)(_tcslen(TEXT("<AdmFile Filename=\"")) * sizeof (TCHAR)), &dwDataRead, NULL) == FALSE)
			|| (WriteFile(hXMLFile, tFilePath, (DWORD)(_tcslen(tFilePath) * sizeof (TCHAR)), &dwDataRead, NULL) == FALSE)
			|| (WriteFile(hXMLFile, TEXT("\">\r\n"), (DWORD)(_tcslen(TEXT("\">\r\n")) * sizeof (TCHAR)), &dwDataRead, NULL) == FALSE))
			goto writerror;

		CloseHandle(hXMLFile);
	}

	if (pSyscrwlrOptions->bShouldPrintCSV)
	{
		HANDLE hCSVFile = GetFileHandle(OUTPUT_FILE_CSV, OUTPUT_DIRECTORY_ADM_FILE, OUTPUT_NAME_ADM_FILE);
		LARGE_INTEGER liFileSize;

		if (!GetFileSizeEx(hCSVFile, &liFileSize))
		{
			DEBUG_LOG(D_WARNING, "Unable to determine file size.\r\nExiting now...");
			DoExit(D_ERROR);
		}

		if ((liFileSize.HighPart == 0) && (liFileSize.LowPart == 0))
		{
			if (WriteFile(hCSVFile, TEXT("File;Size;Data\r\n"), (DWORD)(_tcslen(TEXT("File;Size;Data\r\n")) * sizeof (TCHAR)), &dwDataRead, NULL) == FALSE)
				goto writerror;
		}
		CloseHandle(hCSVFile);
	}
	return TRUE;

writerror:
	DEBUG_LOG(D_WARNING, "Unable to write DATA HEADER for ADM printer.\r\nExiting now...");
	DoExit(D_ERROR);
	return FALSE;
}

BOOL PrintAdmDataFooter(_In_ PTCHAR tFilePath)
{
	DWORD dwDataRead = 0;

	// Hack for closing xml document. Ugly.
	if (pSyscrwlrOptions->bShouldPrintXML)
	{
		HANDLE hXMLFile = GetFileHandle(OUTPUT_FILE_XML, OUTPUT_DIRECTORY_ADM_FILE, OUTPUT_NAME_ADM_FILE);
		if (WriteFile(hXMLFile, TEXT("</AdmFile>\r\n"), (DWORD)(_tcslen(TEXT("</AdmFile>\r\n")) * sizeof (TCHAR)), &dwDataRead, NULL) == FALSE)
			goto writerror;
		CloseHandle(hXMLFile);
	}
	return TRUE;

writerror:
	DEBUG_LOG(D_WARNING, "Unable to write DATA FOOTER for ADM printer.\r\nExiting now...");
	DoExit(D_ERROR);
	return FALSE;
}

BOOL PrintXMLData(_In_ PADM_FILE_DATA pAdmData)
{
	DWORD dwDataRead = 0, dwSizeLength = 0;
	HANDLE hAdmFile = GetFileHandle(OUTPUT_FILE_XML, OUTPUT_DIRECTORY_ADM_FILE, OUTPUT_NAME_ADM_FILE);
	TCHAR tSize[100];
	PTCHAR tData = NULL;

	if (!pAdmData || !(pAdmData->pbData) || !(pAdmData->tFilePath))
	{
		DEBUG_LOG(D_WARNING, "PADM_FILE_DATA invalid for ADM file.\r\n");
		DoExit(D_WARNING);
	}

	if (hAdmFile == INVALID_HANDLE_VALUE)
	{
		DEBUG_LOG(D_WARNING, "Handle to hAdmFile is invalid.\r\nExiting now...");
		DoExit(D_ERROR);
	}

	tData = GetBase64FromByte(pAdmData->pbData, pAdmData->dwDataSize);
	dwSizeLength = _stprintf_s(tSize, 100, TEXT("%d"), (pAdmData->dwDataSize));

	if ((WriteFile(hAdmFile, TEXT("\t\t<Data size=\""), (DWORD)(_tcslen(TEXT("\t\t<Data size=\"")) * sizeof (TCHAR)), &dwDataRead, NULL) == FALSE)
		|| (WriteFile(hAdmFile, tSize, (DWORD)(_tcslen(tSize) * sizeof (WCHAR)), &dwDataRead, NULL) == FALSE)
		|| (WriteFile(hAdmFile, TEXT("\" data=\""), (DWORD)(_tcslen(TEXT("\" data=\"")) * sizeof (TCHAR)), &dwDataRead, NULL) == FALSE)
		|| (WriteFile(hAdmFile, tData, (DWORD)(_tcslen(tData) * sizeof (WCHAR)), &dwDataRead, NULL) == FALSE)
		|| (WriteFile(hAdmFile, TEXT("\"/>\r\n"), (DWORD)(_tcslen(TEXT("\"/>\r\n")) * sizeof (TCHAR)), &dwDataRead, NULL) == FALSE))
		goto writerror;

	HeapFree(hCrawlerHeap, NULL, tData);
	CloseHandle(hAdmFile);
	return TRUE;
writerror:
	DEBUG_LOG(D_WARNING, "Unable to write XML DATA.\r\nExiting now...");
	DoExit(D_ERROR);
	return FALSE;
}

BOOL PrintCSVData(_In_ PADM_FILE_DATA pAdmData)
{
	DWORD dwDataRead = 0, dwSizeLength = 0;
	HANDLE hADMFile = GetFileHandle(OUTPUT_FILE_CSV, OUTPUT_DIRECTORY_ADM_FILE, OUTPUT_NAME_ADM_FILE);
	TCHAR tSize[100];
	PTCHAR tData = NULL;

	if (!pAdmData || !(pAdmData->pbData) || !(pAdmData->tFilePath))
	{
		DEBUG_LOG(D_WARNING, "PADM_FILE_DATA invalid for ADM file.\r\n");
		DoExit(D_WARNING);
	}

	if (hADMFile == INVALID_HANDLE_VALUE)
	{
		DEBUG_LOG(D_WARNING, "Handle to hADMFile is invalid.\r\nExiting now...");
		DoExit(D_ERROR);
	}

	tData = GetBase64FromByte(pAdmData->pbData, pAdmData->dwDataSize);
	dwSizeLength = _stprintf_s(tSize, 100, TEXT("%d"), (pAdmData->dwDataSize));

	if ((WriteFile(hADMFile, pAdmData->tFilePath, (DWORD)(_tcslen(pAdmData->tFilePath) * sizeof (WCHAR)), &dwDataRead, NULL) == FALSE)
		|| (WriteFile(hADMFile, TEXT(";"), (DWORD)(_tcslen(TEXT(";")) * sizeof (TCHAR)), &dwDataRead, NULL) == FALSE)
		|| (WriteFile(hADMFile, tSize, (DWORD)(_tcslen(tSize) * sizeof (WCHAR)), &dwDataRead, NULL) == FALSE)
		|| (WriteFile(hADMFile, TEXT(";"), (DWORD)(_tcslen(TEXT(";")) * sizeof (TCHAR)), &dwDataRead, NULL) == FALSE)
		|| (WriteFile(hADMFile, tData, (DWORD)(_tcslen(tData) * sizeof (WCHAR)), &dwDataRead, NULL) == FALSE)
		|| (WriteFile(hADMFile, TEXT("\r\n"), (DWORD)(_tcslen(TEXT("\r\n")) * sizeof (TCHAR)), &dwDataRead, NULL) == FALSE))
		goto writerror;

	HeapFree(hCrawlerHeap, NULL, tData);
	CloseHandle(hADMFile);
	return TRUE;
writerror:
	DEBUG_LOG(D_WARNING, "Unable to write CSV DATA.\r\nExiting now...");
	DoExit(D_ERROR);
	return FALSE;
}

BOOL PrintSTDOUTData(_In_ PADM_FILE_DATA pAdmData)
{
	PTCHAR tData = NULL;

	if (!pAdmData || !(pAdmData->pbData) || !(pAdmData->tFilePath))
	{
		DEBUG_LOG(D_WARNING, "PADM_FILE_DATA invalid for ADM file.\r\n");
		DoExit(D_WARNING);
	}

	tData = GetBase64FromByte(pAdmData->pbData, pAdmData->dwDataSize);

	printf("[ADM] File=%ws Size=%d Data=%ws\r\n", pAdmData->tFilePath, pAdmData->dwDataSize, tData);

	HeapFree(hCrawlerHeap, NULL, tData);
	return TRUE;
}