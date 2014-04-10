/**************************************************
* SysvolCrawler - POLPrinter.cpp
* AUTHOR: Luc Delsalle	
*
* Display or export POL file content
*	
ANSSI/COSSI/DTO/BAI - 2014
***************************************************/

#include "POLPrinter.h"

BOOL PrintData(_In_ PPOL_DATA pPolData)
{
	BOOL bRes = TRUE;

	if (pPolData == NULL)
	{
		DEBUG_LOG(D_ERROR, "pPolData pointer invalid.\r\nExiting now...");
		DoExit(D_ERROR);
	}

	// Call every printer
	if ((bRes) && (pSyscrwlrOptions->bShouldPrintXML))
		bRes = PrintXMLData(pPolData);

	if ((bRes) && (pSyscrwlrOptions->bShouldPrintCSV))
		bRes = PrintCSVData(pPolData);

	if ((bRes) && (pSyscrwlrOptions->bShouldPrintSTDOUT))
		bRes = PrintSTDOUTData(pPolData);

	return bRes;
}

BOOL PrintPolDataHeader(_In_ PTCHAR tFilePath)
{
	DWORD dwDataRead = 0;
	LARGE_INTEGER liFileSize;
	GPO_FILTER_TARGET dwTarget = GPO_FILTER_UNKNOWN;
	PTCHAR tOutputDirectoryPolFile = OUTPUT_DIRECTORY_POL_FILE;

	if (!tFilePath)
	{
		DEBUG_LOG(D_WARNING, "tFilePath is invalid.\r\nExiting now...");
		DoExit(D_ERROR);
	}

	// Determine if we need to create user or computer file in case of '[Machine||User]' 
	dwTarget = GetTargetGPO(tFilePath);
	if (dwTarget == GPO_FILTER_TARGET_MACHINE)
		tOutputDirectoryPolFile = TEXT(".\\Machine\\");
	else if (dwTarget == GPO_FILTER_TARGET_USER)
		tOutputDirectoryPolFile = TEXT(".\\User\\");

	// Hack for closing xml document. Ugly.
	if (pSyscrwlrOptions->bShouldPrintXML)
	{
		HANDLE hXMLFile = GetFileHandle(OUTPUT_FILE_XML, tOutputDirectoryPolFile, OUTPUT_NAME_POL_FILE);
		LARGE_INTEGER liFileSize;
		
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
			|| (WriteFile(hXMLFile, OUTPUT_NAME_POL_FILE, (DWORD)(_tcslen(OUTPUT_NAME_POL_FILE) * sizeof (TCHAR)), &dwDataRead, NULL) == FALSE)
			|| (WriteFile(hXMLFile, TEXT(".xml>\r\n"), (DWORD)(_tcslen(TEXT(".xml>\r\n")) * sizeof (TCHAR)), &dwDataRead, NULL) == FALSE))
				goto writerror;
		}

		if ((WriteFile(hXMLFile, TEXT("<RegistryPolFile Filename=\""), (DWORD)(_tcslen(TEXT("<RegistryPolFile Filename=\"")) * sizeof (TCHAR)), &dwDataRead, NULL) == FALSE)
		|| (WriteFile(hXMLFile, tFilePath, (DWORD)(_tcslen(tFilePath) * sizeof (TCHAR)), &dwDataRead, NULL) == FALSE)
		|| (WriteFile(hXMLFile, TEXT("\">\r\n"), (DWORD)(_tcslen(TEXT("\">\r\n")) * sizeof (TCHAR)), &dwDataRead, NULL) == FALSE))
			goto writerror;

		CloseHandle(hXMLFile);
	}

	if (pSyscrwlrOptions->bShouldPrintCSV)
	{
		HANDLE hCSVFile = GetFileHandle(OUTPUT_FILE_CSV, tOutputDirectoryPolFile, OUTPUT_NAME_POL_FILE);
	
		if (!GetFileSizeEx(hCSVFile, &liFileSize))
		{
			DEBUG_LOG(D_WARNING, "Unable to determine file size.\r\nExiting now...");
			DoExit(D_ERROR);
		}

		if ((liFileSize.HighPart == 0) && (liFileSize.LowPart == 0))
		{
			if (WriteFile(hCSVFile, TEXT("File;Key;Value;Type;Size;Data\r\n"), (DWORD)(_tcslen(TEXT("File;Key;Value;Type;Size;Data\r\n")) * sizeof (TCHAR)), &dwDataRead, NULL) == FALSE)
				goto writerror;
		}
		CloseHandle(hCSVFile);
	}
	return TRUE;

