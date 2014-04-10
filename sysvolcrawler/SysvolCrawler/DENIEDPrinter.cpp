/**************************************************
* SysvolCrawler - DENIEDPrinter.cpp
* AUTHOR: Luc Delsalle	
*
* Display data for file which couldn't be opened
* during CreateFile attempt
*	
* ANSSI/COSSI/DTO/BAI - 2014
***************************************************/

#include "DENIEDPrinter.h"

BOOL PrintData(_In_ PDENIED_FILE_DATA pDeniedData)
{
	BOOL bRes = TRUE;

	if (pDeniedData == NULL)
	{
		DEBUG_LOG(D_ERROR, "DENIED_FILE_DATA pointer invalid.\r\nExiting now...");
		DoExit(D_ERROR);
	}

	// Call every printer
	if ((bRes) && (pSyscrwlrOptions->bShouldPrintXML))
		bRes = PrintXMLData(pDeniedData);

	if ((bRes) && (pSyscrwlrOptions->bShouldPrintCSV))
		bRes = PrintCSVData(pDeniedData);

	if ((bRes) && (pSyscrwlrOptions->bShouldPrintSTDOUT))
		bRes = PrintSTDOUTData(pDeniedData);

	return bRes;
}

BOOL PrintDeniedDataHeader(_In_ PTCHAR tFilePath)
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
		HANDLE hXMLFile = GetFileHandle(OUTPUT_FILE_XML, OUTPUT_DIRECTORY_DENIED_FILE, OUTPUT_NAME_DENIED_FILE);

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
			|| (WriteFile(hXMLFile, OUTPUT_NAME_DENIED_FILE, (DWORD)(_tcslen(OUTPUT_NAME_DENIED_FILE) * sizeof (TCHAR)), &dwDataRead, NULL) == FALSE)
			|| (WriteFile(hXMLFile, TEXT(".xml>\r\n"), (DWORD)(_tcslen(TEXT(".xml>\r\n")) * sizeof (TCHAR)), &dwDataRead, NULL) == FALSE))
				goto writerror;
		}

		if ((WriteFile(hXMLFile, TEXT("<DENIEDFileFile Filename=\""), (DWORD)(_tcslen(TEXT("<DENIEDFileFile Filename=\"")) * sizeof (TCHAR)), &dwDataRead, NULL) == FALSE)
		|| (WriteFile(hXMLFile, tFilePath, (DWORD)(_tcslen(tFilePath) * sizeof (TCHAR)), &dwDataRead, NULL) == FALSE)
		|| (WriteFile(hXMLFile, TEXT("\">\r\n"), (DWORD)(_tcslen(TEXT("\">\r\n")) * sizeof (TCHAR)), &dwDataRead, NULL) == FALSE))
			goto writerror;

		CloseHandle(hXMLFile);
	}

	if (pSyscrwlrOptions->bShouldPrintCSV)
	{
		HANDLE hCSVFile = GetFileHandle(OUTPUT_FILE_CSV, OUTPUT_DIRECTORY_DENIED_FILE, OUTPUT_NAME_DENIED_FILE);
		LARGE_INTEGER liFileSize;

		if (!GetFileSizeEx(hCSVFile, &liFileSize))
		{
			DEBUG_LOG(D_WARNING, "Unable to determine file size.\r\nExiting now...");
			DoExit(D_ERROR);
		}

		if ((liFileSize.HighPart == 0) && (liFileSize.LowPart == 0))
		{
			if (WriteFile(hCSVFile, TEXT("File;Type\r\n"), (DWORD)(_tcslen(TEXT("File;Type\r\n")) * sizeof (TCHAR)), &dwDataRead, NULL) == FALSE)
				goto writerror;
		}
		CloseHandle(hCSVFile);
	}
	return TRUE;

writerror:
	DEBUG_LOG(D_WARNING, "Unable to write DATA HEADER for DENIED printer.\r\nExiting now...");
	DoExit(D_ERROR);
	return FALSE;
}

BOOL PrintDeniedDataFooter(_In_ PTCHAR tFilePath)
{
	DWORD dwDataRead = 0;

	// Hack for closing xml document. Ugly.
	if (pSyscrwlrOptions->bShouldPrintXML)
	{
		HANDLE hXMLFile = GetFileHandle(OUTPUT_FILE_XML, OUTPUT_DIRECTORY_DENIED_FILE, OUTPUT_NAME_DENIED_FILE);
		if (WriteFile(hXMLFile, TEXT("</DENIEDFileFile>\r\n"), (DWORD)(_tcslen(TEXT("</DENIEDFileFile>\r\n")) * sizeof (TCHAR)), &dwDataRead, NULL) == FALSE)
			goto writerror;
		CloseHandle(hXMLFile);
	}
	return TRUE;

writerror:
	DEBUG_LOG(D_WARNING, "Unable to write DATA FOOTER for DENIED printer.\r\nExiting now...");
	DoExit(D_ERROR);
	return FALSE;
}

