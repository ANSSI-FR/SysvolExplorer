/**************************************************
* SysvolCrawler - GPEiniPrinter.cpp
* AUTHOR: Luc Delsalle	
*
* Display or export GPE.ini data
*
* ANSSI/COSSI/DTO/BAI - 2014
***************************************************/

#include "GPEiniPrinter.h"

BOOL PrintData(_In_ PGPEINI_FILE_DATA pGpeIniData)
{
	BOOL bRes = TRUE;

	if (pGpeIniData == NULL)
	{
		DEBUG_LOG(D_ERROR, "pGpeIniData pointer invalid.\r\nExiting now...");
		DoExit(D_ERROR);
	}

	// Call every printer
	if ((bRes) && (pSyscrwlrOptions->bShouldPrintXML))
		bRes = PrintXMLData(pGpeIniData);

	if ((bRes) && (pSyscrwlrOptions->bShouldPrintCSV))
		bRes = PrintCSVData(pGpeIniData);

	if ((bRes) && (pSyscrwlrOptions->bShouldPrintSTDOUT))
		bRes = PrintSTDOUTData(pGpeIniData);

	return bRes;
}

BOOL PrintGpeIniDataHeader(_In_ PTCHAR tFilePath)
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
		HANDLE hXMLFile = GetFileHandle(OUTPUT_FILE_XML, OUTPUT_DIRECTORY_GPE_INI, OUTPUT_NAME_GPE_INI);

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
			|| (WriteFile(hXMLFile, OUTPUT_NAME_GPE_INI, (DWORD)(_tcslen(OUTPUT_NAME_GPE_INI) * sizeof (TCHAR)), &dwDataRead, NULL) == FALSE)
			|| (WriteFile(hXMLFile, TEXT(".xml>\r\n"), (DWORD)(_tcslen(TEXT(".xml>\r\n")) * sizeof (TCHAR)), &dwDataRead, NULL) == FALSE))
				goto writerror;
		}

		if ((WriteFile(hXMLFile, TEXT("<GPEiniFile Filename=\""), (DWORD)(_tcslen(TEXT("<GPEiniFile Filename=\"")) * sizeof (TCHAR)), &dwDataRead, NULL) == FALSE)
		|| (WriteFile(hXMLFile, tFilePath, (DWORD)(_tcslen(tFilePath) * sizeof (TCHAR)), &dwDataRead, NULL) == FALSE)
		|| (WriteFile(hXMLFile, TEXT("\">\r\n"), (DWORD)(_tcslen(TEXT("\">\r\n")) * sizeof (TCHAR)), &dwDataRead, NULL) == FALSE))
			goto writerror;

		CloseHandle(hXMLFile);
	}

	if (pSyscrwlrOptions->bShouldPrintCSV)
	{
		HANDLE hCSVFile = GetFileHandle(OUTPUT_FILE_CSV, OUTPUT_DIRECTORY_GPE_INI, OUTPUT_NAME_GPE_INI);
		LARGE_INTEGER liFileSize;

		if (!GetFileSizeEx(hCSVFile, &liFileSize))
		{
			DEBUG_LOG(D_WARNING, "Unable to determine file size.\r\nExiting now...");
			DoExit(D_ERROR);
		}

		if ((liFileSize.HighPart == 0) && (liFileSize.LowPart == 0))
		{
			if (WriteFile(hCSVFile, TEXT("File;MachineExtension;UserExtension;Unreferrenced\r\n"), (DWORD)(_tcslen(TEXT("File;MachineExtension;UserExtension;Unreferrenced\r\n")) * sizeof (TCHAR)), &dwDataRead, NULL) == FALSE)
				goto writerror;
		}
		CloseHandle(hCSVFile);
	}
	return TRUE;

writerror:
	DEBUG_LOG(D_WARNING, "Unable to write DATA HEADER for GPE.ini printer.\r\nExiting now...");
	DoExit(D_ERROR);
	return FALSE;
}

