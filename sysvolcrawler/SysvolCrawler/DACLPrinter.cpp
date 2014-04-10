/**************************************************
* SysvolCrawler - DACLPrinter.h
* AUTHOR: Luc Delsalle
*
* Display or export DACL data
*
* ANSSI/COSSI/DTO/BAI - 2014
***************************************************/

#include "DACLPrinter.h"

BOOL PrintData(_In_ PDACL_FILE_DATA pDaclData)
{
	BOOL bRes = TRUE;

	if (pDaclData == NULL)
	{
		DEBUG_LOG(D_ERROR, "pDaclData pointer invalid.\r\nExiting now...");
		DoExit(D_ERROR);
	}

	// Call every printer
	if ((bRes) && (pSyscrwlrOptions->bShouldPrintXML))
		bRes = PrintXMLData(pDaclData);

	if ((bRes) && (pSyscrwlrOptions->bShouldPrintCSV))
		bRes = PrintCSVData(pDaclData);

	if ((bRes) && (pSyscrwlrOptions->bShouldPrintSTDOUT))
		bRes = PrintSTDOUTData(pDaclData);

	return bRes;
}

BOOL PrintDaclDataHeader(_In_ PTCHAR tFilePath)
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
		HANDLE hXMLFile = GetFileHandle(OUTPUT_FILE_XML, OUTPUT_DIRECTORY_DACL_FILE, OUTPUT_NAME_DACL_FILE);

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
				|| (WriteFile(hXMLFile, OUTPUT_NAME_DACL_FILE, (DWORD)(_tcslen(OUTPUT_NAME_DACL_FILE) * sizeof (TCHAR)), &dwDataRead, NULL) == FALSE)
				|| (WriteFile(hXMLFile, TEXT(".xml>\r\n"), (DWORD)(_tcslen(TEXT(".xml>\r\n")) * sizeof (TCHAR)), &dwDataRead, NULL) == FALSE))
				goto writerror;
		}

		if ((WriteFile(hXMLFile, TEXT("<FileDACL Filename=\""), (DWORD)(_tcslen(TEXT("<FileDACL Filename=\"")) * sizeof (TCHAR)), &dwDataRead, NULL) == FALSE)
			|| (WriteFile(hXMLFile, tFilePath, (DWORD)(_tcslen(tFilePath) * sizeof (TCHAR)), &dwDataRead, NULL) == FALSE)
			|| (WriteFile(hXMLFile, TEXT("\">\r\n"), (DWORD)(_tcslen(TEXT("\">\r\n")) * sizeof (TCHAR)), &dwDataRead, NULL) == FALSE))
			goto writerror;

		CloseHandle(hXMLFile);
	}

	if (pSyscrwlrOptions->bShouldPrintCSV)
	{
		HANDLE hCSVFile = GetFileHandle(OUTPUT_FILE_CSV, OUTPUT_DIRECTORY_DACL_FILE, OUTPUT_NAME_DACL_FILE);
		LARGE_INTEGER liFileSize;

		if (!GetFileSizeEx(hCSVFile, &liFileSize))
		{
			DEBUG_LOG(D_WARNING, "Unable to determine file size.\r\nExiting now...");
			DoExit(D_ERROR);
		}

		if ((liFileSize.HighPart == 0) && (liFileSize.LowPart == 0))
		{
			if (WriteFile(hCSVFile, TEXT("File;Owner;Dacl\r\n"), (DWORD)(_tcslen(TEXT("File;Owner;Dacl\r\n")) * sizeof (TCHAR)), &dwDataRead, NULL) == FALSE)
				goto writerror;
		}
		CloseHandle(hCSVFile);
	}
	return TRUE;

writerror:
	DEBUG_LOG(D_WARNING, "Unable to write DATA HEADER for DACL printer.\r\nExiting now...");
	DoExit(D_ERROR);
	return FALSE;
}

BOOL PrintDaclDataFooter(_In_ PTCHAR tFilePath)
{
	DWORD dwDataRead = 0;

	// Hack for closing xml document. Ugly.
	if (pSyscrwlrOptions->bShouldPrintXML)
	{
		HANDLE hXMLFile = GetFileHandle(OUTPUT_FILE_XML, OUTPUT_DIRECTORY_DACL_FILE, OUTPUT_NAME_DACL_FILE);
		if (WriteFile(hXMLFile, TEXT("</FileDACL>\r\n"), (DWORD)(_tcslen(TEXT("</FileDACL>\r\n")) * sizeof (TCHAR)), &dwDataRead, NULL) == FALSE)
			goto writerror;
		CloseHandle(hXMLFile);
	}
	return TRUE;

writerror:
	DEBUG_LOG(D_WARNING, "Unable to write DATA FOOTER for DACL printer.\r\nExiting now...");
	DoExit(D_ERROR);
	return FALSE;
}

