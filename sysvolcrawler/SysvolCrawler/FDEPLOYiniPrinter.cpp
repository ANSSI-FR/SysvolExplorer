/**************************************************
* SysvolCrawler - FDEPLOYiniPrinter.h
* AUTHOR: Luc Delsalle	
*
* Display or store data for folder deployment file
*	
* ANSSI/COSSI/DTO/BAI - 2014
***************************************************/

#include "FDEPLOYiniPrinter.h"

BOOL PrintData(_In_ PFDEPLOYINI_FILE_DATA pFdeployIniFileData)
{
	BOOL bRes = TRUE;

	if (pFdeployIniFileData == NULL)
	{
		DEBUG_LOG(D_ERROR, "FDEPLOYINI_FILE_DATA pointer invalid.\r\nExiting now...");
		DoExit(D_ERROR);
	}

	// Call every printer
	if ((bRes) && (pSyscrwlrOptions->bShouldPrintXML))
		bRes = PrintXMLData(pFdeployIniFileData);

	if ((bRes) && (pSyscrwlrOptions->bShouldPrintCSV))
		bRes = PrintCSVData(pFdeployIniFileData);

	if ((bRes) && (pSyscrwlrOptions->bShouldPrintSTDOUT))
		bRes = PrintSTDOUTData(pFdeployIniFileData);

	return bRes;
}

BOOL PrintFdeployIniDataHeader(_In_ PTCHAR tFilePath)
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
		HANDLE hXMLFile = GetFileHandle(OUTPUT_FILE_XML, OUTPUT_DIRECTORY_FDEPLOY_INI, OUTPUT_NAME_FDEPLOY_INI);

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
			|| (WriteFile(hXMLFile, OUTPUT_NAME_FDEPLOY_INI, (DWORD)(_tcslen(OUTPUT_NAME_FDEPLOY_INI) * sizeof (TCHAR)), &dwDataRead, NULL) == FALSE)
			|| (WriteFile(hXMLFile, TEXT(".xml>\r\n"), (DWORD)(_tcslen(TEXT(".xml>\r\n")) * sizeof (TCHAR)), &dwDataRead, NULL) == FALSE))
				goto writerror;
		}

		if ((WriteFile(hXMLFile, TEXT("<FDEPLOYiniFiles Filename=\""), (DWORD)(_tcslen(TEXT("<FDEPLOYiniFiles Filename=\"")) * sizeof (TCHAR)), &dwDataRead, NULL) == FALSE)
		|| (WriteFile(hXMLFile, tFilePath, (DWORD)(_tcslen(tFilePath) * sizeof (TCHAR)), &dwDataRead, NULL) == FALSE)
		|| (WriteFile(hXMLFile, TEXT("\">\r\n"), (DWORD)(_tcslen(TEXT("\">\r\n")) * sizeof (TCHAR)), &dwDataRead, NULL) == FALSE))
			goto writerror;

		CloseHandle(hXMLFile);
	}

	if (pSyscrwlrOptions->bShouldPrintCSV)
	{
		HANDLE hCSVFile = GetFileHandle(OUTPUT_FILE_CSV, OUTPUT_DIRECTORY_FDEPLOY_INI, OUTPUT_NAME_FDEPLOY_INI);
		LARGE_INTEGER liFileSize;

		if (!GetFileSizeEx(hCSVFile, &liFileSize))
		{
			DEBUG_LOG(D_WARNING, "Unable to determine file size.\r\nExiting now...");
			DoExit(D_ERROR);
		}

		if ((liFileSize.HighPart == 0) && (liFileSize.LowPart == 0))
		{
			if (WriteFile(hCSVFile, TEXT("File;FolderStatus;MyDocuments;MyPictures;AppData;Desktop;StartMenu;Programs;Startup;Unreferrenced\r\n"), (DWORD)(_tcslen(TEXT("File;FolderStatus;MyDocuments;MyPictures;AppData;Desktop;StartMenu;Programs;Startup;Unreferrenced\r\n")) * sizeof (TCHAR)), &dwDataRead, NULL) == FALSE)
				goto writerror;
		}
		CloseHandle(hCSVFile);
	}
	return TRUE;

writerror:
	DEBUG_LOG(D_WARNING, "Unable to write DATA HEADER for FDEPLOYini printer.\r\nExiting now...");
	DoExit(D_ERROR);
	return FALSE;
}

BOOL PrintFdeployIniDataFooter(_In_ PTCHAR tFilePath)
{
	DWORD dwDataRead = 0;

	// Hack for closing xml document. Ugly.
	if (pSyscrwlrOptions->bShouldPrintXML)
	{
		HANDLE hXMLFile = GetFileHandle(OUTPUT_FILE_XML, OUTPUT_DIRECTORY_FDEPLOY_INI, OUTPUT_NAME_FDEPLOY_INI);
		if (WriteFile(hXMLFile, TEXT("<FDEPLOYiniFiles/>\r\n"), (DWORD)(_tcslen(TEXT("<FDEPLOYiniFiles/>\r\n")) * sizeof (TCHAR)), &dwDataRead, NULL) == FALSE)
			goto writerror;
		CloseHandle(hXMLFile);
	}
	return TRUE;

writerror:
	DEBUG_LOG(D_WARNING, "Unable to write DATA FOOTER for FDEPLOYini printer.\r\nExiting now...");
	DoExit(D_ERROR);
	return FALSE;
}

