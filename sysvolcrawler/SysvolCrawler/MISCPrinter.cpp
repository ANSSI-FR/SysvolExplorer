/**************************************************
* SysvolCrawler - MISCPrinter.cpp
* AUTHOR: Luc Delsalle	
*
* Display or export data for odd file which should
* not be on a Sysvol folder
*	
* ANSSI/COSSI/DTO/BAI - 2014
***************************************************/

#include "MISCPrinter.h"

BOOL PrintData(_In_ PMISC_FILE_DATA pMiscData)
{
	BOOL bRes = TRUE;

	if (pMiscData == NULL)
	{
		DEBUG_LOG(D_ERROR, "pMiscData pointer invalid.\r\nExiting now...");
		DoExit(D_ERROR);
	}

	// Call every printer
	if ((bRes) && (pSyscrwlrOptions->bShouldPrintXML))
		bRes = PrintXMLData(pMiscData);

	if ((bRes) && (pSyscrwlrOptions->bShouldPrintCSV))
		bRes = PrintCSVData(pMiscData);

	if ((bRes) && (pSyscrwlrOptions->bShouldPrintSTDOUT))
		bRes = PrintSTDOUTData(pMiscData);

	return bRes;
}

BOOL PrintMiscDataHeader(_In_ PTCHAR tFilePath)
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
		HANDLE hXMLFile = GetFileHandle(OUTPUT_FILE_XML, OUTPUT_DIRECTORY_MISC_FILE, OUTPUT_NAME_MISC_FILE);

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
			|| (WriteFile(hXMLFile, OUTPUT_NAME_MISC_FILE, (DWORD)(_tcslen(OUTPUT_NAME_MISC_FILE) * sizeof (TCHAR)), &dwDataRead, NULL) == FALSE)
			|| (WriteFile(hXMLFile, TEXT(".xml>\r\n"), (DWORD)(_tcslen(TEXT(".xml>\r\n")) * sizeof (TCHAR)), &dwDataRead, NULL) == FALSE))
				goto writerror;
		}

		if ((WriteFile(hXMLFile, TEXT("<MISCellaneousFile Filename=\""), (DWORD)(_tcslen(TEXT("<MISCellaneousFile Filename=\"")) * sizeof (TCHAR)), &dwDataRead, NULL) == FALSE)
		|| (WriteFile(hXMLFile, tFilePath, (DWORD)(_tcslen(tFilePath) * sizeof (TCHAR)), &dwDataRead, NULL) == FALSE)
		|| (WriteFile(hXMLFile, TEXT("\">\r\n"), (DWORD)(_tcslen(TEXT("\">\r\n")) * sizeof (TCHAR)), &dwDataRead, NULL) == FALSE))
			goto writerror;

		CloseHandle(hXMLFile);
	}

	if (pSyscrwlrOptions->bShouldPrintCSV)
	{
		HANDLE hCSVFile = GetFileHandle(OUTPUT_FILE_CSV, OUTPUT_DIRECTORY_MISC_FILE, OUTPUT_NAME_MISC_FILE);
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
	DEBUG_LOG(D_WARNING, "Unable to write DATA HEADER for MISC printer.\r\nExiting now...");
	DoExit(D_ERROR);
	return FALSE;
}

BOOL PrintMiscDataFooter(_In_ PTCHAR tFilePath)
{
	DWORD dwDataRead = 0;

	// Hack for closing xml document. Ugly.
	if (pSyscrwlrOptions->bShouldPrintXML)
	{
		HANDLE hXMLFile = GetFileHandle(OUTPUT_FILE_XML, OUTPUT_DIRECTORY_MISC_FILE, OUTPUT_NAME_MISC_FILE);
		if (WriteFile(hXMLFile, TEXT("</MISCellaneousFile>\r\n"), (DWORD)(_tcslen(TEXT("</MISCellaneousFile>\r\n")) * sizeof (TCHAR)), &dwDataRead, NULL) == FALSE)
			goto writerror;
		CloseHandle(hXMLFile);
	}
	return TRUE;

writerror:
	DEBUG_LOG(D_WARNING, "Unable to write DATA FOOTER for MISC printer.\r\nExiting now...");
	DoExit(D_ERROR);
	return FALSE;
}

