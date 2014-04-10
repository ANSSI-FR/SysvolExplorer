/**************************************************
* SysvolCrawler - SCRIPTSiniPrinter.cpp
* AUTHOR: Luc Delsalle	
*
* Display or export scripts.ini file content
*
* ANSSI/COSSI/DTO/BAI - 2014
***************************************************/

#include "SCRIPTSiniPrinter.h"

BOOL PrintData(_In_ PSCRIPTSINI_FILE_DATA pScriptsIniData)
{
	BOOL bRes = TRUE;

	if (pScriptsIniData == NULL)
	{
		DEBUG_LOG(D_ERROR, "SCRIPTSINI_FILE_DATA pointer invalid.\r\nExiting now...");
		DoExit(D_ERROR);
	}

	// Call every printer
	if ((bRes) && (pSyscrwlrOptions->bShouldPrintXML))
		bRes = PrintXMLData(pScriptsIniData);

	if ((bRes) && (pSyscrwlrOptions->bShouldPrintCSV))
		bRes = PrintCSVData(pScriptsIniData);

	if ((bRes) && (pSyscrwlrOptions->bShouldPrintSTDOUT))
		bRes = PrintSTDOUTData(pScriptsIniData);

	return bRes;
}

BOOL PrintScriptsIniDataHeader(_In_ PTCHAR tFilePath)
{
	DWORD dwDataRead = 0;
	LARGE_INTEGER liFileSize;
	GPO_FILTER_TARGET dwTarget = GPO_FILTER_UNKNOWN;
	PTCHAR tOutputDirectoryScriptFile = OUTPUT_DIRECTORY_SCRIPTS_INI;

	if (!tFilePath)
	{
		DEBUG_LOG(D_WARNING, "tFilePath is invalid.\r\nExiting now...");
		DoExit(D_ERROR);
	}

	// Determine if we need to create user or computer file in case of '[Machine||User]' 
	dwTarget = GetTargetGPO(tFilePath);
	if (dwTarget == GPO_FILTER_TARGET_MACHINE)
		tOutputDirectoryScriptFile = TEXT(".\\Machine\\");
	else if (dwTarget == GPO_FILTER_TARGET_USER)
		tOutputDirectoryScriptFile = TEXT(".\\User\\");

	// Hack for closing xml document. Ugly.
	if (pSyscrwlrOptions->bShouldPrintXML)
	{
		HANDLE hXMLFile = GetFileHandle(OUTPUT_FILE_XML, tOutputDirectoryScriptFile, OUTPUT_NAME_SCRIPTS_INI);

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
				|| (WriteFile(hXMLFile, OUTPUT_NAME_SCRIPTS_INI, (DWORD)(_tcslen(OUTPUT_NAME_SCRIPTS_INI) * sizeof (TCHAR)), &dwDataRead, NULL) == FALSE)
				|| (WriteFile(hXMLFile, TEXT(".xml>\r\n"), (DWORD)(_tcslen(TEXT(".xml>\r\n")) * sizeof (TCHAR)), &dwDataRead, NULL) == FALSE))
				goto writerror;
		}

		if ((WriteFile(hXMLFile, TEXT("<SCRIPTSiniFiles Filename=\""), (DWORD)(_tcslen(TEXT("<SCRIPTSiniFiles Filename=\"")) * sizeof (TCHAR)), &dwDataRead, NULL) == FALSE)
			|| (WriteFile(hXMLFile, tFilePath, (DWORD)(_tcslen(tFilePath) * sizeof (TCHAR)), &dwDataRead, NULL) == FALSE)
			|| (WriteFile(hXMLFile, TEXT("\">\r\n"), (DWORD)(_tcslen(TEXT("\">\r\n")) * sizeof (TCHAR)), &dwDataRead, NULL) == FALSE))
			goto writerror;

		CloseHandle(hXMLFile);
	}

	if (pSyscrwlrOptions->bShouldPrintCSV)
	{
		HANDLE hCSVFile = GetFileHandle(OUTPUT_FILE_CSV, tOutputDirectoryScriptFile, OUTPUT_NAME_SCRIPTS_INI);
		LARGE_INTEGER liFileSize;

		if (!GetFileSizeEx(hCSVFile, &liFileSize))
		{
			DEBUG_LOG(D_WARNING, "Unable to determine file size.\r\nExiting now...");
			DoExit(D_ERROR);
		}

		if ((liFileSize.HighPart == 0) && (liFileSize.LowPart == 0))
		{
			if (WriteFile(hCSVFile, TEXT("File;Logon;Logoff;Startup;Shutdown;Unreferrenced\r\n"), (DWORD)(_tcslen(TEXT("File;Logon;Logoff;Startup;Shutdown;Unreferrenced\r\n")) * sizeof (TCHAR)), &dwDataRead, NULL) == FALSE)
				goto writerror;
		}
		CloseHandle(hCSVFile);
	}
	return TRUE;