BOOL PrintXMLData(_In_ PFDEPLOYINI_FILE_DATA pFdeployIniFileData)
{
	DWORD dwDataRead = 0;
	HANDLE hXMLFile = INVALID_HANDLE_VALUE;
	TCHAR tStatus[11], tMyDocs[11], tMyPics[11], tAppData[11], tDesktop[11], tStartMenu[11], tPrograms[11], tStartup[11];
	DWORD dwStatusLen = 0, dwMyDocsLen = 0, dwMyPicsLen = 0, dwAppDataLen = 0, dwDesktopLen = 0, dwStartMenuLen = 0, dwProgramsLen = 0, dwStartupLen = 0;

	hXMLFile = GetFileHandle(OUTPUT_FILE_XML, OUTPUT_DIRECTORY_FDEPLOY_INI, OUTPUT_NAME_FDEPLOY_INI);

	if (!pFdeployIniFileData || !(pFdeployIniFileData->tFilePath))
	{
		DEBUG_LOG(D_WARNING, "PFDEPLOYINI_FILE_DATA invalid for scripts.ini file.\r\n");
		DoExit(D_WARNING);
	}

	if (hXMLFile == INVALID_HANDLE_VALUE)
	{
		DEBUG_LOG(D_WARNING, "Handle to hXMLFile is invalid.\r\nExiting now...");
		DoExit(D_ERROR);
	}

	dwStatusLen = _stprintf_s(tStatus, 11, TEXT("%d"), pFdeployIniFileData->dwFolderStatusNum);
	dwMyDocsLen = _stprintf_s(tMyDocs, 11, TEXT("%d"), pFdeployIniFileData->dwMyDocumentsRedirectionNum);
	dwMyPicsLen = _stprintf_s(tMyPics, 11, TEXT("%d"), pFdeployIniFileData->dwMyPicturesRedirectionNum);
	dwAppDataLen = _stprintf_s(tAppData, 11, TEXT("%d"), pFdeployIniFileData->dwAppDataRedirectionNum);
	dwDesktopLen = _stprintf_s(tDesktop, 11, TEXT("%d"), pFdeployIniFileData->dwDesktopRedirectionNum);
	dwStartMenuLen = _stprintf_s(tStartMenu, 11, TEXT("%d"), pFdeployIniFileData->dwStartMenuRedirectionNum);
	dwProgramsLen = _stprintf_s(tPrograms, 11, TEXT("%d"), pFdeployIniFileData->dwProgramsRedirectionNum);
	dwStartupLen = _stprintf_s(tStartup, 11, TEXT("%d"), pFdeployIniFileData->dwStartupRedirectionNum);

	if ((WriteFile(hXMLFile, TEXT("\t<Folderstatus numberofstatus=\""), (DWORD)(_tcslen(TEXT("\t<Folderstatus numberofstatus=\"")) * sizeof (TCHAR)), &dwDataRead, NULL) == FALSE)
		|| (WriteFile(hXMLFile, tStatus, (DWORD)(dwStatusLen * sizeof (WCHAR)), &dwDataRead, NULL) == FALSE)
		|| (WriteFile(hXMLFile, TEXT("\">\r\n"), (DWORD)(_tcslen(TEXT("\">\r\n")) * sizeof (TCHAR)), &dwDataRead, NULL) == FALSE))
		goto writerror;
	PrintXMLStatusData(pFdeployIniFileData, hXMLFile);
	if (WriteFile(hXMLFile, TEXT("\t</Folderstatus>\r\n"), (DWORD)(_tcslen(TEXT("\t</Folderstatus>\r\n")) * sizeof (TCHAR)), &dwDataRead, NULL) == FALSE)
		goto writerror;

	if ((WriteFile(hXMLFile, TEXT("\t<MyDocuments numberofredirections=\""), (DWORD)(_tcslen(TEXT("\t<MyDocuments numberofredirections=\"")) * sizeof (TCHAR)), &dwDataRead, NULL) == FALSE)
		|| (WriteFile(hXMLFile, tMyDocs, (DWORD)(dwMyDocsLen * sizeof (WCHAR)), &dwDataRead, NULL) == FALSE)
		|| (WriteFile(hXMLFile, TEXT("\">\r\n"), (DWORD)(_tcslen(TEXT("\">\r\n")) * sizeof (TCHAR)), &dwDataRead, NULL) == FALSE))
		goto writerror;
	PrintXMLRedirectionData(pFdeployIniFileData, hXMLFile, FDEPLOY_MYDOCUMENTS_REDIRECTION_ID);
	if (WriteFile(hXMLFile, TEXT("\t</MyDocuments>\r\n"), (DWORD)(_tcslen(TEXT("\t</MyDocuments>\r\n")) * sizeof (TCHAR)), &dwDataRead, NULL) == FALSE)
		goto writerror;

	if ((WriteFile(hXMLFile, TEXT("\t<MyPictures numberofredirections=\""), (DWORD)(_tcslen(TEXT("\t<MyPictures numberofredirections=\"")) * sizeof (TCHAR)), &dwDataRead, NULL) == FALSE)
		|| (WriteFile(hXMLFile, tMyPics, (DWORD)(dwMyPicsLen * sizeof (WCHAR)), &dwDataRead, NULL) == FALSE)
		|| (WriteFile(hXMLFile, TEXT("\">\r\n"), (DWORD)(_tcslen(TEXT("\">\r\n")) * sizeof (TCHAR)), &dwDataRead, NULL) == FALSE))
		goto writerror;
	PrintXMLRedirectionData(pFdeployIniFileData, hXMLFile, FDEPLOY_MYPICTURES_REDIRECTION_ID);
	if (WriteFile(hXMLFile, TEXT("\t</MyPictures>\r\n"), (DWORD)(_tcslen(TEXT("\t</MyPictures>\r\n")) * sizeof (TCHAR)), &dwDataRead, NULL) == FALSE)
		goto writerror;

	if ((WriteFile(hXMLFile, TEXT("\t<AppData numberofredirections=\""), (DWORD)(_tcslen(TEXT("\t<AppData numberofredirections=\"")) * sizeof (TCHAR)), &dwDataRead, NULL) == FALSE)
		|| (WriteFile(hXMLFile, tAppData, (DWORD)(dwAppDataLen * sizeof (WCHAR)), &dwDataRead, NULL) == FALSE)
		|| (WriteFile(hXMLFile, TEXT("\">\r\n"), (DWORD)(_tcslen(TEXT("\">\r\n")) * sizeof (TCHAR)), &dwDataRead, NULL) == FALSE))
		goto writerror;
	PrintXMLRedirectionData(pFdeployIniFileData, hXMLFile, FDEPLOY_APPDATA_REDIRECTION_ID);
	if (WriteFile(hXMLFile, TEXT("\t</AppData>\r\n"), (DWORD)(_tcslen(TEXT("\t</AppData>\r\n")) * sizeof (TCHAR)), &dwDataRead, NULL) == FALSE)
		goto writerror;

	if ((WriteFile(hXMLFile, TEXT("\t<Desktop numberofredirections=\""), (DWORD)(_tcslen(TEXT("\t<Desktop numberofredirections=\"")) * sizeof (TCHAR)), &dwDataRead, NULL) == FALSE)
		|| (WriteFile(hXMLFile, tDesktop, (DWORD)(dwDesktopLen * sizeof (WCHAR)), &dwDataRead, NULL) == FALSE)
		|| (WriteFile(hXMLFile, TEXT("\">\r\n"), (DWORD)(_tcslen(TEXT("\">\r\n")) * sizeof (TCHAR)), &dwDataRead, NULL) == FALSE))
		goto writerror;
	PrintXMLRedirectionData(pFdeployIniFileData, hXMLFile, FDEPLOY_DESKTOP_REDIRECTION_ID);
	if (WriteFile(hXMLFile, TEXT("\t</Desktop>\r\n"), (DWORD)(_tcslen(TEXT("\t</Desktop>\r\n")) * sizeof (TCHAR)), &dwDataRead, NULL) == FALSE)
		goto writerror;

	if ((WriteFile(hXMLFile, TEXT("\t<StartMenu numberofredirections=\""), (DWORD)(_tcslen(TEXT("\t<StartMenu numberofredirections=\"")) * sizeof (TCHAR)), &dwDataRead, NULL) == FALSE)
		|| (WriteFile(hXMLFile, tStartMenu, (DWORD)(dwStartMenuLen * sizeof (WCHAR)), &dwDataRead, NULL) == FALSE)
		|| (WriteFile(hXMLFile, TEXT("\">\r\n"), (DWORD)(_tcslen(TEXT("\">\r\n")) * sizeof (TCHAR)), &dwDataRead, NULL) == FALSE))
		goto writerror;
	PrintXMLRedirectionData(pFdeployIniFileData, hXMLFile, FDEPLOY_STARTMENU_REDIRECTION_ID);
	if (WriteFile(hXMLFile, TEXT("\t</StartMenu>\r\n"), (DWORD)(_tcslen(TEXT("\t</StartMenu>\r\n")) * sizeof (TCHAR)), &dwDataRead, NULL) == FALSE)
		goto writerror;

	if ((WriteFile(hXMLFile, TEXT("\t<Programs numberofredirections=\""), (DWORD)(_tcslen(TEXT("\t<Programs numberofredirections=\"")) * sizeof (TCHAR)), &dwDataRead, NULL) == FALSE)
		|| (WriteFile(hXMLFile, tPrograms, (DWORD)(dwProgramsLen * sizeof (WCHAR)), &dwDataRead, NULL) == FALSE)
		|| (WriteFile(hXMLFile, TEXT("\">\r\n"), (DWORD)(_tcslen(TEXT("\">\r\n")) * sizeof (TCHAR)), &dwDataRead, NULL) == FALSE))
		goto writerror;
	PrintXMLRedirectionData(pFdeployIniFileData, hXMLFile, FDEPLOY_PROGRAMS_REDIRECTION_ID);
	if (WriteFile(hXMLFile, TEXT("\t</Programs>\r\n"), (DWORD)(_tcslen(TEXT("\t<:Programs>\r\n")) * sizeof (TCHAR)), &dwDataRead, NULL) == FALSE)
		goto writerror;

	if ((WriteFile(hXMLFile, TEXT("\t<Startup numberofredirections=\""), (DWORD)(_tcslen(TEXT("\t<Startup numberofredirections=\"")) * sizeof (TCHAR)), &dwDataRead, NULL) == FALSE)
		|| (WriteFile(hXMLFile, tStartup, (DWORD)(dwStartupLen * sizeof (WCHAR)), &dwDataRead, NULL) == FALSE)
		|| (WriteFile(hXMLFile, TEXT("\">\r\n"), (DWORD)(_tcslen(TEXT("\">\r\n")) * sizeof (TCHAR)), &dwDataRead, NULL) == FALSE))
		goto writerror;
	PrintXMLRedirectionData(pFdeployIniFileData, hXMLFile, FDEPLOY_STARTUP_REDIRECTION_ID);
	if (WriteFile(hXMLFile, TEXT("\t</Startup>\r\n"), (DWORD)(_tcslen(TEXT("\t</Startup>\r\n")) * sizeof (TCHAR)), &dwDataRead, NULL) == FALSE)
		goto writerror;

	for (DWORD i = 0; i < pFdeployIniFileData->dwNumberOfUnReferrencedSections; ++i)
	{
		PINI_SECTION_DATA pCurrSectionData = pFdeployIniFileData->pUnReferrencedSections[i];

		if (!pCurrSectionData->tSectionName)
			continue;

		PrintXMLFdeployUnreferencedSectionData(pCurrSectionData, hXMLFile);
	}	

	CloseHandle(hXMLFile);
	return TRUE;
writerror:
	DEBUG_LOG(D_WARNING, "Unable to write XML DATA.\r\nExiting now...");
	DoExit(D_ERROR);
	return FALSE;
}

