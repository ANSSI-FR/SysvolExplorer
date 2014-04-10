/**************************************************
* SysvolCrawler - AASPrinter.cpp
* AUTHOR: Luc Delsalle	
*
* Display or export AAS data
*	
* ANSSI/COSSI/DTO/BAI - 2014
***************************************************/

#include "AASPrinter.h"

BOOL PrintData(_In_ PAAS_FILE_DATA pAasData)
{
	BOOL bRes = TRUE;

	if (pAasData == NULL)
	{
		DEBUG_LOG(D_ERROR, "AAS_FILE_DATA pointer invalid.\r\nExiting now...");
		DoExit(D_ERROR);
	}

	// Call every printer
	if ((bRes) && (pSyscrwlrOptions->bShouldPrintXML))
		bRes = PrintXMLData(pAasData);

	if ((bRes) && (pSyscrwlrOptions->bShouldPrintCSV))
		bRes = PrintCSVData(pAasData);

	if ((bRes) && (pSyscrwlrOptions->bShouldPrintSTDOUT))
		bRes = PrintSTDOUTData(pAasData);

	return bRes;
}

BOOL PrintAasDataHeader(_In_ PTCHAR tFilePath)
{
	DWORD dwDataRead = 0;
	LARGE_INTEGER liFileSize;
	GPO_FILTER_TARGET dwTarget = GPO_FILTER_UNKNOWN;
	PTCHAR tOutputDirectoryAasFile = OUTPUT_DIRECTORY_AAS_FILE;

	if (!tFilePath)
	{
		DEBUG_LOG(D_WARNING, "tFilePath is invalid.\r\nExiting now...");
		DoExit(D_ERROR);
	}

	// Determine if we need to create user or computer file in case of '[Machine||User]' 
	dwTarget = GetTargetGPO(tFilePath);
	if (dwTarget == GPO_FILTER_TARGET_MACHINE)
		tOutputDirectoryAasFile = TEXT(".\\Machine\\");
	else if (dwTarget == GPO_FILTER_TARGET_USER)
		tOutputDirectoryAasFile = TEXT(".\\User\\");

	// Hack for closing xml document. Ugly.
	if (pSyscrwlrOptions->bShouldPrintXML)
	{
		HANDLE hXMLFile = GetFileHandle(OUTPUT_FILE_XML, tOutputDirectoryAasFile, OUTPUT_NAME_AAS_FILE);

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
			|| (WriteFile(hXMLFile, OUTPUT_NAME_AAS_FILE, (DWORD)(_tcslen(OUTPUT_NAME_AAS_FILE) * sizeof (TCHAR)), &dwDataRead, NULL) == FALSE)
			|| (WriteFile(hXMLFile, TEXT(".xml>\r\n"), (DWORD)(_tcslen(TEXT(".xml>\r\n")) * sizeof (TCHAR)), &dwDataRead, NULL) == FALSE))
				goto writerror;
		}

		if ((WriteFile(hXMLFile, TEXT("<AdvertisementApplicationFile Filename=\""), (DWORD)(_tcslen(TEXT("<AdvertisementApplicationFile Filename=\"")) * sizeof (TCHAR)), &dwDataRead, NULL) == FALSE)
		|| (WriteFile(hXMLFile, tFilePath, (DWORD)(_tcslen(tFilePath) * sizeof (TCHAR)), &dwDataRead, NULL) == FALSE)
		|| (WriteFile(hXMLFile, TEXT("\">\r\n"), (DWORD)(_tcslen(TEXT("\">\r\n")) * sizeof (TCHAR)), &dwDataRead, NULL) == FALSE))
			goto writerror;

		CloseHandle(hXMLFile);
	}

	if (pSyscrwlrOptions->bShouldPrintCSV)
	{
		HANDLE hCSVFile = GetFileHandle(OUTPUT_FILE_CSV, tOutputDirectoryAasFile, OUTPUT_NAME_AAS_FILE);
		LARGE_INTEGER liFileSize;

		if (!GetFileSizeEx(hCSVFile, &liFileSize))
		{
			DEBUG_LOG(D_WARNING, "Unable to determine file size.\r\nExiting now...");
			DoExit(D_ERROR);
		}

		if ((liFileSize.HighPart == 0) && (liFileSize.LowPart == 0))
		{
			if (WriteFile(hCSVFile, TEXT("File;Header;ProductInfo;SourceListPublish;ProductPublish;End;UnknownBlock\r\n"), (DWORD)(_tcslen(TEXT("File;Header;ProductInfo;SourceListPublish;ProductPublish;End;UnknownBlock\r\n")) * sizeof (TCHAR)), &dwDataRead, NULL) == FALSE)
				goto writerror;
		}
		CloseHandle(hCSVFile);
	}
	return TRUE;

writerror:
	DEBUG_LOG(D_WARNING, "Unable to write DATA HEADER for AAS printer.\r\nExiting now...");
	DoExit(D_ERROR);
	return FALSE;
}

BOOL PrintAasDataFooter(_In_ PTCHAR tFilePath)
{
	DWORD dwDataRead = 0;
	GPO_FILTER_TARGET dwTarget = GPO_FILTER_UNKNOWN;
	PTCHAR tOutputDirectoryPolFile = OUTPUT_DIRECTORY_AAS_FILE;

	// Determine if we need to create user or computer file in case of '[Machine||User]' 
	dwTarget = GetTargetGPO(tFilePath);
	if (dwTarget == GPO_FILTER_TARGET_MACHINE)
		tOutputDirectoryPolFile = TEXT(".\\Machine\\");
	else if (dwTarget == GPO_FILTER_TARGET_USER)
		tOutputDirectoryPolFile = TEXT(".\\User\\");

	// Hack for closing xml document. Ugly.
	if (pSyscrwlrOptions->bShouldPrintXML)
	{
		HANDLE hXMLFile = GetFileHandle(OUTPUT_FILE_XML, tOutputDirectoryPolFile, OUTPUT_NAME_AAS_FILE);
		if (WriteFile(hXMLFile, TEXT("</AdvertisementApplicationFile>\r\n"), (DWORD)(_tcslen(TEXT("</AdvertisementApplicationFile>\r\n")) * sizeof (TCHAR)), &dwDataRead, NULL) == FALSE)
			goto writerror;
		CloseHandle(hXMLFile);
	}
	return TRUE;

writerror:
	DEBUG_LOG(D_WARNING, "Unable to write DATA FOOTER for AAS printer.\r\nExiting now...");
	DoExit(D_ERROR);
	return FALSE;
}