BOOL PrintXMLData(_In_ PDENIED_FILE_DATA pDeniedData)
{
	DWORD dwDataRead = 0, dwSizeLength = 0;
	HANDLE hDENIEDFile = GetFileHandle(OUTPUT_FILE_XML, OUTPUT_DIRECTORY_DENIED_FILE, OUTPUT_NAME_DENIED_FILE);
	PTCHAR tFileType = NULL;

	if (!pDeniedData || !(pDeniedData->tFilePath))
	{
		DEBUG_LOG(D_WARNING, "PDENIED_FILE_DATA invalid for DENIED file.\r\n");
		DoExit(D_WARNING);
	}

	if (hDENIEDFile == INVALID_HANDLE_VALUE)
	{
		DEBUG_LOG(D_WARNING, "Handle to hDENIEDFile is invalid.\r\nExiting now...");
		DoExit(D_ERROR);
	}

	if (pDeniedData->bIsADirectory)
		tFileType = TEXT("directory");
	else
		tFileType = TEXT("file");

	if ((WriteFile(hDENIEDFile, TEXT("\t<DeniedFile filepath=\""), (DWORD)(_tcslen(TEXT("\t<DeniedFile filepath=\"")) * sizeof (TCHAR)), &dwDataRead, NULL) == FALSE)
		|| (WriteFile(hDENIEDFile, pDeniedData->tFilePath, (DWORD)(_tcslen(pDeniedData->tFilePath) * sizeof (WCHAR)), &dwDataRead, NULL) == FALSE)
		|| (WriteFile(hDENIEDFile, TEXT("\" type=\""), (DWORD)(_tcslen(TEXT("\" type=\"")) * sizeof (TCHAR)), &dwDataRead, NULL) == FALSE)
		|| (WriteFile(hDENIEDFile, tFileType, (DWORD)(_tcslen(tFileType) * sizeof (WCHAR)), &dwDataRead, NULL) == FALSE)
		|| (WriteFile(hDENIEDFile, TEXT("\"/>\r\n"), (DWORD)(_tcslen(TEXT("\"/>\r\n")) * sizeof (TCHAR)), &dwDataRead, NULL) == FALSE))
		goto writerror;

	CloseHandle(hDENIEDFile);
	return TRUE;
writerror:
	DEBUG_LOG(D_WARNING, "Unable to write XML DATA.\r\nExiting now...");
	DoExit(D_ERROR);
	return FALSE;
}

BOOL PrintCSVData(_In_ PDENIED_FILE_DATA pDeniedData)
{
	DWORD dwDataRead = 0, dwSizeLength = 0;
	HANDLE hDENIEDFile = GetFileHandle(OUTPUT_FILE_CSV, OUTPUT_DIRECTORY_DENIED_FILE, OUTPUT_NAME_DENIED_FILE);
	PTCHAR tFileType = NULL;

	if (!pDeniedData || !(pDeniedData->tFilePath))
	{
		DEBUG_LOG(D_WARNING, "PDENIED_FILE_DATA invalid for DENIED file.\r\n");
		DoExit(D_WARNING);
	}

	if (pDeniedData == INVALID_HANDLE_VALUE)
	{
		DEBUG_LOG(D_WARNING, "Handle to hDENIEDFile is invalid.\r\nExiting now...");
		DoExit(D_ERROR);
	}

	if (pDeniedData->bIsADirectory)
		tFileType = TEXT("directory");
	else
		tFileType = TEXT("file");

	if ((WriteFile(hDENIEDFile, pDeniedData->tFilePath, (DWORD)(_tcslen(pDeniedData->tFilePath) * sizeof (WCHAR)), &dwDataRead, NULL) == FALSE)
		|| (WriteFile(hDENIEDFile, TEXT(";"), (DWORD)(_tcslen(TEXT(";")) * sizeof (TCHAR)), &dwDataRead, NULL) == FALSE)
		|| (WriteFile(hDENIEDFile, tFileType, (DWORD)(_tcslen(tFileType) * sizeof (WCHAR)), &dwDataRead, NULL) == FALSE)
		|| (WriteFile(hDENIEDFile, TEXT("\r\n"), (DWORD)(_tcslen(TEXT("\r\n")) * sizeof (TCHAR)), &dwDataRead, NULL) == FALSE))
		goto writerror;
	
	CloseHandle(hDENIEDFile);
	return TRUE;
writerror:
	DEBUG_LOG(D_WARNING, "Unable to write CSV DATA.\r\nExiting now...");
	DoExit(D_ERROR);
	return FALSE;
}

BOOL PrintSTDOUTData(_In_ PDENIED_FILE_DATA pDeniedData)
{
	PTCHAR tData = NULL;
	PTCHAR tFileType = NULL;
	
	if (!pDeniedData || !(pDeniedData->tFilePath))
	{
		DEBUG_LOG(D_WARNING, "PDENIED_FILE_DATA invalid for  file.\r\n");
		DoExit(D_WARNING);
	}

	if (pDeniedData->bIsADirectory)
		tFileType = TEXT("directory");
	else
		tFileType = TEXT("file");

	printf("[DENIED] File=%ws Type=%ws\r\n", pDeniedData->tFilePath, tFileType);

	return TRUE;
}