BOOL PrintGpeIniDataFooter(_In_ PTCHAR tFilePath)
{
	DWORD dwDataRead = 0;

	// Hack for closing xml document. Ugly.
	if (pSyscrwlrOptions->bShouldPrintXML)
	{
		HANDLE hXMLFile = GetFileHandle(OUTPUT_FILE_XML, OUTPUT_DIRECTORY_GPE_INI, OUTPUT_NAME_GPE_INI);
		if (WriteFile(hXMLFile, TEXT("</GPEiniFile>\r\n"), (DWORD)(_tcslen(TEXT("</GPEiniFile>\r\n")) * sizeof (TCHAR)), &dwDataRead, NULL) == FALSE)
			goto writerror;
		CloseHandle(hXMLFile);
	}
	return TRUE;

writerror:
	DEBUG_LOG(D_WARNING, "Unable to write DATA FOOTER for GPE.ini printer.\r\nExiting now...");
	DoExit(D_ERROR);
	return FALSE;
}

BOOL PrintXMLData(_In_ PGPEINI_FILE_DATA pGpeIniData)
{
	DWORD dwDataRead = 0;
	HANDLE hXMLFile = GetFileHandle(OUTPUT_FILE_XML, OUTPUT_DIRECTORY_GPE_INI, OUTPUT_NAME_GPE_INI);
	TCHAR tMachineExtensionNum[11], tUserExtensionNum[11];
	DWORD dwMachineExtensionNumLen = 0, dwUserExtensionNumLen = 0;

	if (!pGpeIniData || !(pGpeIniData->tFilePath))
	{
		DEBUG_LOG(D_WARNING, "PGPEINI_FILE_DATA invalid for GPE.ini file.\r\n");
		DoExit(D_WARNING);
	}

	if (hXMLFile == INVALID_HANDLE_VALUE)
	{
		DEBUG_LOG(D_WARNING, "Handle to hXMLFile is invalid.\r\nExiting now...");
		DoExit(D_ERROR);
	}

	dwMachineExtensionNumLen = _stprintf_s(tMachineExtensionNum, 11, TEXT("%d"), pGpeIniData->dwMachineExtensionVersionsNum);
	dwUserExtensionNumLen = _stprintf_s(tUserExtensionNum, 11, TEXT("%d"), pGpeIniData->dwUserExtensionVersionsNum);

	if ((WriteFile(hXMLFile, TEXT("\t<MachineExtension numberofcse=\""), (DWORD)(_tcslen(TEXT("\t<MachineExtension numberofcse=\"")) * sizeof (TCHAR)), &dwDataRead, NULL) == FALSE)
		|| (WriteFile(hXMLFile, tMachineExtensionNum, (DWORD)(dwMachineExtensionNumLen * sizeof (WCHAR)), &dwDataRead, NULL) == FALSE)
		|| (WriteFile(hXMLFile, TEXT("\">\r\n"), (DWORD)(_tcslen(TEXT("\">\r\n")) * sizeof (TCHAR)), &dwDataRead, NULL) == FALSE))
		goto writerror;
	for (DWORD i = 0; i < pGpeIniData->dwMachineExtensionVersionsNum; ++i)
	{
		PGPEINI_CSE_DATA pCurrentCseData = pGpeIniData->pMachineExtensionVersions[i];
		TCHAR tId[11], tActionNum[11];
		DWORD dwIdLen = 0, dwActionNumLen = 0;

		dwIdLen = _stprintf_s(tId, 11, TEXT("%d"), pCurrentCseData->dwCSEID);
		dwActionNumLen = _stprintf_s(tActionNum, 11, TEXT("%d"), pCurrentCseData->dwCSEValuesNum);

		if ((WriteFile(hXMLFile, TEXT("\t\t<Cse numberofaction=\""), (DWORD)(_tcslen(TEXT("\t\t<Cse numberofaction=\"")) * sizeof (TCHAR)), &dwDataRead, NULL) == FALSE)
			|| (WriteFile(hXMLFile, tActionNum, (DWORD)(dwActionNumLen * sizeof (WCHAR)), &dwDataRead, NULL) == FALSE)
			|| (WriteFile(hXMLFile, TEXT("\" Id=\""), (DWORD)(_tcslen(TEXT("\" Id=\"")) * sizeof (TCHAR)), &dwDataRead, NULL) == FALSE)
			|| (WriteFile(hXMLFile, tId, (DWORD)(dwIdLen * sizeof (WCHAR)), &dwDataRead, NULL) == FALSE)
			|| (WriteFile(hXMLFile, TEXT("\">\r\n"), (DWORD)(_tcslen(TEXT("\">\r\n")) * sizeof (TCHAR)), &dwDataRead, NULL) == FALSE))
			goto writerror;
		for (DWORD j = 0; j < pCurrentCseData->dwCSEValuesNum; ++j)
		{

			if ((WriteFile(hXMLFile, TEXT("\t\t\t<CseAction Guid=\""), (DWORD)(_tcslen(TEXT("\t\t\t<CseAction Guid=\"")) * sizeof (TCHAR)), &dwDataRead, NULL) == FALSE)
				|| (WriteFile(hXMLFile, pCurrentCseData->pCSEValues[j], (DWORD)(_tcslen(pCurrentCseData->pCSEValues[j]) * sizeof (WCHAR)), &dwDataRead, NULL) == FALSE)
				|| (WriteFile(hXMLFile, TEXT("\">\r\n"), (DWORD)(_tcslen(TEXT("\">\r\n")) * sizeof (TCHAR)), &dwDataRead, NULL) == FALSE))
				goto writerror;
		}
		if (WriteFile(hXMLFile, TEXT("\t\t<Cse/>\r\n"), (DWORD)(_tcslen(TEXT("\t\t<Cse/>\r\n")) * sizeof (TCHAR)), &dwDataRead, NULL) == FALSE)
			goto writerror;
	}
	if (WriteFile(hXMLFile, TEXT("\t<MachineExtension/>\r\n"), (DWORD)(_tcslen(TEXT("\t<MachineExtension/>\r\n")) * sizeof (TCHAR)), &dwDataRead, NULL) == FALSE)
		goto writerror;

	if ((WriteFile(hXMLFile, TEXT("\t<UserExtension numberofcse=\""), (DWORD)(_tcslen(TEXT("\t<UserExtension numberofcse=\"")) * sizeof (TCHAR)), &dwDataRead, NULL) == FALSE)
		|| (WriteFile(hXMLFile, tUserExtensionNum, (DWORD)(dwUserExtensionNumLen * sizeof (WCHAR)), &dwDataRead, NULL) == FALSE)
		|| (WriteFile(hXMLFile, TEXT("\">\r\n"), (DWORD)(_tcslen(TEXT("\">\r\n")) * sizeof (TCHAR)), &dwDataRead, NULL) == FALSE))
		goto writerror;
	for (DWORD i = 0; i < pGpeIniData->dwUserExtensionVersionsNum; ++i)
	{
		PGPEINI_CSE_DATA pCurrentCseData = pGpeIniData->pUserExtensionVersions[i];
		TCHAR tId[11], tActionNum[11];
		DWORD dwIdLen = 0, dwActionNumLen = 0;

		dwIdLen = _stprintf_s(tId, 11, TEXT("%d"), pCurrentCseData->dwCSEID);
		dwActionNumLen = _stprintf_s(tActionNum, 11, TEXT("%d"), pCurrentCseData->dwCSEValuesNum);

		if ((WriteFile(hXMLFile, TEXT("\t\t<Cse numberofaction=\""), (DWORD)(_tcslen(TEXT("\t\t<Cse numberofaction=\"")) * sizeof (TCHAR)), &dwDataRead, NULL) == FALSE)
			|| (WriteFile(hXMLFile, tActionNum, (DWORD)(dwActionNumLen * sizeof (WCHAR)), &dwDataRead, NULL) == FALSE)
			|| (WriteFile(hXMLFile, TEXT("\" Id=\""), (DWORD)(_tcslen(TEXT("\" Id=\"")) * sizeof (TCHAR)), &dwDataRead, NULL) == FALSE)
			|| (WriteFile(hXMLFile, tId, (DWORD)(dwIdLen * sizeof (WCHAR)), &dwDataRead, NULL) == FALSE)
			|| (WriteFile(hXMLFile, TEXT("\">\r\n"), (DWORD)(_tcslen(TEXT("\">\r\n")) * sizeof (TCHAR)), &dwDataRead, NULL) == FALSE))
			goto writerror;
		for (DWORD j = 0; j < pCurrentCseData->dwCSEValuesNum; ++j)
		{

			if ((WriteFile(hXMLFile, TEXT("\t\t\t<CseAction Guid=\""), (DWORD)(_tcslen(TEXT("\t\t\t<CseAction Guid=\"")) * sizeof (TCHAR)), &dwDataRead, NULL) == FALSE)
				|| (WriteFile(hXMLFile, pCurrentCseData->pCSEValues[j], (DWORD)(_tcslen(pCurrentCseData->pCSEValues[j]) * sizeof (WCHAR)), &dwDataRead, NULL) == FALSE)
				|| (WriteFile(hXMLFile, TEXT("\">\r\n"), (DWORD)(_tcslen(TEXT("\">\r\n")) * sizeof (TCHAR)), &dwDataRead, NULL) == FALSE))
				goto writerror;
		}
		if (WriteFile(hXMLFile, TEXT("\t\t<Cse/>\r\n"), (DWORD)(_tcslen(TEXT("\t<Cse/>\r\n")) * sizeof (TCHAR)), &dwDataRead, NULL) == FALSE)
			goto writerror;
	}
	if (WriteFile(hXMLFile, TEXT("\t<UserExtension/>\r\n"), (DWORD)(_tcslen(TEXT("\t<UserExtension/>\r\n")) * sizeof (TCHAR)), &dwDataRead, NULL) == FALSE)
		goto writerror;

	for (DWORD i = 0; i < pGpeIniData->dwNumberOfUnReferrencedSections; ++i)
	{
		PINI_SECTION_DATA pCurrSectionData = pGpeIniData->pUnReferrencedSections[i];

		if (!pCurrSectionData->tSectionName)
			continue;

		PrintXMLUnreferencedSectionDataInGPE(pCurrSectionData, hXMLFile);
	}	

	CloseHandle(hXMLFile);
	return TRUE;
writerror:
	DEBUG_LOG(D_WARNING, "Unable to write XML DATA.\r\nExiting now...");
	DoExit(D_ERROR);
	return FALSE;
}