BOOL PrintXMLData(_In_ PAAS_FILE_DATA pAasData)
{
	DWORD dwDataRead = 0;
	HANDLE hXMLFile = INVALID_HANDLE_VALUE;
	GPO_FILTER_TARGET dwTarget = GPO_FILTER_UNKNOWN;
	PTCHAR tOutputDirectoryAasFile = OUTPUT_DIRECTORY_AAS_FILE;
	PAAS_BLOCK_HEADER pAasBlockHeader = NULL;
	PAAS_BLOCK_PRODUCT_INFO pAasBlockProductInfo = NULL;
	PAAS_BLOCK_SOURCE_LIST_PUBLISH pAasBlockSourceListPublish = NULL;
	PAAS_BLOCK_PRODUCT_PUBLISH pAasBlockProductPublish = NULL;
	PAAS_BLOCK_END pAasBlockEnd = NULL;
	PAAS_BLOCK_UNKNOWN pAasBlockUnkwnown = NULL;

	// Determine if we need to create user or computer file in case of '[Machine||User]' 
	dwTarget = GetTargetGPO(pAasData->tFilePath);
	if (dwTarget == GPO_FILTER_TARGET_MACHINE)
		tOutputDirectoryAasFile = TEXT(".\\Machine\\");
	else if (dwTarget == GPO_FILTER_TARGET_USER)
		tOutputDirectoryAasFile = TEXT(".\\User\\");	
	hXMLFile = GetFileHandle(OUTPUT_FILE_XML, tOutputDirectoryAasFile, OUTPUT_NAME_AAS_FILE);

	if (!pAasData || !(pAasData->tFilePath))
	{
		DEBUG_LOG(D_WARNING, "PAAS_FILE_DATA invalid for AAS file.\r\n");
		DoExit(D_WARNING);
	}

	if (hXMLFile == INVALID_HANDLE_VALUE)
	{
		DEBUG_LOG(D_WARNING, "Handle to hXMLFile is invalid.\r\nExiting now...");
		DoExit(D_ERROR);
	}

	if (pAasData->pAasHeader)
	{
		TCHAR tSignature[11], tVersion[11], tDosTimeStamp[11], tLangID[11],  tPlatform[11],  tScriptType[11], tScriptMajorVersion[11], tScriptMinorversion[11], tScriptAttributes[11];
		DWORD dwSignatureLen = 0, dwVersionLen = 0, dwDosTimeStampLen = 0, dwLangIDLen = 0,  dwPlatformLen = 0,  dwScriptTypeLen = 0, dwScriptMajorVersionLen = 0, dwScriptMinorversionLen = 0, dwScriptAttributesLen = 0;
	
		pAasBlockHeader = pAasData->pAasHeader;
		dwSignatureLen = _stprintf_s(tSignature, 11, TEXT("%d"), *(pAasBlockHeader->pdwSignature));
		dwVersionLen = _stprintf_s(tVersion, 11, TEXT("%d"), *(pAasBlockHeader->pdwVersion));
		dwDosTimeStampLen = _stprintf_s(tDosTimeStamp, 11, TEXT("%d"), *(pAasBlockHeader->pdwDosTimeStamp));
		dwLangIDLen = _stprintf_s(tLangID, 11, TEXT("%d"), *(pAasBlockHeader->pdwLangID));
		dwPlatformLen = _stprintf_s(tPlatform, 11, TEXT("%d"), *(pAasBlockHeader->pdwPlatform));
		dwScriptTypeLen = _stprintf_s(tScriptType, 11, TEXT("%d"), *(pAasBlockHeader->pdwScriptType));
		dwScriptMajorVersionLen = _stprintf_s(tScriptMajorVersion, 11, TEXT("%d"), *(pAasBlockHeader->pdwScriptMajorVersion));
		dwScriptMinorversionLen = _stprintf_s(tScriptMinorversion, 11, TEXT("%d"), *(pAasBlockHeader->pdwScriptMinorVersion));
		dwScriptAttributesLen = _stprintf_s(tScriptAttributes, 11, TEXT("%d"), *(pAasBlockHeader->pdwScriptAttributes));
	
		if ((WriteFile(hXMLFile, TEXT("\t<Header signature=\""), (DWORD)(_tcslen(TEXT("\t<Header signature=\"")) * sizeof (TCHAR)), &dwDataRead, NULL) == FALSE)
			|| (WriteFile(hXMLFile, tSignature, ((dwSignatureLen) * sizeof (WCHAR)), &dwDataRead, NULL) == FALSE)
			|| (WriteFile(hXMLFile, TEXT("\" version=\""), (DWORD)(_tcslen(TEXT("\" version=\"")) * sizeof (TCHAR)), &dwDataRead, NULL) == FALSE)
			|| (WriteFile(hXMLFile, tVersion, ((dwVersionLen) * sizeof (WCHAR)), &dwDataRead, NULL) == FALSE)
			|| (WriteFile(hXMLFile, TEXT("\" dostimestamp=\""), (DWORD)(_tcslen(TEXT("\" dostimestamp=\"")) * sizeof (TCHAR)), &dwDataRead, NULL) == FALSE)
			|| (WriteFile(hXMLFile, tDosTimeStamp, ((dwDosTimeStampLen) * sizeof (WCHAR)), &dwDataRead, NULL) == FALSE)
			|| (WriteFile(hXMLFile, TEXT("\" langid=\""), (DWORD)(_tcslen(TEXT("\" langid=\"")) * sizeof (TCHAR)), &dwDataRead, NULL) == FALSE)
			|| (WriteFile(hXMLFile, tLangID, ((dwLangIDLen) * sizeof (WCHAR)), &dwDataRead, NULL) == FALSE)
			|| (WriteFile(hXMLFile, TEXT("\" platform=\""), (DWORD)(_tcslen(TEXT("\" platform=\"")) * sizeof (TCHAR)), &dwDataRead, NULL) == FALSE)
			|| (WriteFile(hXMLFile, tPlatform, ((dwPlatformLen) * sizeof (WCHAR)), &dwDataRead, NULL) == FALSE)
			|| (WriteFile(hXMLFile, TEXT("\" scripttype=\""), (DWORD)(_tcslen(TEXT("\" scripttype=\"")) * sizeof (TCHAR)), &dwDataRead, NULL) == FALSE)
			|| (WriteFile(hXMLFile, tScriptType, ((dwScriptTypeLen) * sizeof (WCHAR)), &dwDataRead, NULL) == FALSE)
			|| (WriteFile(hXMLFile, TEXT("\" scriptmajorversion=\""), (DWORD)(_tcslen(TEXT("\" scriptmajorversion=\"")) * sizeof (TCHAR)), &dwDataRead, NULL) == FALSE)
			|| (WriteFile(hXMLFile, tScriptMajorVersion, ((dwScriptMajorVersionLen) * sizeof (WCHAR)), &dwDataRead, NULL) == FALSE)
			|| (WriteFile(hXMLFile, TEXT("\" scriptminorversion=\""), (DWORD)(_tcslen(TEXT("\" scriptminorversion=\"")) * sizeof (TCHAR)), &dwDataRead, NULL) == FALSE)
			|| (WriteFile(hXMLFile, tScriptMinorversion, ((dwScriptMinorversionLen) * sizeof (WCHAR)), &dwDataRead, NULL) == FALSE)
			|| (WriteFile(hXMLFile, TEXT("\" scriptattributes=\""), (DWORD)(_tcslen(TEXT("\" scriptattributes=\"")) * sizeof (TCHAR)), &dwDataRead, NULL) == FALSE)
			|| (WriteFile(hXMLFile, tScriptAttributes, ((dwScriptAttributesLen) * sizeof (WCHAR)), &dwDataRead, NULL) == FALSE)
			|| (WriteFile(hXMLFile, TEXT("\"/>\r\n"), (DWORD)(_tcslen(TEXT("\"/>\r\n")) * sizeof (TCHAR)), &dwDataRead, NULL) == FALSE))
			goto writerror;		
	}
	if (pAasData->pAasProductInfo)
	{
		TCHAR tLanguage[11], tVersion[11], tAssignement[11], tObsoleteArg[11], tInstanceType[11], tLUASettings[11], tRemoteURTInstalls[11], tProductDeploymentFlags[11];
		DWORD dwLanguageLen = 0, dwVersionLen = 0, dwAssignementLen = 0, dwObsoleteArgLen = 0, dwInstanceTypeLen = 0, dwLUASettingsLen = 0, dwRemoteURTInstallsLen = 0, dwProductDeploymentFlagsLen = 0;

		pAasBlockProductInfo = pAasData->pAasProductInfo;
		dwLanguageLen = _stprintf_s(tLanguage, 11, TEXT("%d"), *(pAasBlockProductInfo->pdwLanguage));
		dwVersionLen = _stprintf_s(tVersion, 11, TEXT("%d"), *(pAasBlockProductInfo->pdwVersion));
		dwAssignementLen = _stprintf_s(tAssignement, 11, TEXT("%d"), *(pAasBlockProductInfo->pdwAssignment));
		dwObsoleteArgLen = _stprintf_s(tObsoleteArg, 11, TEXT("%d"), *(pAasBlockProductInfo->pdwObsoleteArg));
		dwInstanceTypeLen = _stprintf_s(tInstanceType, 11, TEXT("%d"), *(pAasBlockProductInfo->pdwInstanceType));
		dwLUASettingsLen = _stprintf_s(tLUASettings, 11, TEXT("%d"), *(pAasBlockProductInfo->pdwLUASetting));
		dwRemoteURTInstallsLen = _stprintf_s(tRemoteURTInstalls, 11, TEXT("%d"), *(pAasBlockProductInfo->pdwRemoteURTInstalls));
		dwProductDeploymentFlagsLen = _stprintf_s(tProductDeploymentFlags, 11, TEXT("%d"), *(pAasBlockProductInfo->pdwProductDeploymentFlags));

		pAasBlockProductInfo = pAasData->pAasProductInfo;

		if ((WriteFile(hXMLFile, TEXT("\t<ProductInfo productkey=\""), (DWORD)(_tcslen(TEXT("\t<ProductInfo productkey=\"")) * sizeof (TCHAR)), &dwDataRead, NULL) == FALSE)
			|| (WriteFile(hXMLFile, pAasBlockProductInfo->pwProductKey, (DWORD)(_tcslen(pAasBlockProductInfo->pwProductKey) * sizeof(WCHAR)), &dwDataRead, NULL) == FALSE)
			|| (WriteFile(hXMLFile, TEXT("\" productname=\""), (DWORD)(_tcslen(TEXT("\" productname=\"")) * sizeof (TCHAR)), &dwDataRead, NULL) == FALSE))
			goto writerror;

		if (pAasBlockProductInfo->isProductNameUNICODE)
		{
			if ((WriteFile(hXMLFile, pAasBlockProductInfo->pwProductName, (DWORD)(_tcslen(pAasBlockProductInfo->pwProductName) * sizeof (WCHAR)), &dwDataRead, NULL) == FALSE)
				|| (WriteFile(hXMLFile, TEXT("\" packagename=\""), (DWORD)(_tcslen(TEXT("\" packagename=\"")) * sizeof (TCHAR)), &dwDataRead, NULL) == FALSE))
			goto writerror;
		}
		else
		{
			if ((WriteFile(hXMLFile, pAasBlockProductInfo->pwProductName, (DWORD)(_tcslen(pAasBlockProductInfo->pwProductName) * sizeof (WCHAR)), &dwDataRead, NULL) == FALSE)
				|| (WriteFile(hXMLFile, TEXT("\" packagename=\""), (DWORD)(_tcslen(TEXT("\" packagename=\"")) * sizeof (TCHAR)), &dwDataRead, NULL) == FALSE))
			goto writerror;
		}
		if (pAasBlockProductInfo->isPackageNameUNICODE)
		{
			if ((WriteFile(hXMLFile, pAasBlockProductInfo->pwPackageName, (DWORD)(_tcslen(pAasBlockProductInfo->pwPackageName) * sizeof (WCHAR)), &dwDataRead, NULL) == FALSE)
				|| (WriteFile(hXMLFile, TEXT("\" language=\""), (DWORD)(_tcslen(TEXT("\" packagename=\"")) * sizeof (TCHAR)), &dwDataRead, NULL) == FALSE))
			goto writerror;
		}
		else
		{
			if ((WriteFile(hXMLFile, pAasBlockProductInfo->pwPackageName, (DWORD)(_tcslen(pAasBlockProductInfo->pwPackageName) * sizeof (WCHAR)), &dwDataRead, NULL) == FALSE)
				|| (WriteFile(hXMLFile, TEXT("\" packagename=\""), (DWORD)(_tcslen(TEXT("\" packagename=\"")) * sizeof (TCHAR)), &dwDataRead, NULL) == FALSE))
			goto writerror;
		}

		if ((WriteFile(hXMLFile, tLanguage, ((dwLanguageLen) * sizeof (WCHAR)), &dwDataRead, NULL) == FALSE)
			|| (WriteFile(hXMLFile, TEXT("\" version=\""), (DWORD)(_tcslen(TEXT("\" version=\"")) * sizeof (TCHAR)), &dwDataRead, NULL) == FALSE)
			|| (WriteFile(hXMLFile, tVersion, ((dwVersionLen) * sizeof (WCHAR)), &dwDataRead, NULL) == FALSE)
			|| (WriteFile(hXMLFile, TEXT("\" assignement=\""), (DWORD)(_tcslen(TEXT("\" assignement=\"")) * sizeof (TCHAR)), &dwDataRead, NULL) == FALSE)
			|| (WriteFile(hXMLFile, tAssignement, ((dwAssignementLen) * sizeof (WCHAR)), &dwDataRead, NULL) == FALSE)
			|| (WriteFile(hXMLFile, TEXT("\" obsoletearg=\""), (DWORD)(_tcslen(TEXT("\" obsoletearg=\"")) * sizeof (TCHAR)), &dwDataRead, NULL) == FALSE)
			|| (WriteFile(hXMLFile, tObsoleteArg, ((dwObsoleteArgLen) * sizeof (WCHAR)), &dwDataRead, NULL) == FALSE)
			|| (WriteFile(hXMLFile, TEXT("\" producticon=\""), (DWORD)(_tcslen(TEXT("\" producticon=\"")) * sizeof (TCHAR)), &dwDataRead, NULL) == FALSE)
			|| (WriteFile(hXMLFile, pAasBlockProductInfo->pwProductIcon, (DWORD)(_tcslen(pAasBlockProductInfo->pwProductIcon) * sizeof (WCHAR)), &dwDataRead, NULL) == FALSE)
			|| (WriteFile(hXMLFile, TEXT("\" packagemediapath=\""), (DWORD)(_tcslen(TEXT("\" packagemediapath=\"")) * sizeof (TCHAR)), &dwDataRead, NULL) == FALSE))
			goto writerror;

		if (pAasBlockProductInfo->isPackageMediaPathUNICODE)
		{
			if ((WriteFile(hXMLFile, pAasBlockProductInfo->pwPackageMediaPath, (DWORD)(_tcslen(pAasBlockProductInfo->pwPackageMediaPath) * sizeof (WCHAR)), &dwDataRead, NULL) == FALSE)
				|| (WriteFile(hXMLFile, TEXT("\" packagecode=\""), (DWORD)(_tcslen(TEXT("\" packagecode=\"")) * sizeof (TCHAR)), &dwDataRead, NULL) == FALSE))
			goto writerror;
		}
		else
		{
			if ((WriteFile(hXMLFile, pAasBlockProductInfo->pwPackageMediaPath, (DWORD)(_tcslen(pAasBlockProductInfo->pwPackageMediaPath) * sizeof (WCHAR)), &dwDataRead, NULL) == FALSE)
				|| (WriteFile(hXMLFile, TEXT("\" packagecode=\""), (DWORD)(_tcslen(TEXT("\" packagecode=\"")) * sizeof (TCHAR)), &dwDataRead, NULL) == FALSE))
			goto writerror;
		}
	
		if ((WriteFile(hXMLFile, pAasBlockProductInfo->pwPackageCode, (DWORD)(_tcslen(pAasBlockProductInfo->pwPackageCode) * sizeof (WCHAR)), &dwDataRead, NULL) == FALSE)
			|| (WriteFile(hXMLFile, TEXT("\" instancetype=\""), (DWORD)(_tcslen(TEXT("\" instancetype=\"")) * sizeof (TCHAR)), &dwDataRead, NULL) == FALSE)
			|| (WriteFile(hXMLFile, tInstanceType, ((dwInstanceTypeLen) * sizeof (WCHAR)), &dwDataRead, NULL) == FALSE)
			|| (WriteFile(hXMLFile, TEXT("\" luasettings=\""), (DWORD)(_tcslen(TEXT("\" luasettings=\"")) * sizeof (TCHAR)), &dwDataRead, NULL) == FALSE)
			|| (WriteFile(hXMLFile, tLUASettings, ((dwLUASettingsLen) * sizeof (WCHAR)), &dwDataRead, NULL) == FALSE)
			|| (WriteFile(hXMLFile, TEXT("\" remoteurtinstalls=\""), (DWORD)(_tcslen(TEXT("\" remoteurtinstalls=\"")) * sizeof (TCHAR)), &dwDataRead, NULL) == FALSE)
			|| (WriteFile(hXMLFile, tRemoteURTInstalls, ((dwRemoteURTInstallsLen) * sizeof (WCHAR)), &dwDataRead, NULL) == FALSE)
			|| (WriteFile(hXMLFile, TEXT("\" productdeploymentflags=\""), (DWORD)(_tcslen(TEXT("\" productdeploymentflags=\"")) * sizeof (TCHAR)), &dwDataRead, NULL) == FALSE)
			|| (WriteFile(hXMLFile, tProductDeploymentFlags, ((dwProductDeploymentFlagsLen) * sizeof (WCHAR)), &dwDataRead, NULL) == FALSE)
			|| (WriteFile(hXMLFile, TEXT("\"/>\r\n"), (DWORD)(_tcslen(TEXT("\"/>\r\n")) * sizeof (TCHAR)), &dwDataRead, NULL) == FALSE))
			goto writerror;	
	}
	if (pAasData->pAasSourceListPublish)
	{
		TCHAR tNumberOfDisks[11], tDiskID[11];
		DWORD dwNumberOfDisksLen = 0, dwDiskIDLen = 0;

		pAasBlockSourceListPublish = pAasData->pAasSourceListPublish;
		if ((WriteFile(hXMLFile, TEXT("\t<SourceListPublish patchcode=\""), (DWORD)(_tcslen(TEXT("\t<SourceListPublish patchcode=\"")) * sizeof (TCHAR)), &dwDataRead, NULL) == FALSE)
			|| (WriteFile(hXMLFile, pAasBlockSourceListPublish->pwPatchCode, (DWORD)(_tcslen(pAasBlockSourceListPublish->pwPatchCode) * sizeof (WCHAR)), &dwDataRead, NULL) == FALSE)
			|| (WriteFile(hXMLFile, TEXT("\" patchpackagename=\""), (DWORD)(_tcslen(TEXT("\" patchpackagename=\"")) * sizeof (TCHAR)), &dwDataRead, NULL) == FALSE)
			|| (WriteFile(hXMLFile, pAasBlockSourceListPublish->pwPatchPackageName, (DWORD)(_tcslen(pAasBlockSourceListPublish->pwPatchPackageName) * sizeof (WCHAR)), &dwDataRead, NULL) == FALSE)
			|| (WriteFile(hXMLFile, TEXT("\" diskprompttemplate=\""), (DWORD)(_tcslen(TEXT("\" diskprompttemplate=\"")) * sizeof (TCHAR)), &dwDataRead, NULL) == FALSE))
			goto writerror;

		if (pAasBlockSourceListPublish->isDiskPromptTemplateUNICODE)
		{
			if ((WriteFile(hXMLFile, pAasBlockSourceListPublish->pwDiskPromptTemplate, (DWORD)(_tcslen(pAasBlockSourceListPublish->pwDiskPromptTemplate) * sizeof (WCHAR)), &dwDataRead, NULL) == FALSE)
				|| (WriteFile(hXMLFile, TEXT("\" packagepath=\""), (DWORD)(_tcslen(TEXT("\" packagepath=\"")) * sizeof (TCHAR)), &dwDataRead, NULL) == FALSE))
			goto writerror;
		}
		else
		{
			if ((WriteFile(hXMLFile, pAasBlockSourceListPublish->pwDiskPromptTemplate, (DWORD)(_tcslen(pAasBlockSourceListPublish->pwDiskPromptTemplate) * sizeof (WCHAR)), &dwDataRead, NULL) == FALSE)
				|| (WriteFile(hXMLFile, TEXT("\" packagepath=\""), (DWORD)(_tcslen(TEXT("\" packagepath=\"")) * sizeof (TCHAR)), &dwDataRead, NULL) == FALSE))
			goto writerror;
		}

		if (pAasBlockSourceListPublish->isPackagePathUNICODE)
		{
			if ((WriteFile(hXMLFile, pAasBlockSourceListPublish->pwPackagePath, (DWORD)(_tcslen(pAasBlockSourceListPublish->pwPackagePath) * sizeof (WCHAR)), &dwDataRead, NULL) == FALSE)
				|| (WriteFile(hXMLFile, TEXT("\" launchpath=\""), (DWORD)(_tcslen(TEXT("\" launchpath=\"")) * sizeof (TCHAR)), &dwDataRead, NULL) == FALSE))
			goto writerror;
		}
		else
		{
			if ((WriteFile(hXMLFile, pAasBlockSourceListPublish->pwPackagePath, (DWORD)(_tcslen(pAasBlockSourceListPublish->pwPackagePath) * sizeof (WCHAR)), &dwDataRead, NULL) == FALSE)
				|| (WriteFile(hXMLFile, TEXT("\" launchpath=\""), (DWORD)(_tcslen(TEXT("\" launchpath=\"")) * sizeof (TCHAR)), &dwDataRead, NULL) == FALSE))
			goto writerror;
		}

		if (pAasBlockSourceListPublish->isLaunchPathUNICODE)
		{
			if ((WriteFile(hXMLFile, pAasBlockSourceListPublish->pwLaunchPath, (DWORD)(_tcslen(pAasBlockSourceListPublish->pwLaunchPath) * sizeof (WCHAR)), &dwDataRead, NULL) == FALSE)
				|| (WriteFile(hXMLFile, TEXT("\" numberofdisks=\""), (DWORD)(_tcslen(TEXT("\" numberofdisks=\"")) * sizeof (TCHAR)), &dwDataRead, NULL) == FALSE))
			goto writerror;
		}
		else
		{
			if ((WriteFile(hXMLFile, pAasBlockSourceListPublish->pwLaunchPath, (DWORD)(_tcslen(pAasBlockSourceListPublish->pwLaunchPath) * sizeof (WCHAR)), &dwDataRead, NULL) == FALSE)
				|| (WriteFile(hXMLFile, TEXT("\" numberofdisks=\""), (DWORD)(_tcslen(TEXT("\" numberofdisks=\"")) * sizeof (TCHAR)), &dwDataRead, NULL) == FALSE))
			goto writerror;
		}		

		dwNumberOfDisksLen = _stprintf_s(tNumberOfDisks, 11, TEXT("%d"), *(pAasBlockSourceListPublish->pdwNumberOfDisks));
		if ((WriteFile(hXMLFile, tNumberOfDisks, ((dwNumberOfDisksLen) * sizeof (WCHAR)), &dwDataRead, NULL) == FALSE)
			|| (WriteFile(hXMLFile, TEXT("\">\r\n"), (DWORD)(_tcslen(TEXT("\">\r\n")) * sizeof (TCHAR)), &dwDataRead, NULL) == FALSE))
			goto writerror;

		for (DWORD i = 0; i < *(pAasBlockSourceListPublish->pdwNumberOfDisks); ++i)
		{
			dwDiskIDLen = _stprintf_s(tDiskID, 11, TEXT("%d"), *(pAasBlockSourceListPublish->sDisks[i].pdwDiskId));
			if ((WriteFile(hXMLFile, TEXT("\t\t<Disk diskid=\""), (DWORD)(_tcslen(TEXT("\t\t<Disk diskid=\"")) * sizeof (TCHAR)), &dwDataRead, NULL) == FALSE)
				|| (WriteFile(hXMLFile, tDiskID, ((dwDiskIDLen) * sizeof (WCHAR)), &dwDataRead, NULL) == FALSE)
				|| (WriteFile(hXMLFile, TEXT("\" volumename=\""), (DWORD)(_tcslen(TEXT("\" volumename=\"")) * sizeof (TCHAR)), &dwDataRead, NULL) == FALSE))
				goto writerror;

			if (pAasBlockSourceListPublish->sDisks[i].isVolumeNameUNICODE)
			{
				if ((WriteFile(hXMLFile, pAasBlockSourceListPublish->sDisks[i].pwVolumeName, (DWORD)(_tcslen(pAasBlockSourceListPublish->sDisks[i].pwVolumeName) * sizeof (WCHAR)), &dwDataRead, NULL) == FALSE)
					|| (WriteFile(hXMLFile, TEXT("\" diskprompt=\""), (DWORD)(_tcslen(TEXT("\" diskprompt=\"")) * sizeof (TCHAR)), &dwDataRead, NULL) == FALSE))
				goto writerror;
			}
			else
			{
				if ((WriteFile(hXMLFile, pAasBlockSourceListPublish->sDisks[i].pwVolumeName, (DWORD)(_tcslen(pAasBlockSourceListPublish->sDisks[i].pwVolumeName) * sizeof (WCHAR)), &dwDataRead, NULL) == FALSE)
					|| (WriteFile(hXMLFile, TEXT("\" diskprompt=\""), (DWORD)(_tcslen(TEXT("\" diskprompt=\"")) * sizeof (TCHAR)), &dwDataRead, NULL) == FALSE))
				goto writerror;
			}	

			if (pAasBlockSourceListPublish->sDisks[i].isDiskPromptUNICODE)
			{
				if ((WriteFile(hXMLFile, pAasBlockSourceListPublish->sDisks[i].pwDiskPrompt, (DWORD)(_tcslen(pAasBlockSourceListPublish->sDisks[i].pwDiskPrompt) * sizeof (WCHAR)), &dwDataRead, NULL) == FALSE)
					|| (WriteFile(hXMLFile, TEXT("\"/>\r\n"), (DWORD)(_tcslen(TEXT("\"/>\r\n")) * sizeof (TCHAR)), &dwDataRead, NULL) == FALSE))
				goto writerror;
			}
			else
			{
				if ((WriteFile(hXMLFile, pAasBlockSourceListPublish->sDisks[i].pwDiskPrompt, (DWORD)(_tcslen(pAasBlockSourceListPublish->sDisks[i].pwDiskPrompt) * sizeof (WCHAR)), &dwDataRead, NULL) == FALSE)
					|| (WriteFile(hXMLFile, TEXT("\"/>\r\n"), (DWORD)(_tcslen(TEXT("\"/>\r\n")) * sizeof (TCHAR)), &dwDataRead, NULL) == FALSE))
				goto writerror;
			}
		}
		if (WriteFile(hXMLFile, TEXT("\t<SourceListPublish/>\r\n"), (DWORD)(_tcslen(TEXT("\t<SourceListPublish/>\r\n")) * sizeof (TCHAR)), &dwDataRead, NULL) == FALSE)
			goto writerror;
	}
	if (pAasData->pAasProductPublish)
	{
		pAasBlockProductPublish = pAasData->pAasProductPublish;
		if ((WriteFile(hXMLFile, TEXT("\t<ProductPublish productpublish=\""), (DWORD)(_tcslen(TEXT("\t<ProductPublish productpublish=\"")) * sizeof (TCHAR)), &dwDataRead, NULL) == FALSE)
			|| (WriteFile(hXMLFile, pAasBlockProductPublish->pwProductPublish, (DWORD)(_tcslen(pAasBlockProductPublish->pwProductPublish) * sizeof (WCHAR)), &dwDataRead, NULL) == FALSE)
			|| (WriteFile(hXMLFile, TEXT("\"/>\r\n"), (DWORD)(_tcslen(TEXT("\"/>\r\n")) * sizeof (TCHAR)), &dwDataRead, NULL) == FALSE))
			goto writerror;
	}
	if (pAasData->pAasEnd)
	{
		TCHAR tCheckSum[11], tProgressTotalHDWord[11], tProgressTotalLDWord[11];
		DWORD dwCheckSumLen = 0, dwProgressTotalHDWordLen = 0, dwProgressTotalLDWordLen = 0;
		
		pAasBlockEnd = pAasData->pAasEnd;
		dwCheckSumLen = _stprintf_s(tCheckSum, 11, TEXT("%d"), *(pAasBlockEnd->pdwChecksum));
		dwProgressTotalHDWordLen = _stprintf_s(tProgressTotalHDWord, 11, TEXT("%d"), *(pAasBlockEnd->pdwProgressTotalHDWord));
		dwProgressTotalLDWordLen = _stprintf_s(tProgressTotalLDWord, 11, TEXT("%d"), *(pAasBlockEnd->pdwProgressTotalLDWord));
		

		if ((WriteFile(hXMLFile, TEXT("\t<End checksum=\""), (DWORD)(_tcslen(TEXT("\t<End checksum=\"")) * sizeof (TCHAR)), &dwDataRead, NULL) == FALSE)
			||(WriteFile(hXMLFile, tCheckSum, ((dwCheckSumLen) * sizeof (WCHAR)), &dwDataRead, NULL) == FALSE)
			|| (WriteFile(hXMLFile, TEXT("\" progresstotalhdworld=\""), (DWORD)(_tcslen(TEXT("\" progresstotalhdworld=\"")) * sizeof (TCHAR)), &dwDataRead, NULL) == FALSE)
			|| (WriteFile(hXMLFile, tProgressTotalHDWord, ((dwProgressTotalHDWordLen) * sizeof (WCHAR)), &dwDataRead, NULL) == FALSE)
			|| (WriteFile(hXMLFile, TEXT("\" progresstotalldworld=\""), (DWORD)(_tcslen(TEXT("\" progresstotalldworld=\"")) * sizeof (TCHAR)), &dwDataRead, NULL) == FALSE)
			|| (WriteFile(hXMLFile, tProgressTotalLDWord, ((dwProgressTotalLDWordLen) * sizeof (WCHAR)), &dwDataRead, NULL) == FALSE)
			|| (WriteFile(hXMLFile, TEXT("\"/>\r\n"), (DWORD)(_tcslen(TEXT("\"/>\r\n")) * sizeof (TCHAR)), &dwDataRead, NULL) == FALSE))
			goto writerror;
	}
	if (pAasData->dwNumberOfUnknwownBlock > 0)
	{
		for (DWORD i = 0; i < pAasData->dwNumberOfUnknwownBlock; ++i)
		{
			TCHAR tOpCodeNumber[11], tArgNumber[11];
			DWORD dwOpCodeNumberLen = 0, dwArgNumberLen = 0, dwOpCode = 0, dwArgNum = 0;

			pAasBlockUnkwnown = pAasData->sBlockUnkwnown[i];
			dwOpCode += pAasBlockUnkwnown->bOpcodeNumber;
			dwArgNum += pAasBlockUnkwnown->bArgumentNumber;
			dwOpCodeNumberLen = _stprintf_s(tOpCodeNumber, 11, TEXT("%d"), dwOpCode);
			dwArgNumberLen = _stprintf_s(tArgNumber, 11, TEXT("%d"), dwArgNum);

			if ((WriteFile(hXMLFile, TEXT("\t<UnknwownBlock opcodenumber=\""), (DWORD)(_tcslen(TEXT("\t<UnknwownBlock opcodenumber=\"")) * sizeof (TCHAR)), &dwDataRead, NULL) == FALSE)
				|| (WriteFile(hXMLFile, tOpCodeNumber, (DWORD)(dwOpCodeNumberLen * sizeof (WCHAR)), &dwDataRead, NULL) == FALSE)
				|| (WriteFile(hXMLFile, TEXT("\" argumentnumber=\""), (DWORD)(_tcslen(TEXT("\" argumentnumber=\"")) * sizeof (TCHAR)), &dwDataRead, NULL) == FALSE)
				|| (WriteFile(hXMLFile, tArgNumber, (DWORD)(dwArgNumberLen * sizeof (WCHAR)), &dwDataRead, NULL) == FALSE)
				|| (WriteFile(hXMLFile, TEXT("\">\r\n"), (DWORD)(_tcslen(TEXT("\">\r\n")) * sizeof (TCHAR)), &dwDataRead, NULL) == FALSE))
			goto writerror;

			for (BYTE j = 0; j < pAasBlockUnkwnown->bArgumentNumber; ++j)
			{
				TCHAR tDataType[11], tDataLen[11];
				DWORD dwDataTypeLen = 0, dwDataLenLen = 0;
				PTCHAR tData = GetBase64FromByte(pAasBlockUnkwnown->sDataUnkwnown[j].pbData, pAasBlockUnkwnown->sDataUnkwnown[j].wDataLen);

				dwDataTypeLen = _stprintf_s(tDataType, 11, TEXT("%d"), pAasBlockUnkwnown->sDataUnkwnown[j].wDataType);
				dwDataLenLen = _stprintf_s(tDataLen, 11, TEXT("%d"), pAasBlockUnkwnown->sDataUnkwnown[j].wDataLen);

				if ((WriteFile(hXMLFile, TEXT("\t\t<UnknwownArg datatype=\""), (DWORD)(_tcslen(TEXT("\t\t<UnknwownArg datatype=\"")) * sizeof (TCHAR)), &dwDataRead, NULL) == FALSE)
					|| (WriteFile(hXMLFile, tDataType, ((dwDataTypeLen) * sizeof (WCHAR)), &dwDataRead, NULL) == FALSE)
					|| (WriteFile(hXMLFile, TEXT("\" datalength=\""), (DWORD)(_tcslen(TEXT("\" datalength=\"")) * sizeof (TCHAR)), &dwDataRead, NULL) == FALSE)
					|| (WriteFile(hXMLFile, tDataLen, ((dwDataLenLen) * sizeof (WCHAR)), &dwDataRead, NULL) == FALSE)
					|| (WriteFile(hXMLFile, TEXT("\" data=\""), (DWORD)(_tcslen(TEXT("\" data=\"")) * sizeof (TCHAR)), &dwDataRead, NULL) == FALSE)
					|| (WriteFile(hXMLFile, tData, (DWORD)(_tcslen(tData) * sizeof (WCHAR)), &dwDataRead, NULL) == FALSE)
					|| (WriteFile(hXMLFile, TEXT("\"/>\r\n"), (DWORD)(_tcslen(TEXT("\"/>\r\n")) * sizeof (TCHAR)), &dwDataRead, NULL) == FALSE))
					goto writerror;

				HeapFree(hCrawlerHeap, NULL, tData);
			}
			if (WriteFile(hXMLFile, TEXT("\t</UnknwownBlock>\r\n"), (DWORD)(_tcslen(TEXT("\t</UnknwownBlock>\r\n")) * sizeof (TCHAR)), &dwDataRead, NULL) == FALSE)
				goto writerror;
		}
	}

	CloseHandle(hXMLFile);
	return TRUE;

writerror:
	DEBUG_LOG(D_WARNING, "Unable to write XML DATA.\r\nExiting now...");
	DoExit(D_ERROR);
	return FALSE;
}