BOOL PrintXMLStatusData(_In_ PFDEPLOYINI_FILE_DATA pFdeployIniFileData, _In_ HANDLE hXMLFile)
{
	DWORD dwDataRead = 0;

	if (!pFdeployIniFileData || !hXMLFile)
	{
		DEBUG_LOG(D_WARNING, "PFDEPLOYINI_FILE_DATA invalid for FDEPLOY.ini file.\r\n");
		DoExit(D_WARNING);
	}

	for (DWORD i = 0; i < pFdeployIniFileData->dwFolderStatusNum; ++i)
	{
		TCHAR tStatus[11];
		DWORD dwStatusLen = 0;
		PFDEPLOYINI_FOLDER_STATUS pCurrFolderStatus = pFdeployIniFileData->pFolderStatus[i];

		if (!pCurrFolderStatus)
			continue;

		dwStatusLen = _stprintf_s(tStatus, 11, TEXT("%d"), pCurrFolderStatus->dwStatus);
		if ((WriteFile(hXMLFile, TEXT("\t\t<Folderstatus folder=\""), (DWORD)(_tcslen(TEXT("\t\t<Folderstatus folder=\"")) * sizeof (TCHAR)), &dwDataRead, NULL) == FALSE)
			|| (WriteFile(hXMLFile, pCurrFolderStatus->tTargetedFolder, (DWORD)(_tcslen(pCurrFolderStatus->tTargetedFolder) * sizeof (WCHAR)), &dwDataRead, NULL) == FALSE)
			|| (WriteFile(hXMLFile, TEXT("\" status=\""), (DWORD)(_tcslen(TEXT("\" status=\"")) * sizeof (TCHAR)), &dwDataRead, NULL) == FALSE)
			|| (WriteFile(hXMLFile, tStatus, (DWORD)(dwStatusLen * sizeof (WCHAR)), &dwDataRead, NULL) == FALSE)
			|| (WriteFile(hXMLFile, TEXT("\"/>\r\n"), (DWORD)(_tcslen(TEXT("\"/>\r\n")) * sizeof (TCHAR)), &dwDataRead, NULL) == FALSE))
			goto writerror;
	}

	return TRUE;
writerror:
	DEBUG_LOG(D_WARNING, "Unable to write XML Folder status.\r\nExiting now...");
	DoExit(D_ERROR);
	return FALSE;
}