writerror:
	DEBUG_LOG(D_WARNING, "Unable to write DATA HEADER for SCRIPTSini printer.\r\nExiting now...");
	DoExit(D_ERROR);
	return FALSE;
}

BOOL PrintScriptsIniDataFooter(_In_ PTCHAR tFilePath)
{
	DWORD dwDataRead = 0;
	GPO_FILTER_TARGET dwTarget = GPO_FILTER_UNKNOWN;
	PTCHAR tOutputDirectoryScriptFile = OUTPUT_DIRECTORY_SCRIPTS_INI;

	// Determine if we need to create user or computer file in case of '[Machine||User]' 
	dwTarget = GetTargetGPO(tFilePath);
	if (dwTarget == GPO_FILTER_TARGET_MACHINE)
		tOutputDirectoryScriptFile = TEXT(".\\Machine\\");
	else if (dwTarget == GPO_FILTER_TARGET_USER)
		tOutputDirectoryScriptFile = TEXT(".\\User\\");

	// Hack for closing xml document. Ugly.
	if (pSyscrwlrOptions->bShouldPrintXML)
	{
		HANDLE hXMLFile = GetFileHandle(OUTPUT_FILE_XML, tOutputDirectoryScriptFile, OUTPUT_NAME_SCRIPTS_INI);
		if (WriteFile(hXMLFile, TEXT("</SCRIPTSiniFiles>\r\n"), (DWORD)(_tcslen(TEXT("</SCRIPTSiniFiles>\r\n")) * sizeof (TCHAR)), &dwDataRead, NULL) == FALSE)
			goto writerror;
		CloseHandle(hXMLFile);
	}
	return TRUE;

writerror:
	DEBUG_LOG(D_WARNING, "Unable to write DATA FOOTER for SCRIPTSini printer.\r\nExiting now...");
	DoExit(D_ERROR);
	return FALSE;
}