BOOL PrintCSVData(_In_ PAAS_FILE_DATA pAasData)
{
	DWORD dwDataRead = 0;
	HANDLE hCSVFile = INVALID_HANDLE_VALUE;
	GPO_FILTER_TARGET dwTarget = GPO_FILTER_UNKNOWN;
	PTCHAR tOutputDirectoryAasFile = OUTPUT_DIRECTORY_AAS_FILE;
	PAAS_BLOCK_HEADER pAasBlockHeader = NULL;
	PAAS_BLOCK_PRODUCT_INFO pAasBlockProductInfo = NULL;
	PAAS_BLOCK_SOURCE_LIST_PUBLISH pAasBlockSourceListPublish = NULL;
	PAAS_BLOCK_PRODUCT_PUBLISH pAasBlockProductPublish = NULL;
	PAAS_BLOCK_END pAasBlockEnd = NULL;
	PAAS_BLOCK_UNKNOWN pAasBlockUnkwnown = NULL;

	// Determine if we need to create user or computer file in case of '[Machine||User]' 
	dwTarget = GetTargetGPO(pAasData->tFilePath);
	if (dwTarget == GPO_FILTER_TARGET_MACHINE)
		tOutputDirectoryAasFile = TEXT(".\\Machine\\");
	else if (dwTarget == GPO_FILTER_TARGET_USER)
		tOutputDirectoryAasFile = TEXT(".\\User\\");
	hCSVFile = GetFileHandle(OUTPUT_FILE_CSV, tOutputDirectoryAasFile, OUTPUT_NAME_AAS_FILE);

	if (!pAasData || !(pAasData->tFilePath))
	{
		DEBUG_LOG(D_WARNING, "PINF_FILE_DATA invalid for INF file.\r\n");
		DoExit(D_WARNING);
	}

	if (hCSVFile == INVALID_HANDLE_VALUE)
	{
		DEBUG_LOG(D_WARNING, "Handle to hCSVFile is invalid.\r\nExiting now...");
		DoExit(D_ERROR);
	}

	if ((WriteFile(hCSVFile, pAasData->tFilePath, (DWORD)(_tcslen(pAasData->tFilePath) * sizeof (WCHAR)), &dwDataRead, NULL) == FALSE)
		|| (WriteFile(hCSVFile, TEXT(";"), (DWORD)(_tcslen(TEXT(";")) * sizeof (TCHAR)), &dwDataRead, NULL) == FALSE))
		goto writerror;	

	if (pAasData->pAasHeader)
	{
		
		TCHAR tSignature[11], tVersion[11], tDosTimeStamp[11], tLangID[11],  tPlatform[11],  tScriptType[11], tScriptMajorVersion[11], tScriptMinorversion[11], tScriptAttributes[11];
		DWORD dwSignatureLen = 0, dwVersionLen = 0, dwDosTimeStampLen = 0, dwLangIDLen = 0,  dwPlatformLen = 0,  dwScriptTypeLen = 0, dwScriptMajorVersionLen = 0, dwScriptMinorversionLen = 0, dwScriptAttributesLen = 0;
		
		pAasBlockHeader = pAasData->pAasHeader;
		dwSignatureLen = _stprintf_s(tSignature, 11, TEXT("%d"), *(pAasBlockHeader->pdwSignature));
		dwVersionLen = _stprintf_s(tVersion, 11, TEXT("%d"), *(pAasBlockHeader->pdwVersion));
		dwDosTimeStampLen = _stprintf_s(tDosTimeStamp, 11, TEXT("%d"), *(pAasBlockHeader->pdwDosTimeStamp));
		dwLangIDLen = _stprintf_s(tLangID, 11, TEXT("%d"), *(pAasBlockHeader->pdwLangID));
		dwPlatformLen = _stprintf_s(tPlatform, 11, TEXT("%d"), *(pAasBlockHeader->pdwPlatform));
		dwScriptTypeLen = _stprintf_s(tScriptType, 11, TEXT("%d"), *(pAasBlockHeader->pdwScriptType));
		dwScriptMajorVersionLen = _stprintf_s(tScriptMajorVersion, 11, TEXT("%d"), *(pAasBlockHeader->pdwScriptMajorVersion));
		dwScriptMinorversionLen = _stprintf_s(tScriptMinorversion, 11, TEXT("%d"), *(pAasBlockHeader->pdwScriptMinorVersion));
		dwScriptAttributesLen = _stprintf_s(tScriptAttributes, 11, TEXT("%d"), *(pAasBlockHeader->pdwScriptAttributes));
	
		if ((WriteFile(hCSVFile, tSignature, ((dwSignatureLen) * sizeof (WCHAR)), &dwDataRead, NULL) == FALSE)
			|| (WriteFile(hCSVFile, TEXT(","), (DWORD)(_tcslen(TEXT(",")) * sizeof (TCHAR)), &dwDataRead, NULL) == FALSE)
			|| (WriteFile(hCSVFile, tVersion, ((dwVersionLen) * sizeof (WCHAR)), &dwDataRead, NULL) == FALSE)
			|| (WriteFile(hCSVFile, TEXT(","), (DWORD)(_tcslen(TEXT(",")) * sizeof (TCHAR)), &dwDataRead, NULL) == FALSE)
			|| (WriteFile(hCSVFile, tDosTimeStamp, ((dwDosTimeStampLen) * sizeof (WCHAR)), &dwDataRead, NULL) == FALSE)
			|| (WriteFile(hCSVFile, TEXT(","), (DWORD)(_tcslen(TEXT(",")) * sizeof (TCHAR)), &dwDataRead, NULL) == FALSE)
			|| (WriteFile(hCSVFile, tLangID, ((dwLangIDLen) * sizeof (WCHAR)), &dwDataRead, NULL) == FALSE)
			|| (WriteFile(hCSVFile, TEXT(","), (DWORD)(_tcslen(TEXT(",")) * sizeof (TCHAR)), &dwDataRead, NULL) == FALSE)
			|| (WriteFile(hCSVFile, tPlatform, ((dwPlatformLen) * sizeof (WCHAR)), &dwDataRead, NULL) == FALSE)
			|| (WriteFile(hCSVFile, TEXT(","), (DWORD)(_tcslen(TEXT(",")) * sizeof (TCHAR)), &dwDataRead, NULL) == FALSE)
			|| (WriteFile(hCSVFile, tScriptType, ((dwScriptTypeLen) * sizeof (WCHAR)), &dwDataRead, NULL) == FALSE)
			|| (WriteFile(hCSVFile, TEXT(","), (DWORD)(_tcslen(TEXT(",")) * sizeof (TCHAR)), &dwDataRead, NULL) == FALSE)
			|| (WriteFile(hCSVFile, tScriptMajorVersion, ((dwScriptMajorVersionLen) * sizeof (WCHAR)), &dwDataRead, NULL) == FALSE)
			|| (WriteFile(hCSVFile, TEXT(","), (DWORD)(_tcslen(TEXT(",")) * sizeof (TCHAR)), &dwDataRead, NULL) == FALSE)
			|| (WriteFile(hCSVFile, tScriptMinorversion, ((dwScriptMinorversionLen) * sizeof (WCHAR)), &dwDataRead, NULL) == FALSE)
			|| (WriteFile(hCSVFile, TEXT(","), (DWORD)(_tcslen(TEXT(",")) * sizeof (TCHAR)), &dwDataRead, NULL) == FALSE)
			|| (WriteFile(hCSVFile, tScriptAttributes, ((dwScriptAttributesLen) * sizeof (WCHAR)), &dwDataRead, NULL) == FALSE)
			|| (WriteFile(hCSVFile, TEXT(";"), (DWORD)(_tcslen(TEXT(";")) * sizeof (TCHAR)), &dwDataRead, NULL) == FALSE))
			goto writerror;		
	}
	if (pAasData->pAasProductInfo)
	{
		TCHAR tLanguage[11], tVersion[11], tAssignement[11], tObsoleteArg[11], tInstanceType[11], tLUASettings[11], tRemoteURTInstalls[11], tProductDeploymentFlags[11];
		DWORD dwLanguageLen = 0, dwVersionLen = 0, dwAssignementLen = 0, dwObsoleteArgLen = 0, dwInstanceTypeLen = 0, dwLUASettingsLen = 0, dwRemoteURTInstallsLen = 0, dwProductDeploymentFlagsLen = 0;

		pAasBlockProductInfo = pAasData->pAasProductInfo;
		dwLanguageLen = _stprintf_s(tLanguage, 11, TEXT("%d"), *(pAasBlockProductInfo->pdwLanguage));
		dwVersionLen = _stprintf_s(tVersion, 11, TEXT("%d"), *(pAasBlockProductInfo->pdwVersion));
		dwAssignementLen = _stprintf_s(tAssignement, 11, TEXT("%d"), *(pAasBlockProductInfo->pdwAssignment));
		dwObsoleteArgLen = _stprintf_s(tObsoleteArg, 11, TEXT("%d"), *(pAasBlockProductInfo->pdwObsoleteArg));
		dwInstanceTypeLen = _stprintf_s(tInstanceType, 11, TEXT("%d"), *(pAasBlockProductInfo->pdwInstanceType));
		dwLUASettingsLen = _stprintf_s(tLUASettings, 11, TEXT("%d"), *(pAasBlockProductInfo->pdwLUASetting));
		dwRemoteURTInstallsLen = _stprintf_s(tRemoteURTInstalls, 11, TEXT("%d"), *(pAasBlockProductInfo->pdwRemoteURTInstalls));
		dwProductDeploymentFlagsLen = _stprintf_s(tProductDeploymentFlags, 11, TEXT("%d"), *(pAasBlockProductInfo->pdwProductDeploymentFlags));

		pAasBlockProductInfo = pAasData->pAasProductInfo;

		if ((WriteFile(hCSVFile, pAasBlockProductInfo->pwProductKey, (DWORD)(_tcslen(pAasBlockProductInfo->pwProductKey) * sizeof (WCHAR)), &dwDataRead, NULL) == FALSE)
			|| (WriteFile(hCSVFile, TEXT(","), (DWORD)(_tcslen(TEXT(",")) * sizeof (TCHAR)), &dwDataRead, NULL) == FALSE))
			goto writerror;

		if (pAasBlockProductInfo->isProductNameUNICODE)
		{
			if ((WriteFile(hCSVFile, pAasBlockProductInfo->pwProductName, (DWORD)(_tcslen(pAasBlockProductInfo->pwProductName) * sizeof (WCHAR)), &dwDataRead, NULL) == FALSE)
				|| (WriteFile(hCSVFile, TEXT(","), (DWORD)(_tcslen(TEXT(",")) * sizeof (TCHAR)), &dwDataRead, NULL) == FALSE))
			goto writerror;
		}
		else
		{
			if ((WriteFile(hCSVFile, pAasBlockProductInfo->pwProductName, (DWORD)(_tcslen(pAasBlockProductInfo->pwProductName) * sizeof (WCHAR)), &dwDataRead, NULL) == FALSE)
				|| (WriteFile(hCSVFile, TEXT(","), (DWORD)(_tcslen(TEXT(",")) * sizeof (TCHAR)), &dwDataRead, NULL) == FALSE))
			goto writerror;
		}
		if (pAasBlockProductInfo->isPackageNameUNICODE)
		{
			if ((WriteFile(hCSVFile, pAasBlockProductInfo->pwPackageName, (DWORD)(_tcslen(pAasBlockProductInfo->pwPackageName) * sizeof (WCHAR)), &dwDataRead, NULL) == FALSE)
				|| (WriteFile(hCSVFile, TEXT(","), (DWORD)(_tcslen(TEXT(",")) * sizeof (TCHAR)), &dwDataRead, NULL) == FALSE))
			goto writerror;
		}
		else
		{
			if ((WriteFile(hCSVFile, pAasBlockProductInfo->pwPackageName, (DWORD)(_tcslen(pAasBlockProductInfo->pwPackageName) * sizeof (WCHAR)), &dwDataRead, NULL) == FALSE)
				|| (WriteFile(hCSVFile, TEXT(","), (DWORD)(_tcslen(TEXT(",")) * sizeof (TCHAR)), &dwDataRead, NULL) == FALSE))
			goto writerror;
		}

		if ((WriteFile(hCSVFile, tLanguage, ((dwLanguageLen) * sizeof (WCHAR)), &dwDataRead, NULL) == FALSE)
			|| (WriteFile(hCSVFile, TEXT(","), (DWORD)(_tcslen(TEXT(",")) * sizeof (TCHAR)), &dwDataRead, NULL) == FALSE)
			|| (WriteFile(hCSVFile, tVersion, ((dwVersionLen) * sizeof (WCHAR)), &dwDataRead, NULL) == FALSE)
			|| (WriteFile(hCSVFile, TEXT(","), (DWORD)(_tcslen(TEXT(",")) * sizeof (TCHAR)), &dwDataRead, NULL) == FALSE)
			|| (WriteFile(hCSVFile, tAssignement, ((dwAssignementLen) * sizeof (WCHAR)), &dwDataRead, NULL) == FALSE)
			|| (WriteFile(hCSVFile, TEXT(","), (DWORD)(_tcslen(TEXT(",")) * sizeof (TCHAR)), &dwDataRead, NULL) == FALSE)
			|| (WriteFile(hCSVFile, tObsoleteArg, ((dwObsoleteArgLen) * sizeof (WCHAR)), &dwDataRead, NULL) == FALSE)
			|| (WriteFile(hCSVFile, TEXT(","), (DWORD)(_tcslen(TEXT(",")) * sizeof (TCHAR)), &dwDataRead, NULL) == FALSE)
			|| (WriteFile(hCSVFile, pAasBlockProductInfo->pwProductIcon, (DWORD)(_tcslen(pAasBlockProductInfo->pwProductIcon) * sizeof (WCHAR)), &dwDataRead, NULL) == FALSE)
			|| (WriteFile(hCSVFile, TEXT(","), (DWORD)(_tcslen(TEXT(",")) * sizeof (TCHAR)), &dwDataRead, NULL) == FALSE))
			goto writerror;

		if (pAasBlockProductInfo->isPackageMediaPathUNICODE)
		{
			if ((WriteFile(hCSVFile, pAasBlockProductInfo->pwPackageMediaPath, (DWORD)(_tcslen(pAasBlockProductInfo->pwPackageMediaPath) * sizeof (WCHAR)), &dwDataRead, NULL) == FALSE)
				|| (WriteFile(hCSVFile, TEXT(","), (DWORD)(_tcslen(TEXT(",")) * sizeof (TCHAR)), &dwDataRead, NULL) == FALSE))
			goto writerror;
		}
		else
		{
			if ((WriteFile(hCSVFile, pAasBlockProductInfo->pwPackageMediaPath, (DWORD)(_tcslen(pAasBlockProductInfo->pwPackageMediaPath) * sizeof (WCHAR)), &dwDataRead, NULL) == FALSE)
				|| (WriteFile(hCSVFile, TEXT(","), (DWORD)(_tcslen(TEXT(",")) * sizeof (TCHAR)), &dwDataRead, NULL) == FALSE))
			goto writerror;
		}
	
		if ((WriteFile(hCSVFile, pAasBlockProductInfo->pwPackageCode, (DWORD)(_tcslen(pAasBlockProductInfo->pwPackageCode) * sizeof (WCHAR)), &dwDataRead, NULL) == FALSE)
			|| (WriteFile(hCSVFile, TEXT(","), (DWORD)(_tcslen(TEXT(",")) * sizeof (TCHAR)), &dwDataRead, NULL) == FALSE)
			|| (WriteFile(hCSVFile, tInstanceType, ((dwInstanceTypeLen) * sizeof (WCHAR)), &dwDataRead, NULL) == FALSE)
			|| (WriteFile(hCSVFile, TEXT(","), (DWORD)(_tcslen(TEXT(",")) * sizeof (TCHAR)), &dwDataRead, NULL) == FALSE)
			|| (WriteFile(hCSVFile, tLUASettings, ((dwLUASettingsLen) * sizeof (WCHAR)), &dwDataRead, NULL) == FALSE)
			|| (WriteFile(hCSVFile, TEXT(","), (DWORD)(_tcslen(TEXT(",")) * sizeof (TCHAR)), &dwDataRead, NULL) == FALSE)
			|| (WriteFile(hCSVFile, tRemoteURTInstalls, ((dwRemoteURTInstallsLen) * sizeof (WCHAR)), &dwDataRead, NULL) == FALSE)
			|| (WriteFile(hCSVFile, TEXT(","), (DWORD)(_tcslen(TEXT(",")) * sizeof (TCHAR)), &dwDataRead, NULL) == FALSE)
			|| (WriteFile(hCSVFile, tProductDeploymentFlags, ((dwProductDeploymentFlagsLen) * sizeof (WCHAR)), &dwDataRead, NULL) == FALSE)
			|| (WriteFile(hCSVFile, TEXT(";"), (DWORD)(_tcslen(TEXT(";")) * sizeof (TCHAR)), &dwDataRead, NULL) == FALSE))
			goto writerror;	
	}
	if (pAasData->pAasSourceListPublish)
	{
		TCHAR tNumberOfDisks[11], tDiskID[11];
		DWORD dwNumberOfDisksLen = 0, dwDiskIDLen = 0;
		pAasBlockSourceListPublish = pAasData->pAasSourceListPublish;

		if ((WriteFile(hCSVFile, pAasBlockSourceListPublish->pwPatchCode, (DWORD)(_tcslen(pAasBlockSourceListPublish->pwPatchCode) * sizeof (WCHAR)), &dwDataRead, NULL) == FALSE)
			|| (WriteFile(hCSVFile, TEXT(","), (DWORD)(_tcslen(TEXT(",")) * sizeof (TCHAR)), &dwDataRead, NULL) == FALSE)
			|| (WriteFile(hCSVFile, pAasBlockSourceListPublish->pwPatchPackageName, (DWORD)(_tcslen(pAasBlockSourceListPublish->pwPatchPackageName) * sizeof (WCHAR)), &dwDataRead, NULL) == FALSE)
			|| (WriteFile(hCSVFile, TEXT(","), (DWORD)(_tcslen(TEXT(",")) * sizeof (TCHAR)), &dwDataRead, NULL) == FALSE))
			goto writerror;

		if (pAasBlockSourceListPublish->isDiskPromptTemplateUNICODE)
		{
			if ((WriteFile(hCSVFile, pAasBlockSourceListPublish->pwDiskPromptTemplate, (DWORD)(_tcslen(pAasBlockSourceListPublish->pwDiskPromptTemplate) * sizeof (WCHAR)), &dwDataRead, NULL) == FALSE)
				|| (WriteFile(hCSVFile, TEXT(","), (DWORD)(_tcslen(TEXT(",")) * sizeof (TCHAR)), &dwDataRead, NULL) == FALSE))
			goto writerror;
		}
		else
		{
			if ((WriteFile(hCSVFile, pAasBlockSourceListPublish->pwDiskPromptTemplate, (DWORD)(_tcslen(pAasBlockSourceListPublish->pwDiskPromptTemplate) * sizeof (WCHAR)), &dwDataRead, NULL) == FALSE)
				|| (WriteFile(hCSVFile, TEXT(","), (DWORD)(_tcslen(TEXT(",")) * sizeof (TCHAR)), &dwDataRead, NULL) == FALSE))
			goto writerror;
		}

		if (pAasBlockSourceListPublish->isPackagePathUNICODE)
		{
			if ((WriteFile(hCSVFile, pAasBlockSourceListPublish->pwPackagePath, (DWORD)(_tcslen(pAasBlockSourceListPublish->pwPackagePath) * sizeof (WCHAR)), &dwDataRead, NULL) == FALSE)
				|| (WriteFile(hCSVFile, TEXT(","), (DWORD)(_tcslen(TEXT(",")) * sizeof (TCHAR)), &dwDataRead, NULL) == FALSE))
			goto writerror;
		}
		else
		{
			if ((WriteFile(hCSVFile, pAasBlockSourceListPublish->pwPackagePath, (DWORD)(_tcslen(pAasBlockSourceListPublish->pwPackagePath) * sizeof (WCHAR)), &dwDataRead, NULL) == FALSE)
				|| (WriteFile(hCSVFile, TEXT(","), (DWORD)(_tcslen(TEXT(",")) * sizeof (TCHAR)), &dwDataRead, NULL) == FALSE))
			goto writerror;
		}

		if (pAasBlockSourceListPublish->isLaunchPathUNICODE)
		{
			if ((WriteFile(hCSVFile, pAasBlockSourceListPublish->pwLaunchPath, (DWORD)(_tcslen(pAasBlockSourceListPublish->pwLaunchPath) * sizeof (WCHAR)), &dwDataRead, NULL) == FALSE)
				|| (WriteFile(hCSVFile, TEXT(","), (DWORD)(_tcslen(TEXT(",")) * sizeof (TCHAR)), &dwDataRead, NULL) == FALSE))
			goto writerror;
		}
		else
		{
			if ((WriteFile(hCSVFile, pAasBlockSourceListPublish->pwLaunchPath, (DWORD)(_tcslen(pAasBlockSourceListPublish->pwLaunchPath) * sizeof (WCHAR)), &dwDataRead, NULL) == FALSE)
				|| (WriteFile(hCSVFile, TEXT(","), (DWORD)(_tcslen(TEXT(",")) * sizeof (TCHAR)), &dwDataRead, NULL) == FALSE))
			goto writerror;
		}		

		dwNumberOfDisksLen = _stprintf_s(tNumberOfDisks, 11, TEXT("%d"), *(pAasBlockSourceListPublish->pdwNumberOfDisks));
		if ((WriteFile(hCSVFile, tNumberOfDisks, ((dwNumberOfDisksLen) * sizeof (WCHAR)), &dwDataRead, NULL) == FALSE)
			|| (WriteFile(hCSVFile, TEXT(","), (DWORD)(_tcslen(TEXT(",")) * sizeof (TCHAR)), &dwDataRead, NULL) == FALSE))
			goto writerror;

		for (DWORD i = 0; i < *(pAasBlockSourceListPublish->pdwNumberOfDisks); ++i)
		{
			dwDiskIDLen = _stprintf_s(tDiskID, 11, TEXT("%d"), *(pAasBlockSourceListPublish->sDisks[i].pdwDiskId));
			if ((WriteFile(hCSVFile, TEXT("{"), (DWORD)(_tcslen(TEXT("{")) * sizeof (TCHAR)), &dwDataRead, NULL) == FALSE)
				|| (WriteFile(hCSVFile, tDiskID, ((dwDiskIDLen) * sizeof (WCHAR)), &dwDataRead, NULL) == FALSE)
				|| (WriteFile(hCSVFile, TEXT(","), (DWORD)(_tcslen(TEXT(",")) * sizeof (TCHAR)), &dwDataRead, NULL) == FALSE))
				goto writerror;

			if (pAasBlockSourceListPublish->sDisks[i].isVolumeNameUNICODE)
			{
				if ((WriteFile(hCSVFile, pAasBlockSourceListPublish->sDisks[i].pwVolumeName, (DWORD)(_tcslen(pAasBlockSourceListPublish->sDisks[i].pwVolumeName) * sizeof (WCHAR)), &dwDataRead, NULL) == FALSE)
					|| (WriteFile(hCSVFile, TEXT(","), (DWORD)(_tcslen(TEXT(",")) * sizeof (TCHAR)), &dwDataRead, NULL) == FALSE))
				goto writerror;
			}
			else
			{
				if ((WriteFile(hCSVFile, pAasBlockSourceListPublish->sDisks[i].pwVolumeName, (DWORD)(_tcslen(pAasBlockSourceListPublish->sDisks[i].pwVolumeName) * sizeof (WCHAR)), &dwDataRead, NULL) == FALSE)
					|| (WriteFile(hCSVFile, TEXT(","), (DWORD)(_tcslen(TEXT(",")) * sizeof (TCHAR)), &dwDataRead, NULL) == FALSE))
				goto writerror;
			}	

			if (pAasBlockSourceListPublish->sDisks[i].isDiskPromptUNICODE)
			{
				if ((WriteFile(hCSVFile, pAasBlockSourceListPublish->sDisks[i].pwDiskPrompt, (DWORD)(_tcslen(pAasBlockSourceListPublish->sDisks[i].pwDiskPrompt) * sizeof (WCHAR)), &dwDataRead, NULL) == FALSE))
				goto writerror;
			}
			else
			{
				if ((WriteFile(hCSVFile, pAasBlockSourceListPublish->sDisks[i].pwDiskPrompt, (DWORD)(_tcslen(pAasBlockSourceListPublish->sDisks[i].pwDiskPrompt) * sizeof (WCHAR)), &dwDataRead, NULL) == FALSE))
				goto writerror;
			}
			if ((WriteFile(hCSVFile, TEXT("}"), (DWORD)(_tcslen(TEXT("}")) * sizeof (TCHAR)), &dwDataRead, NULL) == FALSE))
				goto writerror;
		}
		if ((WriteFile(hCSVFile, TEXT(";"), (DWORD)(_tcslen(TEXT(";")) * sizeof (TCHAR)), &dwDataRead, NULL) == FALSE))
			goto writerror;
	}
	if (pAasData->pAasProductPublish)
	{
		pAasBlockProductPublish = pAasData->pAasProductPublish;
		if ((WriteFile(hCSVFile, pAasBlockProductPublish->pwProductPublish, (DWORD)(_tcslen(pAasBlockProductPublish->pwProductPublish) * sizeof (WCHAR)), &dwDataRead, NULL) == FALSE)
			|| (WriteFile(hCSVFile, TEXT(";"), (DWORD)(_tcslen(TEXT(";")) * sizeof (TCHAR)), &dwDataRead, NULL) == FALSE))
			goto writerror;

	}
	if (pAasData->pAasEnd)
	{
		TCHAR tCheckSum[11], tProgressTotalHDWord[11], tProgressTotalLDWord[11];
		DWORD dwCheckSumLen = 0, dwProgressTotalHDWordLen = 0, dwProgressTotalLDWordLen = 0;
		
		pAasBlockEnd = pAasData->pAasEnd;
		dwCheckSumLen = _stprintf_s(tCheckSum, 11, TEXT("%d"), *(pAasBlockEnd->pdwChecksum));
		dwProgressTotalHDWordLen = _stprintf_s(tProgressTotalHDWord, 11, TEXT("%d"), *(pAasBlockEnd->pdwProgressTotalHDWord));
		dwProgressTotalLDWordLen = _stprintf_s(tProgressTotalLDWord, 11, TEXT("%d"), *(pAasBlockEnd->pdwProgressTotalLDWord));

		if ((WriteFile(hCSVFile, tCheckSum, ((dwCheckSumLen) * sizeof (WCHAR)), &dwDataRead, NULL) == FALSE)
			|| (WriteFile(hCSVFile, TEXT(","), (DWORD)(_tcslen(TEXT(",")) * sizeof (TCHAR)), &dwDataRead, NULL) == FALSE)
			|| (WriteFile(hCSVFile, tProgressTotalHDWord, ((dwProgressTotalHDWordLen) * sizeof (WCHAR)), &dwDataRead, NULL) == FALSE)
			|| (WriteFile(hCSVFile, TEXT(","), (DWORD)(_tcslen(TEXT(",")) * sizeof (TCHAR)), &dwDataRead, NULL) == FALSE)
			|| (WriteFile(hCSVFile, tProgressTotalLDWord, ((dwProgressTotalLDWordLen) * sizeof (WCHAR)), &dwDataRead, NULL) == FALSE)
			|| (WriteFile(hCSVFile, TEXT(";"), (DWORD)(_tcslen(TEXT(";")) * sizeof (TCHAR)), &dwDataRead, NULL) == FALSE))
			goto writerror;
	}
	if (pAasData->dwNumberOfUnknwownBlock > 0)
	{
		for (DWORD i = 0; i < pAasData->dwNumberOfUnknwownBlock; ++i)
		{
			TCHAR tOpCodeNumber[11], tArgNumber[11];
			DWORD dwOpCodeNumberLen = 0, dwArgNumberLen = 0, dwOpCode = 0, dwArgNum = 0;

			pAasBlockUnkwnown = pAasData->sBlockUnkwnown[i];
			dwOpCode += pAasBlockUnkwnown->bOpcodeNumber;
			dwArgNum += pAasBlockUnkwnown->bArgumentNumber;
			dwOpCodeNumberLen = _stprintf_s(tOpCodeNumber, 11, TEXT("%d"), dwOpCode);
			dwArgNumberLen = _stprintf_s(tArgNumber, 11, TEXT("%d"), dwArgNum);


			if ((WriteFile(hCSVFile, tOpCodeNumber, (DWORD)(dwOpCodeNumberLen * sizeof (WCHAR)), &dwDataRead, NULL) == FALSE)
				|| (WriteFile(hCSVFile, TEXT(","), (DWORD)(_tcslen(TEXT(",")) * sizeof (TCHAR)), &dwDataRead, NULL) == FALSE)
				|| (WriteFile(hCSVFile, tArgNumber, (DWORD)(dwArgNumberLen * sizeof (WCHAR)), &dwDataRead, NULL) == FALSE)
				|| (WriteFile(hCSVFile, TEXT(","), (DWORD)(_tcslen(TEXT(",")) * sizeof (TCHAR)), &dwDataRead, NULL) == FALSE))
			goto writerror;

			for (BYTE j = 0; j < pAasBlockUnkwnown->bArgumentNumber; ++j)
			{
				TCHAR tDataType[11], tDataLen[11];
				DWORD dwDataTypeLen = 0, dwDataLenLen = 0;
				PTCHAR tData = GetBase64FromByte(pAasBlockUnkwnown->sDataUnkwnown[j].pbData, pAasBlockUnkwnown->sDataUnkwnown[j].wDataLen);

				dwDataTypeLen = _stprintf_s(tDataType, 11, TEXT("%d"), pAasBlockUnkwnown->sDataUnkwnown[j].wDataType);
				dwDataLenLen = _stprintf_s(tDataLen, 11, TEXT("%d"), pAasBlockUnkwnown->sDataUnkwnown[j].wDataLen);

				if ((WriteFile(hCSVFile, TEXT("{"), (DWORD)(_tcslen(TEXT("{")) * sizeof (TCHAR)), &dwDataRead, NULL) == FALSE)
					|| (WriteFile(hCSVFile, tDataType, ((dwDataTypeLen) * sizeof (WCHAR)), &dwDataRead, NULL) == FALSE)
					|| (WriteFile(hCSVFile, TEXT(","), (DWORD)(_tcslen(TEXT(",")) * sizeof (TCHAR)), &dwDataRead, NULL) == FALSE)
					|| (WriteFile(hCSVFile, tDataLen, ((dwDataLenLen) * sizeof (WCHAR)), &dwDataRead, NULL) == FALSE)
					|| (WriteFile(hCSVFile, TEXT(","), (DWORD)(_tcslen(TEXT(",")) * sizeof (TCHAR)), &dwDataRead, NULL) == FALSE)
					|| (WriteFile(hCSVFile, tData, (DWORD) (_tcslen(tData) * sizeof (WCHAR)), &dwDataRead, NULL) == FALSE)
					|| (WriteFile(hCSVFile, TEXT(","), (DWORD)(_tcslen(TEXT(",")) * sizeof (TCHAR)), &dwDataRead, NULL) == FALSE)
					|| (WriteFile(hCSVFile, TEXT("}"), (DWORD)(_tcslen(TEXT("}")) * sizeof (TCHAR)), &dwDataRead, NULL) == FALSE))
					goto writerror;

				HeapFree(hCrawlerHeap, NULL, tData);
			}
		}
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

BOOL PrintSTDOUTData(_In_ PAAS_FILE_DATA pAasData)
{
	DWORD dwDataRead = 0;
	PAAS_BLOCK_HEADER pAasBlockHeader = NULL;
	PAAS_BLOCK_PRODUCT_INFO pAasBlockProductInfo = NULL;
	PAAS_BLOCK_SOURCE_LIST_PUBLISH pAasBlockSourceListPublish = NULL;
	PAAS_BLOCK_PRODUCT_PUBLISH pAasBlockProductPublish = NULL;
	PAAS_BLOCK_END pAasBlockEnd = NULL;
	PAAS_BLOCK_UNKNOWN pAasBlockUnkwnown = NULL;

	if (!pAasData || !(pAasData->tFilePath))
	{
		DEBUG_LOG(D_WARNING, "PAAS_FILE_DATA invalid for INF file.\r\n");
		DoExit(D_WARNING);
	}

	printf("[AAS] File=%ws ", pAasData->tFilePath);
	if (pAasData->pAasHeader)
	{
		pAasBlockHeader = pAasData->pAasHeader;
		printf("Header=(Signature:%d, Version:%d, DosTimeStamp:%d, LangID:%d, Platform:%d, ScriptType:%d, ScriptMajorVersion:%d, ScriptMinorVersion:%d, ScriptAttributes:%d) ", 
			*(pAasBlockHeader->pdwSignature), *(pAasBlockHeader->pdwVersion), *(pAasBlockHeader->pdwDosTimeStamp), *(pAasBlockHeader->pdwLangID), *(pAasBlockHeader->pdwPlatform), *(pAasBlockHeader->pdwScriptType), *(pAasBlockHeader->pdwScriptMajorVersion), *(pAasBlockHeader->pdwScriptMinorVersion), *(pAasBlockHeader->pdwScriptAttributes));
	}
	if (pAasData->pAasProductInfo)
	{
		pAasBlockProductInfo = pAasData->pAasProductInfo;
		
		printf("ProductInfo=(ProductKey:%ws, ", pAasBlockProductInfo->pwProductKey);
		if (pAasBlockProductInfo->isProductNameUNICODE)
			printf("ProductName:%wws, ", pAasBlockProductInfo->pwProductName);
		else
			printf("ProductName:%ws, ", pAasBlockProductInfo->pwProductName);
		if (pAasBlockProductInfo->isPackageNameUNICODE)
			printf("PackageName:%ws, ", pAasBlockProductInfo->pwPackageName);
		else
			printf("PackageName:%ws, ", pAasBlockProductInfo->pwPackageName);

		printf("Language:%d, Version:%d, Assignment:%d, ObsoleteArg:%d, ProductIcon:%ws, ", *(pAasBlockProductInfo->pdwLanguage), *(pAasBlockProductInfo->pdwVersion), *(pAasBlockProductInfo->pdwAssignment), *(pAasBlockProductInfo->pdwObsoleteArg), pAasBlockProductInfo->pwProductIcon);

		if (pAasBlockProductInfo->isPackageMediaPathUNICODE)
			printf("PackageMediaPath:%ws, ", pAasBlockProductInfo->pwPackageMediaPath);
		else
			printf("PackageMediaPath:%ws, ", pAasBlockProductInfo->pwPackageMediaPath);

		printf("PackageCode:%ws, InstanceType:%d, LUASetting:%d, RemoteURTInstalls:%d, ProductDeploymentFlags:%d) ", pAasBlockProductInfo->pwPackageCode, *(pAasBlockProductInfo->pdwInstanceType), *(pAasBlockProductInfo->pdwLUASetting), *(pAasBlockProductInfo->pdwRemoteURTInstalls), *(pAasBlockProductInfo->pdwProductDeploymentFlags));
	}
	if (pAasData->pAasSourceListPublish)
	{
		pAasBlockSourceListPublish = pAasData->pAasSourceListPublish;
		
		printf("SourceListPublish=(PatchCode:%ws, PatchPackageName:%ws, ", pAasBlockSourceListPublish->pwPatchCode, pAasBlockSourceListPublish->pwPatchPackageName);
		if (pAasBlockSourceListPublish->isDiskPromptTemplateUNICODE)
			printf("DiskPromptTemplate:%ws, ", pAasBlockSourceListPublish->pwDiskPromptTemplate);
		else
			printf("DiskPromptTemplate:%ws, ", pAasBlockSourceListPublish->pwDiskPromptTemplate);

		if (pAasBlockSourceListPublish->isPackagePathUNICODE)
			printf("PackagePath:%ws, ", pAasBlockSourceListPublish->pwPackagePath);
		else
			printf("PackagePath:%ws, ", pAasBlockSourceListPublish->pwPackagePath);

		if (pAasBlockSourceListPublish->isLaunchPathUNICODE)
			printf("LaunchPath:%ws, ", pAasBlockSourceListPublish->pwLaunchPath);
		else
			printf("LaunchPath:%ws, ", pAasBlockSourceListPublish->pwLaunchPath);
		printf("NumberOfDisks:%d, Disks:", *(pAasBlockSourceListPublish->pdwNumberOfDisks));

		for (DWORD i = 0; i < *(pAasBlockSourceListPublish->pdwNumberOfDisks); ++i)
		{
			printf("{DiskId:%d, ", *(pAasBlockSourceListPublish->sDisks[i].pdwDiskId));		
		
			if (pAasBlockSourceListPublish->sDisks[i].isVolumeNameUNICODE)
				printf("VolumeName:%ws, ", pAasBlockSourceListPublish->sDisks[i].pwVolumeName);
			else
				printf("VolumeName:%ws, ", pAasBlockSourceListPublish->sDisks[i].pwVolumeName);
			if (pAasBlockSourceListPublish->sDisks[i].isDiskPromptUNICODE)
				printf("DiskPrompt:%ws}", pAasBlockSourceListPublish->sDisks[i].pwDiskPrompt);
			else
				printf("DiskPrompt:%ws}", pAasBlockSourceListPublish->sDisks[i].pwDiskPrompt);
		}
		printf(") ");
	}
	if (pAasData->pAasProductPublish)
	{
		pAasBlockProductPublish = pAasData->pAasProductPublish;
		printf("ProductPublish=(ProductPublish:%ws) ", pAasBlockProductPublish->pwProductPublish);
	}
	if (pAasData->pAasEnd)
	{
		pAasBlockEnd = pAasData->pAasEnd;
		printf("End=(Checksum:%d, ProgressTotalHDWord:%d, ProgressTotalLDWord:%d) ", *(pAasBlockEnd->pdwChecksum), *(pAasBlockEnd->pdwProgressTotalHDWord), *(pAasBlockEnd->pdwProgressTotalLDWord));
	}
	if (pAasData->dwNumberOfUnknwownBlock > 0)
	{
		for (DWORD i = 0; i < pAasData->dwNumberOfUnknwownBlock; ++i)
		{
			DWORD dwOpCodeNumberLen = 0, dwArgNumberLen = 0, dwOpCode = 0, dwArgNum = 0;

			pAasBlockUnkwnown = pAasData->sBlockUnkwnown[i];
			dwOpCode += pAasBlockUnkwnown->bOpcodeNumber;
			dwArgNum += pAasBlockUnkwnown->bArgumentNumber;

			printf("UnknownBlock=(OpcodeNumber:%d, ArgumentNumber:%d, DataUnkwnown:", dwOpCode, dwArgNum);

			for (BYTE j = 0; j < pAasBlockUnkwnown->bArgumentNumber; ++j)
			{
				PTCHAR tData = GetBase64FromByte(pAasBlockUnkwnown->sDataUnkwnown[j].pbData, pAasBlockUnkwnown->sDataUnkwnown[j].wDataLen);
				printf("{DataType:%d, DataLen:%d, Data:%ws}", pAasBlockUnkwnown->sDataUnkwnown[j].wDataType, pAasBlockUnkwnown->sDataUnkwnown[j].wDataLen, tData);
				HeapFree(hCrawlerHeap, NULL, tData);
			}
			printf(") ");
		}
	}
	printf("\r\n");

	return TRUE;
}