BOOL PrintXMLUnreferencedSectionDataInGPE(_In_ PINI_SECTION_DATA pSectionData, _In_ HANDLE hXMLFile)
{
	DWORD dwDataRead = 0;

	if (!pSectionData || !(pSectionData->tSectionName))
	{
		DEBUG_LOG(D_WARNING, "PINI_SECTION_DATA invalid for GPE.ini file.\r\n");
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

BOOL PrintCSVData(_In_ PGPEINI_FILE_DATA pGpeIniData)
{
	DWORD dwDataRead = 0;
	HANDLE hCSVFile = GetFileHandle(OUTPUT_FILE_CSV, OUTPUT_DIRECTORY_GPE_INI, OUTPUT_NAME_GPE_INI);

	if (!pGpeIniData || !(pGpeIniData->tFilePath))
	{
		DEBUG_LOG(D_WARNING, "PGPEINI_FILE_DATA invalid for CSV file.\r\n");
		DoExit(D_WARNING);
	}

	if (hCSVFile == INVALID_HANDLE_VALUE)
	{
		DEBUG_LOG(D_WARNING, "Handle to hCSVFile is invalid.\r\nExiting now...");
		DoExit(D_ERROR);
	}

	if ((WriteFile(hCSVFile, pGpeIniData->tFilePath, (DWORD)(_tcslen(pGpeIniData->tFilePath) * sizeof (WCHAR)), &dwDataRead, NULL) == FALSE)
		|| (WriteFile(hCSVFile, TEXT(";"), (DWORD)(_tcslen(TEXT(";")) * sizeof (TCHAR)), &dwDataRead, NULL) == FALSE))
		goto writerror;	

	for (DWORD i = 0; i < pGpeIniData->dwMachineExtensionVersionsNum; ++i)
	{
		PGPEINI_CSE_DATA pCurrentCseData = pGpeIniData->pMachineExtensionVersions[i];
		TCHAR tId[11];
		DWORD dwIdLen = 0;

		dwIdLen = _stprintf_s(tId, 11, TEXT("%d"), pCurrentCseData->dwCSEID);

		if (WriteFile(hCSVFile, TEXT("(Cse:"), (DWORD)(_tcslen(TEXT("(Cse:")) * sizeof (TCHAR)), &dwDataRead, NULL) == FALSE)
			goto writerror;	
		for (DWORD j = 0; j < pCurrentCseData->dwCSEValuesNum; ++j)
		{
			if (WriteFile(hCSVFile, pCurrentCseData->pCSEValues[j], (DWORD)(_tcslen(pCurrentCseData->pCSEValues[j]) * sizeof (WCHAR)), &dwDataRead, NULL) == FALSE)
				goto writerror;	
		}
		if ((WriteFile(hCSVFile, TEXT(", Id:"), (DWORD)(_tcslen(TEXT(", Id:")) * sizeof (TCHAR)), &dwDataRead, NULL) == FALSE)
			|| (WriteFile(hCSVFile, tId, (DWORD)(dwIdLen * sizeof (WCHAR)), &dwDataRead, NULL) == FALSE)
			|| (WriteFile(hCSVFile, TEXT(")"), (DWORD)(_tcslen(TEXT(")")) * sizeof (TCHAR)), &dwDataRead, NULL) == FALSE))
			goto writerror;	
	}
	if (!pGpeIniData->dwMachineExtensionVersionsNum)
	{
		if (WriteFile(hCSVFile, TEXT("(missing);"), (DWORD)(_tcslen(TEXT("(missing);")) * sizeof (TCHAR)), &dwDataRead, NULL) == FALSE)
			goto writerror;	
	}
	else
	{
		if (WriteFile(hCSVFile, TEXT(";"), (DWORD)(_tcslen(TEXT(";")) * sizeof (TCHAR)), &dwDataRead, NULL) == FALSE)
			goto writerror;	
	}


	for (DWORD i = 0; i < pGpeIniData->dwUserExtensionVersionsNum; ++i)
	{
		PGPEINI_CSE_DATA pCurrentCseData = pGpeIniData->pUserExtensionVersions[i];
		TCHAR tId[11];
		DWORD dwIdLen = 0;

		dwIdLen = _stprintf_s(tId, 11, TEXT("%d"), pCurrentCseData->dwCSEID);

		if (WriteFile(hCSVFile, TEXT("(Cse:"), (DWORD)(_tcslen(TEXT("(Cse:")) * sizeof (TCHAR)), &dwDataRead, NULL) == FALSE)
			goto writerror;	
		for (DWORD j = 0; j < pCurrentCseData->dwCSEValuesNum; ++j)
		{
			if (WriteFile(hCSVFile, pCurrentCseData->pCSEValues[j], (DWORD)(_tcslen(pCurrentCseData->pCSEValues[j]) * sizeof (WCHAR)), &dwDataRead, NULL) == FALSE)
				goto writerror;	
		}
		if ((WriteFile(hCSVFile, TEXT(", Id:"), (DWORD)(_tcslen(TEXT(", Id:")) * sizeof (TCHAR)), &dwDataRead, NULL) == FALSE)
			|| (WriteFile(hCSVFile, tId, (DWORD)(dwIdLen * sizeof (WCHAR)), &dwDataRead, NULL) == FALSE)
			|| (WriteFile(hCSVFile, TEXT(")"), (DWORD)(_tcslen(TEXT(")")) * sizeof (TCHAR)), &dwDataRead, NULL) == FALSE))
			goto writerror;	
	}
	if (!pGpeIniData->dwUserExtensionVersionsNum)
	{
		if (WriteFile(hCSVFile, TEXT("(missing);"), (DWORD)(_tcslen(TEXT("(missing);")) * sizeof (TCHAR)), &dwDataRead, NULL) == FALSE)
			goto writerror;	
	}
	else
	{
		if (WriteFile(hCSVFile, TEXT(";"), (DWORD)(_tcslen(TEXT(";")) * sizeof (TCHAR)), &dwDataRead, NULL) == FALSE)
			goto writerror;	
	}

	for (DWORD i = 0; i < pGpeIniData->dwNumberOfUnReferrencedSections; ++i)
	{
		PINI_SECTION_DATA pCurrSectionData = pGpeIniData->pUnReferrencedSections[i];

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

	if (WriteFile(hCSVFile, TEXT("\r\n"), (DWORD)(_tcslen(TEXT("\r\n")) * sizeof (TCHAR)), &dwDataRead, NULL) == FALSE)
		goto writerror;	

	CloseHandle(hCSVFile);
	return TRUE;
writerror:
	DEBUG_LOG(D_WARNING, "Unable to write CSV DATA.\r\nExiting now...");
	DoExit(D_ERROR);
	return FALSE;
}

BOOL PrintSTDOUTData(_In_ PGPEINI_FILE_DATA pGpeIniData)
{
	DWORD dwDataRead = 0;

	if (!pGpeIniData || !(pGpeIniData->tFilePath))
	{
		DEBUG_LOG(D_WARNING, "PGPEINI_FILE_DATA invalid for GPE.ini file.\r\n");
		DoExit(D_WARNING);
	}
	printf("[GPEini] File=%ws", pGpeIniData->tFilePath);

	printf(" MachineExtension=");
	for (DWORD i = 0; i < pGpeIniData->dwMachineExtensionVersionsNum; ++i)
	{
		PGPEINI_CSE_DATA pCurrentCseData = pGpeIniData->pMachineExtensionVersions[i];
		printf("(Cse:");
		for (DWORD j = 0; j < pCurrentCseData->dwCSEValuesNum; ++j)
			printf("%ws", pCurrentCseData->pCSEValues[j]);
		printf(" Id:%d)", pCurrentCseData->dwCSEID);
	}
	if (!pGpeIniData->dwMachineExtensionVersionsNum)
		printf("(missing)");

	printf(" UserExtension=");
	for (DWORD i = 0; i < pGpeIniData->dwUserExtensionVersionsNum; ++i)
	{
		PGPEINI_CSE_DATA pCurrentCseData = pGpeIniData->pUserExtensionVersions[i];
		printf("(Cse:");
		for (DWORD j = 0; j < pCurrentCseData->dwCSEValuesNum; ++j)
			printf("%ws", pCurrentCseData->pCSEValues[j]);
		printf(" Id:%d)", pCurrentCseData->dwCSEID);
	}
	if (!pGpeIniData->dwUserExtensionVersionsNum)
		printf("(missing)");

	for (DWORD i = 0; i < pGpeIniData->dwNumberOfUnReferrencedSections; ++i)
	{
		PINI_SECTION_DATA pCurrSectionData = pGpeIniData->pUnReferrencedSections[i];

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