BOOL PrintXMLData(_In_ PMISC_FILE_DATA pMiscData)
{
	DWORD dwDataRead = 0, dwSizeLength = 0;
	HANDLE hMISCFile = GetFileHandle(OUTPUT_FILE_XML, OUTPUT_DIRECTORY_MISC_FILE, OUTPUT_NAME_MISC_FILE);
	TCHAR tSize[100];
	PTCHAR tData = NULL;

	if (!pMiscData || !(pMiscData->pbData) || !(pMiscData->tFilePath))
	{
		DEBUG_LOG(D_WARNING, "PMISC_FILE_DATA invalid for MISC file.\r\n");
		DoExit(D_WARNING);
	}

	if (hMISCFile == INVALID_HANDLE_VALUE)
	{
		DEBUG_LOG(D_WARNING, "Handle to hMISCFile is invalid.\r\nExiting now...");
		DoExit(D_ERROR);
	}

	tData = GetBase64FromByte(pMiscData->pbData, pMiscData->dwDataSize);
	dwSizeLength = _stprintf_s(tSize, 100, TEXT("%d"), (pMiscData->dwDataSize));

	if ((WriteFile(hMISCFile, TEXT("\t\t<Data size=\""), (DWORD)(_tcslen(TEXT("\t\t<Data size=\"")) * sizeof (TCHAR)), &dwDataRead, NULL) == FALSE)
		|| (WriteFile(hMISCFile, tSize, (DWORD)(_tcslen(tSize) * sizeof (WCHAR)), &dwDataRead, NULL) == FALSE)
		|| (WriteFile(hMISCFile, TEXT("\" data=\""), (DWORD)(_tcslen(TEXT("\" data=\"")) * sizeof (TCHAR)), &dwDataRead, NULL) == FALSE)
		|| (WriteFile(hMISCFile, tData, (DWORD)(_tcslen(tData) * sizeof (WCHAR)), &dwDataRead, NULL) == FALSE)
		|| (WriteFile(hMISCFile, TEXT("\"/>\r\n"), (DWORD)(_tcslen(TEXT("\"/>\r\n")) * sizeof (TCHAR)), &dwDataRead, NULL) == FALSE))
		goto writerror;

	HeapFree(hCrawlerHeap, NULL, tData);
	CloseHandle(hMISCFile);
	return TRUE;
writerror:
	DEBUG_LOG(D_WARNING, "Unable to write XML DATA.\r\nExiting now...");
	DoExit(D_ERROR);
	return FALSE;
}

BOOL PrintCSVData(_In_ PMISC_FILE_DATA pMiscData)
{
	DWORD dwDataRead = 0, dwSizeLength = 0;
	HANDLE hMISCFile = GetFileHandle(OUTPUT_FILE_CSV, OUTPUT_DIRECTORY_MISC_FILE, OUTPUT_NAME_MISC_FILE);
	TCHAR tSize[100];
	PTCHAR tData = NULL;

	if (!pMiscData || !(pMiscData->pbData) || !(pMiscData->tFilePath))
	{
		DEBUG_LOG(D_WARNING, "PMISC_FILE_DATA invalid for MISC file.\r\n");
		DoExit(D_WARNING);
	}

	if (hMISCFile == INVALID_HANDLE_VALUE)
	{
		DEBUG_LOG(D_WARNING, "Handle to hMISCFile is invalid.\r\nExiting now...");
		DoExit(D_ERROR);
	}

	tData = GetBase64FromByte(pMiscData->pbData, pMiscData->dwDataSize);
	dwSizeLength = _stprintf_s(tSize, 100, TEXT("%d"), (pMiscData->dwDataSize));

	if ((WriteFile(hMISCFile, pMiscData->tFilePath, (DWORD)(_tcslen(pMiscData->tFilePath) * sizeof (WCHAR)), &dwDataRead, NULL) == FALSE)
		|| (WriteFile(hMISCFile, TEXT(";"), (DWORD)(_tcslen(TEXT(";")) * sizeof (TCHAR)), &dwDataRead, NULL) == FALSE)
		|| (WriteFile(hMISCFile, tSize, (DWORD)(_tcslen(tSize) * sizeof (WCHAR)), &dwDataRead, NULL) == FALSE)
		|| (WriteFile(hMISCFile, TEXT(";"), (DWORD)(_tcslen(TEXT(";")) * sizeof (TCHAR)), &dwDataRead, NULL) == FALSE)
		|| (WriteFile(hMISCFile, tData, (DWORD)(_tcslen(tData) * sizeof (WCHAR)), &dwDataRead, NULL) == FALSE)
		|| (WriteFile(hMISCFile, TEXT("\r\n"), (DWORD)(_tcslen(TEXT("\r\n")) * sizeof (TCHAR)), &dwDataRead, NULL) == FALSE))
		goto writerror;
	
	HeapFree(hCrawlerHeap, NULL, tData);
	CloseHandle(hMISCFile);
	return TRUE;
writerror:
	DEBUG_LOG(D_WARNING, "Unable to write CSV DATA.\r\nExiting now...");
	DoExit(D_ERROR);
	return FALSE;
}

BOOL PrintSTDOUTData(_In_ PMISC_FILE_DATA pMiscData)
{
	PTCHAR tData = NULL;
	
	if (!pMiscData || !(pMiscData->pbData) || !(pMiscData->tFilePath))
	{
		DEBUG_LOG(D_WARNING, "PMISC_FILE_DATA invalid for MISC file.\r\n");
		DoExit(D_WARNING);
	}

	tData = GetBase64FromByte(pMiscData->pbData, pMiscData->dwDataSize);

	printf("[MISC] File=%ws Size=%d Data=%ws\r\n", pMiscData->tFilePath, pMiscData->dwDataSize, tData);

	HeapFree(hCrawlerHeap, NULL, tData);
	return TRUE;
}