BOOL PrintXMLData(_In_ PSCRIPTSINI_FILE_DATA pScriptsIniData)
{
	DWORD dwDataRead = 0;
	HANDLE hXMLFile = INVALID_HANDLE_VALUE;
	TCHAR tLogon[11], tLogoff[11], tStartup[11], tShutdown[11], tUnreferenced[11];
	DWORD dwLogonLen = 0, dwLogoffLen = 0, dwStartupLen = 0, dwShutdownLen = 0, dwUnreferencedLen = 0;
	GPO_FILTER_TARGET dwTarget = GPO_FILTER_UNKNOWN;
	PTCHAR tOutputDirectoryScriptsFile = OUTPUT_DIRECTORY_SCRIPTS_INI;

	// Determine if we need to create user or computer file in case of '[Machine||User]' 
	dwTarget = GetTargetGPO(pScriptsIniData->tFilePath);
	if (dwTarget == GPO_FILTER_TARGET_MACHINE)
		tOutputDirectoryScriptsFile = TEXT(".\\Machine\\");
	else if (dwTarget == GPO_FILTER_TARGET_USER)
		tOutputDirectoryScriptsFile = TEXT(".\\User\\");	
	hXMLFile = GetFileHandle(OUTPUT_FILE_XML, tOutputDirectoryScriptsFile, OUTPUT_NAME_SCRIPTS_INI);

	if (!pScriptsIniData || !(pScriptsIniData->tFilePath))
	{
		DEBUG_LOG(D_WARNING, "PSCRIPTSINI_FILE_DATA invalid for scripts.ini file.\r\n");
		DoExit(D_WARNING);
	}

	if (hXMLFile == INVALID_HANDLE_VALUE)
	{
		DEBUG_LOG(D_WARNING, "Handle to hXMLFile is invalid.\r\nExiting now...");
		DoExit(D_ERROR);
	}

	dwLogonLen = _stprintf_s(tLogon, 11, TEXT("%d"), pScriptsIniData->dwLogonScriptNum);
	dwLogoffLen = _stprintf_s(tLogoff, 11, TEXT("%d"), pScriptsIniData->dwLogoffScriptNum);
	dwStartupLen = _stprintf_s(tStartup, 11, TEXT("%d"), pScriptsIniData->dwStartupScriptNum);
	dwShutdownLen = _stprintf_s(tShutdown, 11, TEXT("%d"), pScriptsIniData->dwShutdownScriptNum);
	dwUnreferencedLen = _stprintf_s(tUnreferenced, 11, TEXT("%d"), pScriptsIniData->dwNumberOfUnReferrencedSections);

	if ((WriteFile(hXMLFile, TEXT("\t<Logon numberofaction=\""), (DWORD)(_tcslen(TEXT("\t<Logon numberofaction=\"")) * sizeof (TCHAR)), &dwDataRead, NULL) == FALSE)
		|| (WriteFile(hXMLFile, tLogon, (DWORD)(dwLogonLen * sizeof (WCHAR)), &dwDataRead, NULL) == FALSE)
		|| (WriteFile(hXMLFile, TEXT("\">\r\n"), (DWORD)(_tcslen(TEXT("\">\r\n")) * sizeof (TCHAR)), &dwDataRead, NULL) == FALSE))
		goto writerror;
	for (DWORD i = 0; i < pScriptsIniData->dwLogonScriptNum; ++i)
	{
		PTCHAR tCmdLine = EscapeXMLString(pScriptsIniData->pLogonScripts[i]->tCmdLine);
		PTCHAR tParams = EscapeXMLString(pScriptsIniData->pLogonScripts[i]->tParameters);

		if ((WriteFile(hXMLFile, TEXT("\t\t<Action cmdline=\""), (DWORD)(_tcslen(TEXT("\t\t<Action cmdline=\"")) * sizeof (TCHAR)), &dwDataRead, NULL) == FALSE)
			|| (WriteFile(hXMLFile, tCmdLine, (DWORD)(_tcslen(tCmdLine) * sizeof (WCHAR)), &dwDataRead, NULL) == FALSE)
			|| (WriteFile(hXMLFile, TEXT("\" parameters=\""), (DWORD)(_tcslen(TEXT("\" parameters=\"")) * sizeof (TCHAR)), &dwDataRead, NULL) == FALSE)
			|| (WriteFile(hXMLFile, tParams, (DWORD)(_tcslen(tParams) * sizeof (WCHAR)), &dwDataRead, NULL) == FALSE)
			|| (WriteFile(hXMLFile, TEXT("\"/>\r\n"), (DWORD)(_tcslen(TEXT("\"/>\r\n")) * sizeof (TCHAR)), &dwDataRead, NULL) == FALSE))
			goto writerror;

		if (tCmdLine)
			HeapFree(hCrawlerHeap, NULL, tCmdLine);
		if (tParams)
			HeapFree(hCrawlerHeap, NULL, tParams);
	}
	if (WriteFile(hXMLFile, TEXT("\t<Logon/>\r\n"), (DWORD)(_tcslen(TEXT("\t<Logon/>\r\n")) * sizeof (TCHAR)), &dwDataRead, NULL) == FALSE)
		goto writerror;

	if ((WriteFile(hXMLFile, TEXT("\t<Logoff numberofaction=\""), (DWORD)(_tcslen(TEXT("\t<Logoff numberofaction=\"")) * sizeof (TCHAR)), &dwDataRead, NULL) == FALSE)
		|| (WriteFile(hXMLFile, tLogoff, (DWORD)(dwLogoffLen * sizeof (WCHAR)), &dwDataRead, NULL) == FALSE)
		|| (WriteFile(hXMLFile, TEXT("\">\r\n"), (DWORD)(_tcslen(TEXT("\">\r\n")) * sizeof (TCHAR)), &dwDataRead, NULL) == FALSE))
		goto writerror;
	for (DWORD i = 0; i < pScriptsIniData->dwLogoffScriptNum; ++i)
	{
		PTCHAR tCmdLine = EscapeXMLString(pScriptsIniData->pLogoffScripts[i]->tCmdLine);
		PTCHAR tParams = EscapeXMLString(pScriptsIniData->pLogoffScripts[i]->tParameters);

		if ((WriteFile(hXMLFile, TEXT("\t\t<Action cmdline=\""), (DWORD)(_tcslen(TEXT("\t\t<Action cmdline=\"")) * sizeof (TCHAR)), &dwDataRead, NULL) == FALSE)
			|| (WriteFile(hXMLFile, tCmdLine, (DWORD)(_tcslen(tCmdLine) * sizeof (WCHAR)), &dwDataRead, NULL) == FALSE)
			|| (WriteFile(hXMLFile, TEXT("\" parameters=\""), (DWORD)(_tcslen(TEXT("\" parameters=\"")) * sizeof (TCHAR)), &dwDataRead, NULL) == FALSE)
			|| (WriteFile(hXMLFile, tParams, (DWORD)(_tcslen(tParams) * sizeof (WCHAR)), &dwDataRead, NULL) == FALSE)
			|| (WriteFile(hXMLFile, TEXT("\"/>\r\n"), (DWORD)(_tcslen(TEXT("\"/>\r\n")) * sizeof (TCHAR)), &dwDataRead, NULL) == FALSE))
			goto writerror;
		if (tCmdLine)
			HeapFree(hCrawlerHeap, NULL, tCmdLine);
		if (tParams)
			HeapFree(hCrawlerHeap, NULL, tParams);
	}
	if (WriteFile(hXMLFile, TEXT("\t<Logoff/>\r\n"), (DWORD)(_tcslen(TEXT("\t<Logoff/>\r\n")) * sizeof (TCHAR)), &dwDataRead, NULL) == FALSE)
		goto writerror;

	if ((WriteFile(hXMLFile, TEXT("\t<Startup numberofaction=\""), (DWORD)(_tcslen(TEXT("\t<Startup numberofaction=\"")) * sizeof (TCHAR)), &dwDataRead, NULL) == FALSE)
		|| (WriteFile(hXMLFile, tStartup, (DWORD)(dwStartupLen * sizeof (WCHAR)), &dwDataRead, NULL) == FALSE)
		|| (WriteFile(hXMLFile, TEXT("\">\r\n"), (DWORD)(_tcslen(TEXT("\">\r\n")) * sizeof (TCHAR)), &dwDataRead, NULL) == FALSE))
		goto writerror;
	for (DWORD i = 0; i < pScriptsIniData->dwStartupScriptNum; ++i)
	{
		PTCHAR tCmdLine = EscapeXMLString(pScriptsIniData->pStartupScripts[i]->tCmdLine);
		PTCHAR tParams = EscapeXMLString(pScriptsIniData->pStartupScripts[i]->tParameters);

		if ((WriteFile(hXMLFile, TEXT("\t\t<Action cmdline=\""), (DWORD)(_tcslen(TEXT("\t\t<Action cmdline=\"")) * sizeof (TCHAR)), &dwDataRead, NULL) == FALSE)
			|| (WriteFile(hXMLFile, tCmdLine, (DWORD)(_tcslen(tCmdLine) * sizeof (WCHAR)), &dwDataRead, NULL) == FALSE)
			|| (WriteFile(hXMLFile, TEXT("\" parameters=\""), (DWORD)(_tcslen(TEXT("\" parameters=\"")) * sizeof (TCHAR)), &dwDataRead, NULL) == FALSE)
			|| (WriteFile(hXMLFile, tParams, (DWORD)(_tcslen(tParams) * sizeof (WCHAR)), &dwDataRead, NULL) == FALSE)
			|| (WriteFile(hXMLFile, TEXT("\"/>\r\n"), (DWORD)(_tcslen(TEXT("\"/>\r\n")) * sizeof (TCHAR)), &dwDataRead, NULL) == FALSE))
			goto writerror;
		if (tCmdLine)
			HeapFree(hCrawlerHeap, NULL, tCmdLine);
		if (tParams)
			HeapFree(hCrawlerHeap, NULL, tParams);
	}
	if (WriteFile(hXMLFile, TEXT("\t<Startup/>\r\n"), (DWORD)(_tcslen(TEXT("\t<Startup/>\r\n")) * sizeof (TCHAR)), &dwDataRead, NULL) == FALSE)
		goto writerror;

	if ((WriteFile(hXMLFile, TEXT("\t<Shutdown numberofaction=\""), (DWORD)(_tcslen(TEXT("\t<Shutdown numberofaction=\"")) * sizeof (TCHAR)), &dwDataRead, NULL) == FALSE)
		|| (WriteFile(hXMLFile, tShutdown, (DWORD)(dwShutdownLen * sizeof (WCHAR)), &dwDataRead, NULL) == FALSE)
		|| (WriteFile(hXMLFile, TEXT("\">\r\n"), (DWORD)(_tcslen(TEXT("\">\r\n")) * sizeof (TCHAR)), &dwDataRead, NULL) == FALSE))
		goto writerror;
	for (DWORD i = 0; i < pScriptsIniData->dwShutdownScriptNum; ++i)
	{
		PTCHAR tCmdLine = EscapeXMLString(pScriptsIniData->pShutdownScripts[i]->tCmdLine);
		PTCHAR tParams = EscapeXMLString(pScriptsIniData->pShutdownScripts[i]->tParameters);

		if ((WriteFile(hXMLFile, TEXT("\t\t<Action cmdline=\""), (DWORD)(_tcslen(TEXT("\t\t<Action cmdline=\"")) * sizeof (TCHAR)), &dwDataRead, NULL) == FALSE)
			|| (WriteFile(hXMLFile, tCmdLine, (DWORD)(_tcslen(tCmdLine) * sizeof (WCHAR)), &dwDataRead, NULL) == FALSE)
			|| (WriteFile(hXMLFile, TEXT("\" parameters=\""), (DWORD)(_tcslen(TEXT("\" parameters=\"")) * sizeof (TCHAR)), &dwDataRead, NULL) == FALSE)
			|| (WriteFile(hXMLFile, tParams, (DWORD)(_tcslen(tParams) * sizeof (WCHAR)), &dwDataRead, NULL) == FALSE)
			|| (WriteFile(hXMLFile, TEXT("\"/>\r\n"), (DWORD)(_tcslen(TEXT("\"/>\r\n")) * sizeof (TCHAR)), &dwDataRead, NULL) == FALSE))
			goto writerror;
		if (tCmdLine)
			HeapFree(hCrawlerHeap, NULL, tCmdLine);
		if (tParams)
			HeapFree(hCrawlerHeap, NULL, tParams);
	}
	if (WriteFile(hXMLFile, TEXT("\t<Shutdown/>\r\n"), (DWORD)(_tcslen(TEXT("\t<Shutdown/>\r\n")) * sizeof (TCHAR)), &dwDataRead, NULL) == FALSE)
		goto writerror;

	for (DWORD i = 0; i < pScriptsIniData->dwNumberOfUnReferrencedSections; ++i)
	{
		PINI_SECTION_DATA pCurrSectionData = pScriptsIniData->pUnReferrencedSections[i];

		if (!pCurrSectionData->tSectionName)
			continue;

		PrintXMLUnreferencedSectionData(pCurrSectionData, hXMLFile);
	}	

	CloseHandle(hXMLFile);
	return TRUE;
writerror:
	DEBUG_LOG(D_WARNING, "Unable to write XML DATA.\r\nExiting now...");
	DoExit(D_ERROR);
	return FALSE;
}