writerror:
	DEBUG_LOG(D_WARNING, "Unable to write DATA HEADER for POL printer.\r\nExiting now...");
	DoExit(D_ERROR);
	return FALSE;
}

BOOL PrintPolDataFooter(_In_ PTCHAR tFilePath)
{
	DWORD dwDataRead = 0;
	GPO_FILTER_TARGET dwTarget = GPO_FILTER_UNKNOWN;
	PTCHAR tOutputDirectoryPolFile = OUTPUT_DIRECTORY_POL_FILE;

	// Determine if we need to create user or computer file in case of '[Machine||User]' 
	dwTarget = GetTargetGPO(tFilePath);
	if (dwTarget == GPO_FILTER_TARGET_MACHINE)
		tOutputDirectoryPolFile = TEXT(".\\Machine\\");
	else if (dwTarget == GPO_FILTER_TARGET_USER)
		tOutputDirectoryPolFile = TEXT(".\\User\\");

	// Hack for closing xml document. Ugly.
	if (pSyscrwlrOptions->bShouldPrintXML)
	{
		HANDLE hXMLFile = GetFileHandle(OUTPUT_FILE_XML, tOutputDirectoryPolFile, OUTPUT_NAME_POL_FILE);
		if (WriteFile(hXMLFile, TEXT("</RegistryPolFile>\r\n"), (DWORD)(_tcslen(TEXT("</RegistryPolFile>\r\n")) * sizeof (TCHAR)), &dwDataRead, NULL) == FALSE)
			goto writerror;
		CloseHandle(hXMLFile);
	}
	return TRUE;

writerror:
	DEBUG_LOG(D_WARNING, "Unable to write DATA FOOTER for POL printer.\r\nExiting now...");
	DoExit(D_ERROR);
	return FALSE;
}

BOOL PrintXMLData(_In_ PPOL_DATA pPolData)
{
	HANDLE hXMLFile = INVALID_HANDLE_VALUE;
	DWORD dwDataRead = 0, dwSizeLength = 0;
	TCHAR tBuffToWrite = NULL;
	PTCHAR tType = GetTypeFromID(*pPolData->pwdType);
	PTCHAR tValue = GetBase64FromByte(pPolData->pbValue, pPolData->dwValueSize);
	PTCHAR tData = GetBase64FromByte(pPolData->pbData, (*pPolData->pwdSize));
	TCHAR tSize[100];
	GPO_FILTER_TARGET dwTarget = GPO_FILTER_UNKNOWN;
	PTCHAR tOutputDirectoryPolFile = OUTPUT_DIRECTORY_POL_FILE;

	// Determine if we need to create user or computer file in case of '[Machine||User]' 
	dwTarget = GetTargetGPO(pPolData->tFilePath);
	if (dwTarget == GPO_FILTER_TARGET_MACHINE)
		tOutputDirectoryPolFile = TEXT(".\\Machine\\");
	else if (dwTarget == GPO_FILTER_TARGET_USER)
		tOutputDirectoryPolFile = TEXT(".\\User\\");
	hXMLFile = GetFileHandle(OUTPUT_FILE_XML, tOutputDirectoryPolFile, OUTPUT_NAME_POL_FILE);

	if (!tType || !tValue || !tData)
	{
		DEBUG_LOG(D_WARNING, "Type, value or data invalid for POL file.\r\n");
		DoExit(D_WARNING);
	}

	if (hXMLFile == INVALID_HANDLE_VALUE)
	{
		DEBUG_LOG(D_WARNING, "Handle to hXMLFile is invalid.\r\nExiting now...");
		DoExit(D_ERROR);
	}

	if (!pPolData)
	{
		DEBUG_LOG(D_WARNING, "pPolData is invalid.\r\nExiting now...");
		DoExit(D_ERROR);
	}

	RemoveEndline(pPolData->pwKey);
	dwSizeLength = _stprintf_s(tSize, 100, TEXT("%d"), (*pPolData->pwdSize));

	if ((WriteFile(hXMLFile, TEXT("\t<POL Key=\""), (DWORD)(_tcslen(TEXT("\t<POL Key=\"")) * sizeof (TCHAR)), &dwDataRead, NULL) == FALSE)
		|| (WriteFile(hXMLFile, pPolData->pwKey, (DWORD)(_tcslen(pPolData->pwKey) * sizeof (WCHAR)), &dwDataRead, NULL) == FALSE)
		|| (WriteFile(hXMLFile, TEXT("\" value=\""), (DWORD)(_tcslen(TEXT("\" value=\"")) * sizeof (TCHAR)), &dwDataRead, NULL) == FALSE)
		|| (WriteFile(hXMLFile, tValue, (DWORD)(_tcslen(tValue) * sizeof (WCHAR)), &dwDataRead, NULL) == FALSE)
		|| (WriteFile(hXMLFile, TEXT("\" type=\""), (DWORD)(_tcslen(TEXT("\" type=\"")) * sizeof (TCHAR)), &dwDataRead, NULL) == FALSE)
		|| (WriteFile(hXMLFile, tType, (DWORD)(_tcslen(tType) * sizeof (WCHAR)), &dwDataRead, NULL) == FALSE)
		|| (WriteFile(hXMLFile, TEXT("\" size=\""), (DWORD)(_tcslen(TEXT("\" size=\"")) * sizeof (TCHAR)), &dwDataRead, NULL) == FALSE)
		|| (WriteFile(hXMLFile, tSize, ((dwSizeLength) * sizeof (WCHAR)), &dwDataRead, NULL) == FALSE)
		|| (WriteFile(hXMLFile, TEXT("\" data=\""), (DWORD)(_tcslen(TEXT("\" data=\"")) * sizeof (TCHAR)), &dwDataRead, NULL) == FALSE)
		|| (WriteFile(hXMLFile, tData, (DWORD)(_tcslen(tData) * sizeof (WCHAR)), &dwDataRead, NULL) == FALSE)
		|| (WriteFile(hXMLFile, TEXT("\"/>\r\n"), (DWORD)(_tcslen(TEXT("\"/>\r\n")) * sizeof (TCHAR)), &dwDataRead, NULL) == FALSE))
			goto writerror;		

	HeapFree(hCrawlerHeap, NULL, tValue);
	HeapFree(hCrawlerHeap, NULL, tData);
	CloseHandle(hXMLFile);

	return TRUE;


writerror:
	DEBUG_LOG(D_WARNING, "Unable to write XML DATA.\r\nExiting now...");
	DoExit(D_ERROR);
	return FALSE;
}