BOOL PrintXMLData(_In_ PDACL_FILE_DATA pDaclData)
{
	HANDLE hDACLFile = GetFileHandle(OUTPUT_FILE_XML, OUTPUT_DIRECTORY_DACL_FILE, OUTPUT_NAME_DACL_FILE);
	DWORD dwDataRead = 0;

	if (!pDaclData || !(pDaclData->tOwnerSid) || !(pDaclData->tSDDL))
	{
		DEBUG_LOG(D_WARNING, "PDACL_FILE_DATA invalid for current file.\r\n");
		DoExit(D_WARNING);
	}

	if (hDACLFile == INVALID_HANDLE_VALUE)
	{
		DEBUG_LOG(D_WARNING, "Handle to hMISCFile is invalid.\r\nExiting now...");
		DoExit(D_ERROR);
	}

	if ((WriteFile(hDACLFile, TEXT("\t\t<SD owner=\""), (DWORD)(_tcslen(TEXT("\t\t<SD owner=\"")) * sizeof (TCHAR)), &dwDataRead, NULL) == FALSE)
		|| (WriteFile(hDACLFile, pDaclData->tOwnerSid, (DWORD)(_tcslen(pDaclData->tOwnerSid) * sizeof (WCHAR)), &dwDataRead, NULL) == FALSE)
		|| (WriteFile(hDACLFile, TEXT("\" dacl=\""), (DWORD)(_tcslen(TEXT("\" dacl=\"")) * sizeof (TCHAR)), &dwDataRead, NULL) == FALSE)
		|| (WriteFile(hDACLFile, pDaclData->tSDDL, (DWORD)(_tcslen(pDaclData->tSDDL) * sizeof (WCHAR)), &dwDataRead, NULL) == FALSE)
		|| (WriteFile(hDACLFile, TEXT("\"/>\r\n"), (DWORD)(_tcslen(TEXT("\"/>\r\n")) * sizeof (TCHAR)), &dwDataRead, NULL) == FALSE))
		goto writerror;

	CloseHandle(hDACLFile);
	return TRUE;
writerror:
	DEBUG_LOG(D_WARNING, "Unable to write XML DATA.\r\nExiting now...");
	DoExit(D_ERROR);
	return FALSE;
}

BOOL PrintCSVData(_In_ PDACL_FILE_DATA pDaclData)
{
	HANDLE hDACLFile = GetFileHandle(OUTPUT_FILE_CSV, OUTPUT_DIRECTORY_DACL_FILE, OUTPUT_NAME_DACL_FILE);
	DWORD dwDataRead = 0;

	if (!pDaclData || !(pDaclData->tOwnerSid) || !(pDaclData->tSDDL))
	{
		DEBUG_LOG(D_WARNING, "PDACL_FILE_DATA invalid for current file.\r\n");
		DoExit(D_WARNING);
	}

	if (hDACLFile == INVALID_HANDLE_VALUE)
	{
		DEBUG_LOG(D_WARNING, "Handle to hDACLFile is invalid.\r\nExiting now...");
		DoExit(D_ERROR);
	}

	PTCHAR tEscapedOwnerSid = EscapeCSVString(pDaclData->tOwnerSid);
	PTCHAR tEscapedDacl = EscapeCSVString(pDaclData->tSDDL);


	if ((WriteFile(hDACLFile, pDaclData->tFilePath, (DWORD)(_tcslen(pDaclData->tFilePath) * sizeof (WCHAR)), &dwDataRead, NULL) == FALSE)
		|| (WriteFile(hDACLFile, TEXT(";"), (DWORD)(_tcslen(TEXT(";")) * sizeof (TCHAR)), &dwDataRead, NULL) == FALSE)
		|| (WriteFile(hDACLFile, tEscapedOwnerSid, (DWORD)(_tcslen(tEscapedOwnerSid) * sizeof (WCHAR)), &dwDataRead, NULL) == FALSE)
		|| (WriteFile(hDACLFile, TEXT(";"), (DWORD)(_tcslen(TEXT(";")) * sizeof (TCHAR)), &dwDataRead, NULL) == FALSE)
		|| (WriteFile(hDACLFile, tEscapedDacl, (DWORD)(_tcslen(tEscapedDacl) * sizeof (WCHAR)), &dwDataRead, NULL) == FALSE)
		|| (WriteFile(hDACLFile, TEXT("\r\n"), (DWORD)(_tcslen(TEXT("\r\n")) * sizeof (TCHAR)), &dwDataRead, NULL) == FALSE))
		goto writerror;

	HeapFree(hCrawlerHeap, NULL, tEscapedOwnerSid);
	HeapFree(hCrawlerHeap, NULL, tEscapedDacl);
	CloseHandle(hDACLFile);
	return TRUE;
writerror:
	DEBUG_LOG(D_WARNING, "Unable to write CSV DATA.\r\nExiting now...");
	DoExit(D_ERROR);
	return FALSE;
}

BOOL PrintSTDOUTData(_In_ PDACL_FILE_DATA pDaclData)
{
	PTCHAR tData = NULL;

	if (!pDaclData || !(pDaclData->tOwnerSid) || !(pDaclData->tSDDL))
	{
		DEBUG_LOG(D_WARNING, "PDACL_FILE_DATA invalid for current file.\r\n");
		DoExit(D_WARNING);
	}

	printf("[DACL] File=%ws Owner=%ws SDDL=%ws\r\n", pDaclData->tFilePath, pDaclData->tOwnerSid, pDaclData->tSDDL);

	HeapFree(hCrawlerHeap, NULL, tData);
	return TRUE;
}