BOOL PrintXMLRedirectionData(_In_ PFDEPLOYINI_FILE_DATA pFdeployIniFileData, _In_ HANDLE hXMLFile, _In_ FDEPLOY_REDIRECTION_ID dwRedirectionID)
{
	DWORD dwCompt = 0;
	DWORD dwDataRead = 0;

	if (!pFdeployIniFileData || !hXMLFile)
	{
		DEBUG_LOG(D_WARNING, "PFDEPLOYINI_FILE_DATA invalid for FDEPLOY.ini file.\r\n");
		DoExit(D_WARNING);
	}

	switch (dwRedirectionID)
	{
	case FDEPLOY_MYDOCUMENTS_REDIRECTION_ID:
		dwCompt = pFdeployIniFileData->dwMyDocumentsRedirectionNum;
		break;
	case FDEPLOY_MYPICTURES_REDIRECTION_ID:
		dwCompt = pFdeployIniFileData->dwMyPicturesRedirectionNum;
		break;
	case FDEPLOY_APPDATA_REDIRECTION_ID:
		dwCompt = pFdeployIniFileData->dwAppDataRedirectionNum;
		break;
	case FDEPLOY_DESKTOP_REDIRECTION_ID:
		dwCompt = pFdeployIniFileData->dwDesktopRedirectionNum;
		break;
	case FDEPLOY_STARTMENU_REDIRECTION_ID:
		dwCompt = pFdeployIniFileData->dwStartMenuRedirectionNum;
		break;
	case FDEPLOY_PROGRAMS_REDIRECTION_ID:
		dwCompt = pFdeployIniFileData->dwProgramsRedirectionNum;
		break;
	case FDEPLOY_STARTUP_REDIRECTION_ID:
		dwCompt = pFdeployIniFileData->dwStartupRedirectionNum;
		break;
	default:
		DEBUG_LOG(D_ERROR, "Unable to identify FDEPLOY_REDIRECTION_ID.\r\nExiting now...");
		DoExit(D_ERROR);
		break;
	}

	for (DWORD i = 0; i < dwCompt; ++i)
	{
		PFDEPLOYINI_FOLDER_REDIRECTION pCurrFolderRedirection = NULL;

		switch (dwRedirectionID)
		{
		case FDEPLOY_MYDOCUMENTS_REDIRECTION_ID:
			pCurrFolderRedirection = pFdeployIniFileData->pMyDocumentsRedirection[i];
			break;
		case FDEPLOY_MYPICTURES_REDIRECTION_ID:
			pCurrFolderRedirection = pFdeployIniFileData->pMyPicturesRedirection[i];
			break;
		case FDEPLOY_APPDATA_REDIRECTION_ID:
			pCurrFolderRedirection = pFdeployIniFileData->pAppdataRedirection[i];
			break;
		case FDEPLOY_DESKTOP_REDIRECTION_ID:
			pCurrFolderRedirection = pFdeployIniFileData->pDesktopRedirection[i];
			break;
		case FDEPLOY_STARTMENU_REDIRECTION_ID:
			pCurrFolderRedirection = pFdeployIniFileData->pStartMenuRedirection[i];
			break;
		case FDEPLOY_PROGRAMS_REDIRECTION_ID:
			pCurrFolderRedirection = pFdeployIniFileData->pProgramsRedirection[i];
			break;
		case FDEPLOY_STARTUP_REDIRECTION_ID:
			pCurrFolderRedirection = pFdeployIniFileData->pStartupRedirection[i];
			break;
		default:
			DEBUG_LOG(D_ERROR, "Unable to identify FDEPLOY_REDIRECTION_ID.\r\nExiting now...");
			DoExit(D_ERROR);
			break;
		}

		if (!pCurrFolderRedirection)
			continue;

		if ((WriteFile(hXMLFile, TEXT("\t\t<FolderRedirection targetedsid=\""), (DWORD)(_tcslen(TEXT("\t\t<FolderRedirection targetedsid=\"")) * sizeof (TCHAR)), &dwDataRead, NULL) == FALSE)
			|| (WriteFile(hXMLFile, pCurrFolderRedirection->tTargetedSID, (DWORD)(_tcslen(pCurrFolderRedirection->tTargetedSID) * sizeof (WCHAR)), &dwDataRead, NULL) == FALSE)
			|| (WriteFile(hXMLFile, TEXT("\" redirectionpath=\""), (DWORD)(_tcslen(TEXT("\" redirectionpath=\"")) * sizeof (TCHAR)), &dwDataRead, NULL) == FALSE)
			|| (WriteFile(hXMLFile, pCurrFolderRedirection->tRedirectionPath, (DWORD)(_tcslen(pCurrFolderRedirection->tRedirectionPath) * sizeof (WCHAR)), &dwDataRead, NULL) == FALSE)
			|| (WriteFile(hXMLFile, TEXT("\"/>\r\n"), (DWORD)(_tcslen(TEXT("\"/>\r\n")) * sizeof (TCHAR)), &dwDataRead, NULL) == FALSE))
			goto writerror;
	}

	return TRUE;
writerror:
	DEBUG_LOG(D_WARNING, "Unable to write XML Folder redirection.\r\nExiting now...");
	DoExit(D_ERROR);
	return FALSE;
}