BOOL PrintCSVData(_In_ PPOL_DATA pPolData)
{
	HANDLE hCSVFile = INVALID_HANDLE_VALUE;
	DWORD dwDataRead = 0, dwSizeLength = 0;
	PTCHAR tType = GetTypeFromID(*pPolData->pwdType);
	PTCHAR tValue = GetBase64FromByte(pPolData->pbValue, pPolData->dwValueSize);
	PTCHAR tData = GetBase64FromByte(pPolData->pbData, (*pPolData->pwdSize));
	TCHAR tSize[100];
	GPO_FILTER_TARGET dwTarget = GPO_FILTER_UNKNOWN;
	PTCHAR tOutputDirectoryPolFile = OUTPUT_DIRECTORY_POL_FILE;

	// Determine if we need to create user or computer file in case of '[Machine||User]' 
	dwTarget = GetTargetGPO(pPolData->tFilePath);
	if (dwTarget == GPO_FILTER_TARGET_MACHINE)
		tOutputDirectoryPolFile = TEXT(".\\Machine\\");
	else if (dwTarget == GPO_FILTER_TARGET_USER)
		tOutputDirectoryPolFile = TEXT(".\\User\\");
	hCSVFile = GetFileHandle(OUTPUT_FILE_CSV, tOutputDirectoryPolFile, OUTPUT_NAME_POL_FILE);

	if (!tType || !tValue || !tData)
	{
		DEBUG_LOG(D_WARNING, "Type, value or data invalid for POL file.\r\n");
		DoExit(D_WARNING);
	}

	RemoveEndline(pPolData->pwKey);
	dwSizeLength = _stprintf_s(tSize, 100, TEXT("%d"), (*pPolData->pwdSize));

	if ((WriteFile(hCSVFile, pPolData->tFilePath, (DWORD)(_tcslen(pPolData->tFilePath) * sizeof (WCHAR)), &dwDataRead, NULL) == FALSE)
		|| (WriteFile(hCSVFile, TEXT(";"), (DWORD)(_tcslen(TEXT(";")) * sizeof (TCHAR)), &dwDataRead, NULL) == FALSE)
		|| (WriteFile(hCSVFile, pPolData->pwKey, (DWORD)(_tcslen(pPolData->pwKey) * sizeof (WCHAR)), &dwDataRead, NULL) == FALSE)
		|| (WriteFile(hCSVFile, TEXT(";"), (DWORD)(_tcslen(TEXT(";")) * sizeof (TCHAR)), &dwDataRead, NULL) == FALSE)
		|| (WriteFile(hCSVFile, tValue, (DWORD)(_tcslen(tValue) * sizeof (WCHAR)), &dwDataRead, NULL) == FALSE)
		|| (WriteFile(hCSVFile, TEXT(";"), (DWORD)(_tcslen(TEXT(";")) * sizeof (TCHAR)), &dwDataRead, NULL) == FALSE)
		|| (WriteFile(hCSVFile, tType, (DWORD)(_tcslen(tType) * sizeof (WCHAR)), &dwDataRead, NULL) == FALSE)
		|| (WriteFile(hCSVFile, TEXT(";"), (DWORD)(_tcslen(TEXT(";")) * sizeof (TCHAR)), &dwDataRead, NULL) == FALSE)
		|| (WriteFile(hCSVFile, tSize, ((dwSizeLength) * sizeof (WCHAR)), &dwDataRead, NULL) == FALSE)
		|| (WriteFile(hCSVFile, TEXT(";"), (DWORD)(_tcslen(TEXT(";")) * sizeof (TCHAR)), &dwDataRead, NULL) == FALSE)
		|| (WriteFile(hCSVFile, tData, (DWORD)(_tcslen(tData) * sizeof (WCHAR)), &dwDataRead, NULL) == FALSE)
		|| (WriteFile(hCSVFile, TEXT("\r\n"), (DWORD)(_tcslen(TEXT("\r\n")) * sizeof (TCHAR)), &dwDataRead, NULL) == FALSE))
			goto writerror;		

	HeapFree(hCrawlerHeap, NULL, tValue);
	HeapFree(hCrawlerHeap, NULL, tData);
	CloseHandle(hCSVFile);

	return TRUE;

writerror:
	DEBUG_LOG(D_WARNING, "Unable to write CSV DATA.\r\nExiting now...");
	DoExit(D_ERROR);
	return FALSE;
}