BOOL PrintXMLUnreferencedSectionData(_In_ PINI_SECTION_DATA pSectionData, _In_ HANDLE hXMLFile)
{
	DWORD dwDataRead = 0;

	if (!pSectionData || !(pSectionData->tSectionName))
	{
		DEBUG_LOG(D_WARNING, "PINI_SECTION_DATA invalid for SCRIPTS.ini file.\r\n");
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

BOOL PrintCSVData(_In_ PSCRIPTSINI_FILE_DATA pScriptsIniData)
{
	DWORD dwDataRead = 0;
	HANDLE hCSVFile = INVALID_HANDLE_VALUE;
	PTCHAR tOutputDirectoryScriptsFile = OUTPUT_DIRECTORY_SCRIPTS_INI;
	GPO_FILTER_TARGET dwTarget = GPO_FILTER_UNKNOWN;

	// Determine if we need to create user or computer file in case of '[Machine||User]' 
	dwTarget = GetTargetGPO(pScriptsIniData->tFilePath);
	if (dwTarget == GPO_FILTER_TARGET_MACHINE)
		tOutputDirectoryScriptsFile = TEXT(".\\Machine\\");
	else if (dwTarget == GPO_FILTER_TARGET_USER)
		tOutputDirectoryScriptsFile = TEXT(".\\User\\");	
	hCSVFile = GetFileHandle(OUTPUT_FILE_CSV, tOutputDirectoryScriptsFile, OUTPUT_NAME_SCRIPTS_INI);

	if (!pScriptsIniData || !(pScriptsIniData->tFilePath))
	{
		DEBUG_LOG(D_WARNING, "PSCRIPTSINI_FILE_DATA invalid for CSV file.\r\n");
		DoExit(D_WARNING);
	}

	if (hCSVFile == INVALID_HANDLE_VALUE)
	{
		DEBUG_LOG(D_WARNING, "Handle to hCSVFile is invalid.\r\nExiting now...");
		DoExit(D_ERROR);
	}

	if ((WriteFile(hCSVFile, pScriptsIniData->tFilePath, (DWORD)(_tcslen(pScriptsIniData->tFilePath) * sizeof (WCHAR)), &dwDataRead, NULL) == FALSE)
		|| (WriteFile(hCSVFile, TEXT(";"), (DWORD)(_tcslen(TEXT(";")) * sizeof (TCHAR)), &dwDataRead, NULL) == FALSE))
		goto writerror;	

	for (DWORD i = 0; i < pScriptsIniData->dwLogonScriptNum; ++i)
	{
		if ((WriteFile(hCSVFile, TEXT("(CmdLine:{"), (DWORD)(_tcslen(TEXT("(CmdLine:{")) * sizeof (TCHAR)), &dwDataRead, NULL) == FALSE)
			|| (WriteFile(hCSVFile, pScriptsIniData->pLogonScripts[i]->tCmdLine, (DWORD)(_tcslen(pScriptsIniData->pLogonScripts[i]->tCmdLine) * sizeof (WCHAR)), &dwDataRead, NULL) == FALSE)
			|| (WriteFile(hCSVFile, TEXT("}, Parameters:{"), (DWORD)(_tcslen(TEXT("}, Parameters:{")) * sizeof (TCHAR)), &dwDataRead, NULL) == FALSE)
			|| (WriteFile(hCSVFile, pScriptsIniData->pLogonScripts[i]->tParameters, (DWORD)(_tcslen(pScriptsIniData->pLogonScripts[i]->tParameters) * sizeof (WCHAR)), &dwDataRead, NULL) == FALSE)
			|| (WriteFile(hCSVFile, TEXT("})"), (DWORD)(_tcslen(TEXT("})")) * sizeof (TCHAR)), &dwDataRead, NULL) == FALSE))
			goto writerror;	
	}
	if (!pScriptsIniData->dwLogonScriptNum)
	{
		if (WriteFile(hCSVFile, TEXT("(missing);"), (DWORD)(_tcslen(TEXT("(missing);")) * sizeof (TCHAR)), &dwDataRead, NULL) == FALSE)
			goto writerror;	
	}
	else
	{
		if (WriteFile(hCSVFile, TEXT(";"), (DWORD)(_tcslen(TEXT(";")) * sizeof (TCHAR)), &dwDataRead, NULL) == FALSE)
			goto writerror;	
	}

	for (DWORD i = 0; i < pScriptsIniData->dwLogoffScriptNum; ++i)
	{
		if ((WriteFile(hCSVFile, TEXT("(CmdLine:{"), (DWORD)(_tcslen(TEXT("(CmdLine:{")) * sizeof (TCHAR)), &dwDataRead, NULL) == FALSE)
			|| (WriteFile(hCSVFile, pScriptsIniData->pLogoffScripts[i]->tCmdLine, (DWORD)(_tcslen(pScriptsIniData->pLogoffScripts[i]->tCmdLine) * sizeof (WCHAR)), &dwDataRead, NULL) == FALSE)
			|| (WriteFile(hCSVFile, TEXT("}, Parameters:{"), (DWORD)(_tcslen(TEXT("}, Parameters:{")) * sizeof (TCHAR)), &dwDataRead, NULL) == FALSE)
			|| (WriteFile(hCSVFile, pScriptsIniData->pLogoffScripts[i]->tParameters, (DWORD)(_tcslen(pScriptsIniData->pLogoffScripts[i]->tParameters) * sizeof (WCHAR)), &dwDataRead, NULL) == FALSE)
			|| (WriteFile(hCSVFile, TEXT("})"), (DWORD)(_tcslen(TEXT("})")) * sizeof (TCHAR)), &dwDataRead, NULL) == FALSE))
			goto writerror;	
	}
	if (!pScriptsIniData->dwLogoffScriptNum)
	{
		if (WriteFile(hCSVFile, TEXT("(missing);"), (DWORD)(_tcslen(TEXT("(missing);")) * sizeof (TCHAR)), &dwDataRead, NULL) == FALSE)
			goto writerror;	
	}
	else
	{
		if (WriteFile(hCSVFile, TEXT(";"), (DWORD)(_tcslen(TEXT(";")) * sizeof (TCHAR)), &dwDataRead, NULL) == FALSE)
			goto writerror;	

	}

	for (DWORD i = 0; i < pScriptsIniData->dwStartupScriptNum; ++i)
	{
		if ((WriteFile(hCSVFile, TEXT("(CmdLine:{"), (DWORD)(_tcslen(TEXT("(CmdLine:{")) * sizeof (TCHAR)), &dwDataRead, NULL) == FALSE)
			|| (WriteFile(hCSVFile, pScriptsIniData->pStartupScripts[i]->tCmdLine, (DWORD)(_tcslen(pScriptsIniData->pStartupScripts[i]->tCmdLine) * sizeof (WCHAR)), &dwDataRead, NULL) == FALSE)
			|| (WriteFile(hCSVFile, TEXT("}, Parameters:{"), (DWORD)(_tcslen(TEXT("}, Parameters:{")) * sizeof (TCHAR)), &dwDataRead, NULL) == FALSE)
			|| (WriteFile(hCSVFile, pScriptsIniData->pStartupScripts[i]->tParameters, (DWORD)(_tcslen(pScriptsIniData->pStartupScripts[i]->tParameters) * sizeof (WCHAR)), &dwDataRead, NULL) == FALSE)
			|| (WriteFile(hCSVFile, TEXT("})"), (DWORD)(_tcslen(TEXT("})")) * sizeof (TCHAR)), &dwDataRead, NULL) == FALSE))
			goto writerror;	
	}
	if (!pScriptsIniData->dwStartupScriptNum)
	{
		if (WriteFile(hCSVFile, TEXT("(missing);"), (DWORD)(_tcslen(TEXT("(missing);")) * sizeof (TCHAR)), &dwDataRead, NULL) == FALSE)
			goto writerror;	
	}
	else
	{
		if (WriteFile(hCSVFile, TEXT(";"), (DWORD)(_tcslen(TEXT(";")) * sizeof (TCHAR)), &dwDataRead, NULL) == FALSE)
			goto writerror;	
	}

	for (DWORD i = 0; i < pScriptsIniData->dwShutdownScriptNum; ++i)
	{
		if ((WriteFile(hCSVFile, TEXT("(CmdLine:{"), (DWORD)(_tcslen(TEXT("(CmdLine:{")) * sizeof (TCHAR)), &dwDataRead, NULL) == FALSE)
			|| (WriteFile(hCSVFile, pScriptsIniData->pShutdownScripts[i]->tCmdLine, (DWORD)(_tcslen(pScriptsIniData->pShutdownScripts[i]->tCmdLine) * sizeof (WCHAR)), &dwDataRead, NULL) == FALSE)
			|| (WriteFile(hCSVFile, TEXT("}, Parameters:{"), (DWORD)(_tcslen(TEXT("}, Parameters:{")) * sizeof (TCHAR)), &dwDataRead, NULL) == FALSE)
			|| (WriteFile(hCSVFile, pScriptsIniData->pShutdownScripts[i]->tParameters, (DWORD)(_tcslen(pScriptsIniData->pShutdownScripts[i]->tParameters) * sizeof (WCHAR)), &dwDataRead, NULL) == FALSE)
			|| (WriteFile(hCSVFile, TEXT("})"), (DWORD)(_tcslen(TEXT("})")) * sizeof (TCHAR)), &dwDataRead, NULL) == FALSE))
			goto writerror;	
	}
	if (!pScriptsIniData->dwShutdownScriptNum)
	{
		if (WriteFile(hCSVFile, TEXT("(missing);"), (DWORD)(_tcslen(TEXT("(missing);")) * sizeof (TCHAR)), &dwDataRead, NULL) == FALSE)
			goto writerror;	
	}
	else
	{
		if (WriteFile(hCSVFile, TEXT(";"), (DWORD)(_tcslen(TEXT(";")) * sizeof (TCHAR)), &dwDataRead, NULL) == FALSE)
			goto writerror;	
	}

	for (DWORD i = 0; i < pScriptsIniData->dwNumberOfUnReferrencedSections; ++i)
	{
		PINI_SECTION_DATA pCurrSectionData = pScriptsIniData->pUnReferrencedSections[i];

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

BOOL PrintSTDOUTData(_In_ PSCRIPTSINI_FILE_DATA pScriptsIniData)
{
	if (!pScriptsIniData)
	{
		DEBUG_LOG(D_WARNING, "PSCRIPTSINI_FILE_DATA invalid for scripts.ini file.\r\n");
		DoExit(D_ERROR);
	}

	printf("[SCRIPTSini] File=%ws", pScriptsIniData->tFilePath);

	printf(" Logon=");
	for (DWORD i = 0; i < pScriptsIniData->dwLogonScriptNum; ++i)
		printf("(CmdLine:{%ws}, Parameters:{%ws})", pScriptsIniData->pLogonScripts[i]->tCmdLine , pScriptsIniData->pLogonScripts[i]->tParameters);
	if (!pScriptsIniData->dwLogonScriptNum)
		printf("(missing)");

	printf(" Logoff=");
	for (DWORD i = 0; i < pScriptsIniData->dwLogoffScriptNum; ++i)
		printf("(CmdLine:{%ws}, Parameters:{%ws})", pScriptsIniData->pLogoffScripts[i]->tCmdLine , pScriptsIniData->pLogoffScripts[i]->tParameters);
	if (!pScriptsIniData->dwLogoffScriptNum)
		printf("(missing)");

	printf(" Startup=");
	for (DWORD i = 0; i < pScriptsIniData->dwStartupScriptNum; ++i)
		printf("(CmdLine:{%ws}, Parameters:{%ws})", pScriptsIniData->pStartupScripts[i]->tCmdLine , pScriptsIniData->pStartupScripts[i]->tParameters);
	if (!pScriptsIniData->dwStartupScriptNum)
		printf("(missing)");


	printf(" Shutdown=");
	for (DWORD i = 0; i < pScriptsIniData->dwShutdownScriptNum; ++i)
		printf("(CmdLine:{%ws}, Parameters:{%ws})", pScriptsIniData->pShutdownScripts[i]->tCmdLine , pScriptsIniData->pShutdownScripts[i]->tParameters);
	if (!pScriptsIniData->dwShutdownScriptNum)
		printf("(missing)");

	for (DWORD i = 0; i < pScriptsIniData->dwNumberOfUnReferrencedSections; ++i)
	{
		PINI_SECTION_DATA pCurrSectionData = pScriptsIniData->pUnReferrencedSections[i];

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