BOOL PrintXMLFdeployUnreferencedSectionData(_In_ PINI_SECTION_DATA pSectionData, _In_ HANDLE hXMLFile)
{
	DWORD dwDataRead = 0;

	if (!pSectionData || !(pSectionData->tSectionName))
	{
		DEBUG_LOG(D_WARNING, "PINI_SECTION_DATA invalid for FDEPLOY.ini file.\r\n");
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

BOOL PrintCSVData(_In_ PFDEPLOYINI_FILE_DATA pFdeployIniFileData)
{
	DWORD dwDataRead = 0;
	HANDLE hCSVFile = INVALID_HANDLE_VALUE;

	if (!pFdeployIniFileData || !(pFdeployIniFileData->tFilePath))
	{
		DEBUG_LOG(D_WARNING, "PFDEPLOYINI_FILE_DATA invalid for CSV file.\r\n");
		DoExit(D_WARNING);
	}

	hCSVFile = GetFileHandle(OUTPUT_FILE_CSV, OUTPUT_DIRECTORY_FDEPLOY_INI, OUTPUT_NAME_FDEPLOY_INI);
	if (hCSVFile == INVALID_HANDLE_VALUE)
	{
		DEBUG_LOG(D_WARNING, "Handle to hCSVFile is invalid.\r\nExiting now...");
		DoExit(D_ERROR);
	}

	if ((WriteFile(hCSVFile, pFdeployIniFileData->tFilePath, (DWORD)(_tcslen(pFdeployIniFileData->tFilePath) * sizeof (WCHAR)), &dwDataRead, NULL) == FALSE)
		|| (WriteFile(hCSVFile, TEXT(";"), (DWORD)(_tcslen(TEXT(";")) * sizeof (TCHAR)), &dwDataRead, NULL) == FALSE))
		goto writerror;	

	PrintCSVStatusData(pFdeployIniFileData, hCSVFile);
	PrintCSVRedirectionData(pFdeployIniFileData, hCSVFile, FDEPLOY_MYDOCUMENTS_REDIRECTION_ID);
	PrintCSVRedirectionData(pFdeployIniFileData, hCSVFile, FDEPLOY_MYPICTURES_REDIRECTION_ID);
	PrintCSVRedirectionData(pFdeployIniFileData, hCSVFile, FDEPLOY_APPDATA_REDIRECTION_ID);
	PrintCSVRedirectionData(pFdeployIniFileData, hCSVFile, FDEPLOY_DESKTOP_REDIRECTION_ID);
	PrintCSVRedirectionData(pFdeployIniFileData, hCSVFile, FDEPLOY_STARTMENU_REDIRECTION_ID);
	PrintCSVRedirectionData(pFdeployIniFileData, hCSVFile, FDEPLOY_PROGRAMS_REDIRECTION_ID);
	PrintCSVRedirectionData(pFdeployIniFileData, hCSVFile, FDEPLOY_STARTUP_REDIRECTION_ID);

	for (DWORD i = 0; i < pFdeployIniFileData->dwNumberOfUnReferrencedSections; ++i)
	{
		PINI_SECTION_DATA pCurrSectionData = pFdeployIniFileData->pUnReferrencedSections[i];

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

BOOL PrintCSVStatusData(_In_ PFDEPLOYINI_FILE_DATA pFdeployIniFileData, _In_ HANDLE hCSVFile)
{
	DWORD dwDataRead = 0;

	if (!pFdeployIniFileData || !hCSVFile)
	{
		DEBUG_LOG(D_WARNING, "PFDEPLOYINI_FILE_DATA invalid for FDEPLOY.ini file.\r\n");
		DoExit(D_WARNING);
	}

	for (DWORD i = 0; i < pFdeployIniFileData->dwFolderStatusNum; ++i)
	{
		TCHAR tStatus[11];
		DWORD dwStatusLen = 0;
		PFDEPLOYINI_FOLDER_STATUS pCurrFolderStatus = pFdeployIniFileData->pFolderStatus[i];

		if (!pCurrFolderStatus)
			continue;

		dwStatusLen = _stprintf_s(tStatus, 11, TEXT("%d"), pCurrFolderStatus->dwStatus);
		if ((WriteFile(hCSVFile, TEXT("(Folder:{"), (DWORD)(_tcslen(TEXT("(Folder:{")) * sizeof (TCHAR)), &dwDataRead, NULL) == FALSE)
			|| (WriteFile(hCSVFile, pCurrFolderStatus->tTargetedFolder, (DWORD)(_tcslen(pCurrFolderStatus->tTargetedFolder) * sizeof (WCHAR)), &dwDataRead, NULL) == FALSE)
			|| (WriteFile(hCSVFile, TEXT("}, status:"), (DWORD)(_tcslen(TEXT("}, status:")) * sizeof (TCHAR)), &dwDataRead, NULL) == FALSE)
			|| (WriteFile(hCSVFile, tStatus, (DWORD)(dwStatusLen * sizeof (WCHAR)), &dwDataRead, NULL) == FALSE)
			|| (WriteFile(hCSVFile, TEXT(")"), (DWORD)(_tcslen(TEXT(")")) * sizeof (TCHAR)), &dwDataRead, NULL) == FALSE))
			goto writerror;
	}
	if (!pFdeployIniFileData->dwFolderStatusNum)
	{
		if (WriteFile(hCSVFile, TEXT("(missing);"), (DWORD)(_tcslen(TEXT("(missing);")) * sizeof (TCHAR)), &dwDataRead, NULL) == FALSE)
			goto writerror;	
	}
	else
	{
		if (WriteFile(hCSVFile, TEXT(";"), (DWORD)(_tcslen(TEXT(";")) * sizeof (TCHAR)), &dwDataRead, NULL) == FALSE)
			goto writerror;	
	}

	return TRUE;
writerror:
	DEBUG_LOG(D_WARNING, "Unable to write CSV Folder status.\r\nExiting now...");
	DoExit(D_ERROR);
	return FALSE;
}

BOOL PrintCSVRedirectionData(_In_ PFDEPLOYINI_FILE_DATA pFdeployIniFileData, _In_ HANDLE hCSVFile, _In_ FDEPLOY_REDIRECTION_ID dwRedirectionID)
{
	DWORD dwCompt = 0;
	DWORD dwDataRead = 0;

	if (!pFdeployIniFileData || !hCSVFile)
	{
		DEBUG_LOG(D_WARNING, "PFDEPLOYINI_FILE_DATA invalid for FDEPLOY.ini file.\r\n");
		DoExit(D_WARNING);
	}

	switch (dwRedirectionID)
	{
	case FDEPLOY_MYDOCUMENTS_REDIRECTION_ID:
		dwCompt = pFdeployIniFileData->dwMyDocumentsRedirectionNum;
		break;
	case FDEPLOY_MYPICTURES_REDIRECTION_ID:
		dwCompt = pFdeployIniFileData->dwMyPicturesRedirectionNum;
		break;
	case FDEPLOY_APPDATA_REDIRECTION_ID:
		dwCompt = pFdeployIniFileData->dwAppDataRedirectionNum;
		break;
	case FDEPLOY_DESKTOP_REDIRECTION_ID:
		dwCompt = pFdeployIniFileData->dwDesktopRedirectionNum;
		break;
	case FDEPLOY_STARTMENU_REDIRECTION_ID:
		dwCompt = pFdeployIniFileData->dwStartMenuRedirectionNum;
		break;
	case FDEPLOY_PROGRAMS_REDIRECTION_ID:
		dwCompt = pFdeployIniFileData->dwProgramsRedirectionNum;
		break;
	case FDEPLOY_STARTUP_REDIRECTION_ID:
		dwCompt = pFdeployIniFileData->dwStartupRedirectionNum;
		break;
	default:
		DEBUG_LOG(D_ERROR, "Unable to identify FDEPLOY_REDIRECTION_ID.\r\nExiting now...");
		DoExit(D_ERROR);
		break;
	}

	for (DWORD i = 0; i < dwCompt; ++i)
	{
		PFDEPLOYINI_FOLDER_REDIRECTION pCurrFolderRedirection = NULL;

		switch (dwRedirectionID)
		{
		case FDEPLOY_MYDOCUMENTS_REDIRECTION_ID:
			pCurrFolderRedirection = pFdeployIniFileData->pMyDocumentsRedirection[i];
			break;
		case FDEPLOY_MYPICTURES_REDIRECTION_ID:
			pCurrFolderRedirection = pFdeployIniFileData->pMyPicturesRedirection[i];
			break;
		case FDEPLOY_APPDATA_REDIRECTION_ID:
			pCurrFolderRedirection = pFdeployIniFileData->pAppdataRedirection[i];
			break;
		case FDEPLOY_DESKTOP_REDIRECTION_ID:
			pCurrFolderRedirection = pFdeployIniFileData->pDesktopRedirection[i];
			break;
		case FDEPLOY_STARTMENU_REDIRECTION_ID:
			pCurrFolderRedirection = pFdeployIniFileData->pStartMenuRedirection[i];
			break;
		case FDEPLOY_PROGRAMS_REDIRECTION_ID:
			pCurrFolderRedirection = pFdeployIniFileData->pProgramsRedirection[i];
			break;
		case FDEPLOY_STARTUP_REDIRECTION_ID:
			pCurrFolderRedirection = pFdeployIniFileData->pStartupRedirection[i];
			break;
		default:
			DEBUG_LOG(D_ERROR, "Unable to identify FDEPLOY_REDIRECTION_ID.\r\nExiting now...");
			DoExit(D_ERROR);
			break;
		}

		if (!pCurrFolderRedirection)
			continue;

		if ((WriteFile(hCSVFile, TEXT("(TragetedSid:"), (DWORD)(_tcslen(TEXT("(TragetedSid:")) * sizeof (TCHAR)), &dwDataRead, NULL) == FALSE)
			|| (WriteFile(hCSVFile, pCurrFolderRedirection->tTargetedSID, (DWORD)(_tcslen(pCurrFolderRedirection->tTargetedSID) * sizeof (WCHAR)), &dwDataRead, NULL) == FALSE)
			|| (WriteFile(hCSVFile, TEXT(", RedirectionPath:"), (DWORD)(_tcslen(TEXT(", RedirectionPath:")) * sizeof (TCHAR)), &dwDataRead, NULL) == FALSE)
			|| (WriteFile(hCSVFile, pCurrFolderRedirection->tRedirectionPath, (DWORD)(_tcslen(pCurrFolderRedirection->tRedirectionPath) * sizeof (WCHAR)), &dwDataRead, NULL) == FALSE)
			|| (WriteFile(hCSVFile, TEXT(")"), (DWORD)(_tcslen(TEXT(")")) * sizeof (TCHAR)), &dwDataRead, NULL) == FALSE))
			goto writerror;
	}
	if (!dwCompt)
	{
		if (WriteFile(hCSVFile, TEXT("(missing);"), (DWORD)(_tcslen(TEXT("(missing);")) * sizeof (TCHAR)), &dwDataRead, NULL) == FALSE)
			goto writerror;	
	}
	else
	{
		if (WriteFile(hCSVFile, TEXT(";"), (DWORD)(_tcslen(TEXT(";")) * sizeof (TCHAR)), &dwDataRead, NULL) == FALSE)
			goto writerror;	
	}

	return TRUE;
writerror:
	DEBUG_LOG(D_WARNING, "Unable to write CSV Folder redirection.\r\nExiting now...");
	DoExit(D_ERROR);
	return FALSE;
}

BOOL PrintSTDOUTData(_In_ PFDEPLOYINI_FILE_DATA pFdeployIniFileData)
{
	if (!pFdeployIniFileData)
	{
		DEBUG_LOG(D_WARNING, "PFDEPLOYINI_FILE_DATA invalid for fdeploy.ini file.\r\n");
		DoExit(D_ERROR);
	}

	printf("[FDEPLOYini] File=%ws", pFdeployIniFileData->tFilePath);

	printf(" FolderStatus=");
	PrintSTDOUTStatusData(pFdeployIniFileData);

	printf("FolderRedirection=[MyDocuments:");
	PrintSTDOUTRedirectionData(pFdeployIniFileData, FDEPLOY_MYDOCUMENTS_REDIRECTION_ID);
	printf(" MyPictures:");
	PrintSTDOUTRedirectionData(pFdeployIniFileData, FDEPLOY_MYPICTURES_REDIRECTION_ID);
	printf(" AppData:");
	PrintSTDOUTRedirectionData(pFdeployIniFileData, FDEPLOY_APPDATA_REDIRECTION_ID);
	printf(" Desktop:");
	PrintSTDOUTRedirectionData(pFdeployIniFileData, FDEPLOY_DESKTOP_REDIRECTION_ID);
	printf(" StartMenu:");
	PrintSTDOUTRedirectionData(pFdeployIniFileData, FDEPLOY_STARTMENU_REDIRECTION_ID);
	printf(" Programs:");
	PrintSTDOUTRedirectionData(pFdeployIniFileData, FDEPLOY_PROGRAMS_REDIRECTION_ID);
	printf(" Startup:");
	PrintSTDOUTRedirectionData(pFdeployIniFileData, FDEPLOY_STARTUP_REDIRECTION_ID);
	printf("] ");

	for (DWORD i = 0; i < pFdeployIniFileData->dwNumberOfUnReferrencedSections; ++i)
	{
		PINI_SECTION_DATA pCurrSectionData = pFdeployIniFileData->pUnReferrencedSections[i];

		if (i == 0)
			printf("UnreferrencedSection=");

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

BOOL PrintSTDOUTStatusData(_In_ PFDEPLOYINI_FILE_DATA pFdeployIniFileData)
{
	if (!pFdeployIniFileData)
	{
		DEBUG_LOG(D_WARNING, "PFDEPLOYINI_FILE_DATA invalid for FDEPLOY.ini file.\r\n");
		DoExit(D_WARNING);
	}

	for (DWORD i = 0; i < pFdeployIniFileData->dwFolderStatusNum; ++i)
	{
		PFDEPLOYINI_FOLDER_STATUS pCurrFolderStatus = pFdeployIniFileData->pFolderStatus[i];

		if (!pCurrFolderStatus)
			continue;

		printf("Status:(Folder:{%ws}, Status:%d) ", pCurrFolderStatus->tTargetedFolder , pCurrFolderStatus->dwStatus);
	}
	if (!pFdeployIniFileData->dwFolderStatusNum)
		printf("(missing) ");

	return TRUE;
}

BOOL PrintSTDOUTRedirectionData(_In_ PFDEPLOYINI_FILE_DATA pFdeployIniFileData, _In_ FDEPLOY_REDIRECTION_ID dwRedirectionID)
{
	DWORD dwCompt = 0;

	if (!pFdeployIniFileData)
	{
		DEBUG_LOG(D_WARNING, "PFDEPLOYINI_FILE_DATA invalid for FDEPLOY.ini file.\r\n");
		DoExit(D_WARNING);
	}

	switch (dwRedirectionID)
	{
	case FDEPLOY_MYDOCUMENTS_REDIRECTION_ID:
		dwCompt = pFdeployIniFileData->dwMyDocumentsRedirectionNum;
		break;
	case FDEPLOY_MYPICTURES_REDIRECTION_ID:
		dwCompt = pFdeployIniFileData->dwMyPicturesRedirectionNum;
		break;
	case FDEPLOY_APPDATA_REDIRECTION_ID:
		dwCompt = pFdeployIniFileData->dwAppDataRedirectionNum;
		break;
	case FDEPLOY_DESKTOP_REDIRECTION_ID:
		dwCompt = pFdeployIniFileData->dwDesktopRedirectionNum;
		break;
	case FDEPLOY_STARTMENU_REDIRECTION_ID:
		dwCompt = pFdeployIniFileData->dwStartMenuRedirectionNum;
		break;
	case FDEPLOY_PROGRAMS_REDIRECTION_ID:
		dwCompt = pFdeployIniFileData->dwProgramsRedirectionNum;
		break;
	case FDEPLOY_STARTUP_REDIRECTION_ID:
		dwCompt = pFdeployIniFileData->dwStartupRedirectionNum;
		break;
	default:
		DEBUG_LOG(D_ERROR, "Unable to identify FDEPLOY_REDIRECTION_ID.\r\nExiting now...");
		DoExit(D_ERROR);
		break;
	}

	for (DWORD i = 0; i < dwCompt; ++i)
	{
		PFDEPLOYINI_FOLDER_REDIRECTION pCurrFolderRedirection = NULL;

		switch (dwRedirectionID)
		{
		case FDEPLOY_MYDOCUMENTS_REDIRECTION_ID:
			pCurrFolderRedirection = pFdeployIniFileData->pMyDocumentsRedirection[i];
			break;
		case FDEPLOY_MYPICTURES_REDIRECTION_ID:
			pCurrFolderRedirection = pFdeployIniFileData->pMyPicturesRedirection[i];
			break;
		case FDEPLOY_APPDATA_REDIRECTION_ID:
			pCurrFolderRedirection = pFdeployIniFileData->pAppdataRedirection[i];
			break;
		case FDEPLOY_DESKTOP_REDIRECTION_ID:
			pCurrFolderRedirection = pFdeployIniFileData->pDesktopRedirection[i];
			break;
		case FDEPLOY_STARTMENU_REDIRECTION_ID:
			pCurrFolderRedirection = pFdeployIniFileData->pStartMenuRedirection[i];
			break;
		case FDEPLOY_PROGRAMS_REDIRECTION_ID:
			pCurrFolderRedirection = pFdeployIniFileData->pProgramsRedirection[i];
			break;
		case FDEPLOY_STARTUP_REDIRECTION_ID:
			pCurrFolderRedirection = pFdeployIniFileData->pStartupRedirection[i];
			break;
		default:
			DEBUG_LOG(D_ERROR, "Unable to identify FDEPLOY_REDIRECTION_ID.\r\nExiting now...");
			DoExit(D_ERROR);
			break;
		}

		if (!pCurrFolderRedirection)
			continue;

		printf("(TargetedSid:%ws, Redirectionpath:{%ws}) ", pCurrFolderRedirection->tTargetedSID , pCurrFolderRedirection->tRedirectionPath);
	}
	if (!pFdeployIniFileData->dwFolderStatusNum)
		printf("(missing)");

	return TRUE;
}