BOOL PrintSTDOUTData(_In_ PPOL_DATA pPolData)
{
	PTCHAR tType = GetTypeFromID(*pPolData->pwdType);
	PTCHAR tValue = GetBase64FromByte(pPolData->pbValue, pPolData->dwValueSize);
	PTCHAR tData = GetBase64FromByte(pPolData->pbData, (*pPolData->pwdSize));
	
	if (!tType || !tValue || !tData)
	{
		DEBUG_LOG(D_WARNING, "Type, value or data invalid for POL file.\r\n");
		DoExit(D_WARNING);
	}

	printf("[POL] File=%ws Key=%ws Value=%ws Type=%ws Size=%d Data=%ws\r\n", pPolData->tFilePath, pPolData->pwKey, tValue, tType, (*pPolData->pwdSize), tData);

	HeapFree(hCrawlerHeap, NULL, tValue);
	HeapFree(hCrawlerHeap, NULL, tData);
	return TRUE;
}

PTCHAR GetTypeFromID(_In_ DWORD dwPolType)
{
	PTCHAR tRes = NULL;

	switch(dwPolType)
	{
	case REG_NONE:
		tRes = TEXT("REG_NONE");
		break;
	case REG_SZ:
		tRes = TEXT("REG_SZ");
		break;
	case REG_EXPAND_SZ:
		tRes = TEXT("REG_EXPAND_SZ");
		break;
	case REG_BINARY:
		tRes = TEXT("REG_BINARY");
		break;
	case REG_DWORD:
		tRes = TEXT("REG_DWORD_LITTLE_ENDIAN");
		break;
		break;
	case REG_DWORD_BIG_ENDIAN:
		tRes = TEXT("REG_DWORD_BIG_ENDIAN");
		break;
	case REG_LINK:
		tRes = TEXT("REG_LINK");
		break;
	case REG_MULTI_SZ:
		tRes = TEXT("REG_MULTI_SZ");
		break;
	case REG_QWORD_LITTLE_ENDIAN:
		tRes = TEXT("REG_QWORD_LITTLE_ENDIAN");
		break;
	}
	return tRes;
}

BOOL RemoveEndline(_In_ PTCHAR tString)
{
	DWORD dwStringLen = 0;

	if (!tString)
		return FALSE;

	dwStringLen = (DWORD) _tcslen(tString);
	for (UINT i = 0; i < dwStringLen; ++i)
	{
		TCHAR currentValue = tString[i];

		if (tString[i] == '\x0d')
			tString[i] = TEXT(' ');
		else if (tString[i] == '\x0a')
			tString[i] = TEXT(' ');
	}
	return TRUE;
}