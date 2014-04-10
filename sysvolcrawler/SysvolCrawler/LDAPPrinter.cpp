/**************************************************
* SysvolCrawler - LDAPPrinter.h
* AUTHOR: Luc Delsalle	
*
* Display or export data extracted from LDAP 
* directory
*	
* ANSSI/COSSI/DTO/BAI - 2014
***************************************************/

#include "LDAPPrinter.h"

BOOL PrintData(_In_ PLDAP_AD_INFOS pLdapADInfos)
{
	BOOL bRes = TRUE;

	if (pLdapADInfos == NULL)
	{
		DEBUG_LOG(D_ERROR, "LDAP_AD_INFOS pointer invalid.\r\nExiting now...");
		DoExit(D_ERROR);
	}

	// Call every printer
	if ((bRes) && (pSyscrwlrOptions->bShouldPrintXML))
		bRes = PrintXMLData(pLdapADInfos);

	if ((bRes) && (pSyscrwlrOptions->bShouldPrintCSV))
		bRes = PrintCSVData(pLdapADInfos);

	if ((bRes) && (pSyscrwlrOptions->bShouldPrintSTDOUT))
		bRes = PrintSTDOUTData(pLdapADInfos);

	return bRes;
}

BOOL PrintLdapDataHeader()
{
	DWORD dwDataRead = 0;

	if (pSyscrwlrOptions->bShouldPrintXML)
	{
		HANDLE hXMLUsersFile = GetFileHandle(OUTPUT_FILE_XML, OUTPUT_DIRECTORY_LDAP_FILE, OUTPUT_NAME_USERS_FILE);
		HANDLE hXMLGroupsFile = GetFileHandle(OUTPUT_FILE_XML, OUTPUT_DIRECTORY_LDAP_FILE, OUTPUT_NAME_GROUPS_FILE);
		HANDLE hXMLOusFile = GetFileHandle(OUTPUT_FILE_XML, OUTPUT_DIRECTORY_LDAP_FILE, OUTPUT_NAME_OUS_FILE);
		HANDLE hXMLGPOsFile = GetFileHandle(OUTPUT_FILE_XML, OUTPUT_DIRECTORY_LDAP_FILE, OUTPUT_NAME_GPOS_FILE);
		LARGE_INTEGER liUsersFileSize, liGroupsFileSize, liOusFileSize, liGposFileSize;

		if ((!GetFileSizeEx(hXMLUsersFile, &liUsersFileSize))
			||(!GetFileSizeEx(hXMLGroupsFile, &liGroupsFileSize))
			||(!GetFileSizeEx(hXMLOusFile, &liOusFileSize))
			||(!GetFileSizeEx(hXMLGPOsFile, &liGposFileSize)))
		{
			DEBUG_LOG(D_WARNING, "Unable to determine file size.\r\nExiting now...");
			DoExit(D_ERROR);
		}

		if ((liUsersFileSize.HighPart == 0) && (liUsersFileSize.LowPart == 0))
		{
			if (WriteFile(hXMLUsersFile, TEXT("<?xml version=\"1.0\"?>\r\n<LDAPUsers>\r\n"), (DWORD)(_tcslen(TEXT("<?xml version=\"1.0\"?>\r\n<LDAPUsers>\r\n")) * sizeof (TCHAR)), &dwDataRead, NULL) == FALSE)
				goto writerror;
		}
		if ((liGroupsFileSize.HighPart == 0) && (liGroupsFileSize.LowPart == 0))
		{
			if (WriteFile(hXMLGroupsFile, TEXT("<?xml version=\"1.0\"?>\r\n<LDAPGroups>\r\n"), (DWORD)(_tcslen(TEXT("<?xml version=\"1.0\"?>\r\n<LDAPGroups>\r\n")) * sizeof (TCHAR)), &dwDataRead, NULL) == FALSE)
				goto writerror;
		}
		if ((liOusFileSize.HighPart == 0) && (liOusFileSize.LowPart == 0))
		{
			if (WriteFile(hXMLOusFile, TEXT("<?xml version=\"1.0\"?>\r\n<LDAPOUs>\r\n"), (DWORD)(_tcslen(TEXT("<?xml version=\"1.0\"?>\r\n<LDAPOUs>\r\n")) * sizeof (TCHAR)), &dwDataRead, NULL) == FALSE)
				goto writerror;
		}
		if ((liGposFileSize.HighPart == 0) && (liGposFileSize.LowPart == 0))
		{
			if (WriteFile(hXMLGPOsFile, TEXT("<?xml version=\"1.0\"?>\r\n<LDAPGPOs>\r\n"), (DWORD)(_tcslen(TEXT("<?xml version=\"1.0\"?>\r\n<LDAPGPOs>\r\n")) * sizeof (TCHAR)), &dwDataRead, NULL) == FALSE)
				goto writerror;
		}

		CloseHandle(hXMLUsersFile);
		CloseHandle(hXMLGroupsFile);
		CloseHandle(hXMLOusFile);
		CloseHandle(hXMLGPOsFile);
	}

	if (pSyscrwlrOptions->bShouldPrintCSV)
	{
		HANDLE hCSVUsersFile = GetFileHandle(OUTPUT_FILE_CSV, OUTPUT_DIRECTORY_LDAP_FILE, OUTPUT_NAME_USERS_FILE);
		HANDLE hCSVGroupsFile = GetFileHandle(OUTPUT_FILE_CSV, OUTPUT_DIRECTORY_LDAP_FILE, OUTPUT_NAME_GROUPS_FILE);
		HANDLE hCSVOusFile = GetFileHandle(OUTPUT_FILE_CSV, OUTPUT_DIRECTORY_LDAP_FILE, OUTPUT_NAME_OUS_FILE);
		HANDLE hCSVGPOsFile = GetFileHandle(OUTPUT_FILE_CSV, OUTPUT_DIRECTORY_LDAP_FILE, OUTPUT_NAME_GPOS_FILE);
		LARGE_INTEGER liUsersFileSize, liGroupsFileSize, liOusFileSize, liGposFileSize;

		if ((!GetFileSizeEx(hCSVUsersFile, &liUsersFileSize))
			||(!GetFileSizeEx(hCSVGroupsFile, &liGroupsFileSize))
			||(!GetFileSizeEx(hCSVGroupsFile, &liOusFileSize))
			||(!GetFileSizeEx(hCSVGPOsFile, &liGposFileSize)))
		{
			DEBUG_LOG(D_WARNING, "Unable to determine file size.\r\nExiting now...");
			DoExit(D_ERROR);
		}

		if ((liUsersFileSize.HighPart == 0) && (liUsersFileSize.LowPart == 0))
		{
			if (WriteFile(hCSVUsersFile, TEXT("DistinguishedName;CN;UserPrincipalName;LastLogon;PwdLastSet;MemberOf;SecurityDescriptor;SID\r\n"), (DWORD)(_tcslen(TEXT("DistinguishedName;CN;UserPrincipalName;LastLogon;PwdLastSet;MemberOf;SecurityDescriptor;SID\r\n")) * sizeof (TCHAR)), &dwDataRead, NULL) == FALSE)
				goto writerror;
		}
		if ((liGroupsFileSize.HighPart == 0) && (liGroupsFileSize.LowPart == 0))
		{
			if (WriteFile(hCSVGroupsFile, TEXT("DistinguishedName;CN;Description;Members;GroupType;SecurityDescriptor;SID\r\n"), (DWORD)(_tcslen(TEXT("DistinguishedName;CN;Description;Members;GroupType;SecurityDescriptor;SID\r\n")) * sizeof (TCHAR)), &dwDataRead, NULL) == FALSE)
				goto writerror;
		}
		if ((liOusFileSize.HighPart == 0) && (liOusFileSize.LowPart == 0))
		{
			if (WriteFile(hCSVOusFile, TEXT("DistinguishedName;OU;Description;Name;GPLink;GPOptions;SecurityDescriptor\r\n"), (DWORD)(_tcslen(TEXT("DistinguishedName;OU;Description;Name;GPLink;GPOptions;SecurityDescriptor\r\n")) * sizeof (TCHAR)), &dwDataRead, NULL) == FALSE)
				goto writerror;
		}
		if ((liGposFileSize.HighPart == 0) && (liGposFileSize.LowPart == 0))
		{
			if (WriteFile(hCSVGPOsFile, TEXT("DistinguishedName;CN;CreatedDate;ChangeDate;DisplayName;Flags;CorePropagationData;FileSysPath;FunctionalityVersion;MachineExtensionName;UserExtensionName;VersionNumber;WMIFilter;SecurityDescriptor\r\n"), (DWORD)(_tcslen(TEXT("DistinguishedName;CN;CreatedDate;ChangeDate;DisplayName;Flags;CorePropagationData;FileSysPath;FunctionalityVersion;MachineExtensionName;UserExtensionName;VersionNumber;WMIFilter;SecurityDescriptor\r\n")) * sizeof (TCHAR)), &dwDataRead, NULL) == FALSE)
				goto writerror;
		}

		CloseHandle(hCSVUsersFile);
		CloseHandle(hCSVGroupsFile);
		CloseHandle(hCSVOusFile);
		CloseHandle(hCSVGPOsFile);
	}
	return TRUE;

writerror:
	DEBUG_LOG(D_WARNING, "Unable to write DATA HEADER for LDAP printer.\r\nExiting now...");
	DoExit(D_ERROR);
	return FALSE;
}

BOOL PrintLdapDataFooter()
{
	DWORD dwDataRead = 0;

	if (pSyscrwlrOptions->bShouldPrintXML)
	{
		HANDLE hXMLUsersFile = GetFileHandle(OUTPUT_FILE_XML, OUTPUT_DIRECTORY_LDAP_FILE, OUTPUT_NAME_USERS_FILE);
		HANDLE hXMLGroupsFile = GetFileHandle(OUTPUT_FILE_XML, OUTPUT_DIRECTORY_LDAP_FILE, OUTPUT_NAME_GROUPS_FILE);
		HANDLE hXMLOusFile = GetFileHandle(OUTPUT_FILE_XML, OUTPUT_DIRECTORY_LDAP_FILE, OUTPUT_NAME_OUS_FILE);
		HANDLE hXMLGPOsFile = GetFileHandle(OUTPUT_FILE_XML, OUTPUT_DIRECTORY_LDAP_FILE, OUTPUT_NAME_GPOS_FILE);

		if (WriteFile(hXMLUsersFile, TEXT("</LDAPUsers>\r\n"), (DWORD)(_tcslen(TEXT("</LDAPUsers>\r\n")) * sizeof (TCHAR)), &dwDataRead, NULL) == FALSE)
			goto writerror;

		if (WriteFile(hXMLGroupsFile, TEXT("</LDAPGroups>\r\n"), (DWORD)(_tcslen(TEXT("</LDAPGroups>\r\n")) * sizeof (TCHAR)), &dwDataRead, NULL) == FALSE)
			goto writerror;

		if (WriteFile(hXMLOusFile, TEXT("</LDAPOUs>\r\n"), (DWORD)(_tcslen(TEXT("</LDAPOUs>\r\n")) * sizeof (TCHAR)), &dwDataRead, NULL) == FALSE)
			goto writerror;

		if (WriteFile(hXMLGPOsFile, TEXT("</LDAPGPOs>\r\n"), (DWORD)(_tcslen(TEXT("</LDAPGPOs>\r\n")) * sizeof (TCHAR)), &dwDataRead, NULL) == FALSE)
			goto writerror;

		CloseHandle(hXMLUsersFile);
		CloseHandle(hXMLGroupsFile);
		CloseHandle(hXMLOusFile);
		CloseHandle(hXMLGPOsFile);
	}

	return TRUE;

writerror:
	DEBUG_LOG(D_WARNING, "Unable to write DATA FOOTER for LDAP printer.\r\nExiting now...");
	DoExit(D_ERROR);
	return FALSE;
}

BOOL PrintSpecifiedData(_In_ PLDAP_AD_INFOS pLdapADInfos, _In_ LDAP_REQUESTED_DATA_INFO dwRequestedInfo)
{
	HANDLE hXMLFile = INVALID_HANDLE_VALUE;
	HANDLE hCSVFile = INVALID_HANDLE_VALUE;

	if (!pLdapADInfos)
	{
		DEBUG_LOG(D_ERROR, "LDAP_AD_INFOS pointer invalid.\r\nExiting now...");
		DoExit(D_ERROR);
	}

	switch (dwRequestedInfo)
	{
	case LDAP_USER_TARGETED:
		hXMLFile = GetFileHandle(OUTPUT_FILE_XML, OUTPUT_DIRECTORY_LDAP_FILE, OUTPUT_NAME_USERS_FILE);
		hCSVFile = GetFileHandle(OUTPUT_FILE_CSV, OUTPUT_DIRECTORY_LDAP_FILE, OUTPUT_NAME_USERS_FILE);

		if (hXMLFile == INVALID_HANDLE_VALUE || hCSVFile == INVALID_HANDLE_VALUE)
		{
			DEBUG_LOG(D_WARNING, "Handle invalid for LDAP extract.\r\n");
			return FALSE;
		}
		if ((pSyscrwlrOptions->bShouldPrintXML == TRUE) && (PrintXMLDataUsers(hXMLFile, pLdapADInfos) != TRUE))
		{
			DEBUG_LOG(D_ERROR, "Unable to print XML data for LDAP users.\r\n");
			return(FALSE);
		}
		if ((pSyscrwlrOptions->bShouldPrintCSV == TRUE) && (PrintCSVDataUsers(hCSVFile, pLdapADInfos) != TRUE))
		{
			DEBUG_LOG(D_ERROR, "Unable to print CSV data for LDAP users.\r\n");
			return(FALSE);
		}
		if ((pSyscrwlrOptions->bShouldPrintSTDOUT == TRUE) && (PrintSTDOUTDataUsers(pLdapADInfos) != TRUE))
		{
			DEBUG_LOG(D_ERROR, "Unable to print STDOUT data for LDAP users.\r\n");
			return(FALSE);
		}
		break;
	case LDAP_GROUP_TARGETED:
		hXMLFile = GetFileHandle(OUTPUT_FILE_XML, OUTPUT_DIRECTORY_LDAP_FILE, OUTPUT_NAME_GROUPS_FILE);
		hCSVFile = GetFileHandle(OUTPUT_FILE_CSV, OUTPUT_DIRECTORY_LDAP_FILE, OUTPUT_NAME_GROUPS_FILE);

		if (hXMLFile == INVALID_HANDLE_VALUE || hCSVFile == INVALID_HANDLE_VALUE)
		{
			DEBUG_LOG(D_WARNING, "Handle invalid for LDAP extract.\r\n");
			return FALSE;
		}
		if ((pSyscrwlrOptions->bShouldPrintXML == TRUE) && (PrintXMLDataGroups(hXMLFile, pLdapADInfos) != TRUE))
		{
			DEBUG_LOG(D_ERROR, "Unable to print XML data for LDAP groups.\r\n");
			return(FALSE);
		}
		if ((pSyscrwlrOptions->bShouldPrintCSV == TRUE) && (PrintCSVDataGroups(hCSVFile, pLdapADInfos) != TRUE))
		{
			DEBUG_LOG(D_ERROR, "Unable to print CSV data for LDAP groups.\r\n");
			return(FALSE);
		}
		if ((pSyscrwlrOptions->bShouldPrintSTDOUT == TRUE) && (PrintSTDOUTDataGroups(pLdapADInfos) != TRUE))
		{
			DEBUG_LOG(D_ERROR, "Unable to print STDOUT data for LDAP groups.\r\n");
			return(FALSE);
		}
		break;
	case LDAP_OU_TARGETED:
		hXMLFile = GetFileHandle(OUTPUT_FILE_XML, OUTPUT_DIRECTORY_LDAP_FILE, OUTPUT_NAME_OUS_FILE);
		hCSVFile = GetFileHandle(OUTPUT_FILE_CSV, OUTPUT_DIRECTORY_LDAP_FILE, OUTPUT_NAME_OUS_FILE);

		if (hXMLFile == INVALID_HANDLE_VALUE || hCSVFile == INVALID_HANDLE_VALUE)
		{
			DEBUG_LOG(D_WARNING, "Handle invalid for LDAP extract.\r\n");
			return FALSE;
		}
		if ((pSyscrwlrOptions->bShouldPrintXML == TRUE) && (PrintXMLDataOUs(hXMLFile, pLdapADInfos) != TRUE))
		{
			DEBUG_LOG(D_ERROR, "Unable to print XML data for LDAP OU.\r\n");
			return(FALSE);
		}
		if ((pSyscrwlrOptions->bShouldPrintCSV == TRUE) && (PrintCSVDataOUs(hCSVFile, pLdapADInfos) != TRUE))
		{
			DEBUG_LOG(D_ERROR, "Unable to print CSV data for LDAP OU.\r\n");
			return(FALSE);
		}
		if ((pSyscrwlrOptions->bShouldPrintSTDOUT == TRUE) && (PrintSTDOUTDataOUs(pLdapADInfos) != TRUE))
		{
			DEBUG_LOG(D_ERROR, "Unable to print STDOUT data for LDAP OU.\r\n");
			return(FALSE);
		}
		break;
	case LDAP_GPO_TARGETED:
		hXMLFile = GetFileHandle(OUTPUT_FILE_XML, OUTPUT_DIRECTORY_LDAP_FILE, OUTPUT_NAME_GPOS_FILE);
		hCSVFile = GetFileHandle(OUTPUT_FILE_CSV, OUTPUT_DIRECTORY_LDAP_FILE, OUTPUT_NAME_GPOS_FILE);

		if (hXMLFile == INVALID_HANDLE_VALUE || hCSVFile == INVALID_HANDLE_VALUE)
		{
			DEBUG_LOG(D_WARNING, "Handle invalid for LDAP extract.\r\n");
			return FALSE;
		}
		if ((pSyscrwlrOptions->bShouldPrintXML == TRUE) && (PrintXMLDataGPOs(hXMLFile, pLdapADInfos) != TRUE))
		{
			DEBUG_LOG(D_ERROR, "Unable to print XML data for LDAP GPO.\r\n");
			return(FALSE);
		}
		if ((pSyscrwlrOptions->bShouldPrintCSV == TRUE) && (PrintCSVDataGPOs(hCSVFile, pLdapADInfos) != TRUE))
		{
			DEBUG_LOG(D_ERROR, "Unable to print CSV data for LDAP GPO.\r\n");
			return(FALSE);
		}
		if ((pSyscrwlrOptions->bShouldPrintSTDOUT == TRUE) && (PrintSTDOUTDataGPOs(pLdapADInfos) != TRUE))
		{
			DEBUG_LOG(D_ERROR, "Unable to print STDOUT data for LDAP GPO.\r\n");
			return(FALSE);
		}
		break;
	default:
		DEBUG_LOG(D_ERROR, "LDAP_REQUESTED_DATA_INFO unknown.\r\n");
		return FALSE;
		break;
	}

	return TRUE;
}

BOOL PrintXMLData(_In_ PLDAP_AD_INFOS pLdapADInfos)
{
	HANDLE hXMLUsersFile = GetFileHandle(OUTPUT_FILE_XML, OUTPUT_DIRECTORY_LDAP_FILE, OUTPUT_NAME_USERS_FILE);
	HANDLE hXMLGroupsFile = GetFileHandle(OUTPUT_FILE_XML, OUTPUT_DIRECTORY_LDAP_FILE, OUTPUT_NAME_GROUPS_FILE);
	HANDLE hXMLOusFile = GetFileHandle(OUTPUT_FILE_XML, OUTPUT_DIRECTORY_LDAP_FILE, OUTPUT_NAME_OUS_FILE);
	HANDLE hXMLGPOsFile = GetFileHandle(OUTPUT_FILE_XML, OUTPUT_DIRECTORY_LDAP_FILE, OUTPUT_NAME_GPOS_FILE);
	
	if (!pLdapADInfos || hXMLUsersFile == INVALID_HANDLE_VALUE || hXMLGroupsFile == INVALID_HANDLE_VALUE || hXMLOusFile == INVALID_HANDLE_VALUE || hXMLGPOsFile == INVALID_HANDLE_VALUE)
	{
		DEBUG_LOG(D_WARNING, "PLDAP_AD_INFOS or handle invalid for LDAP extract.\r\n");
		return(FALSE);
	}

	if (PrintXMLDataUsers(hXMLUsersFile, pLdapADInfos) != TRUE)
	{
		DEBUG_LOG(D_ERROR, "Unable to print XML data for LDAP users.\r\n");
		return(FALSE);
	}

	if (PrintXMLDataGroups(hXMLGroupsFile, pLdapADInfos) != TRUE)
	{
		DEBUG_LOG(D_ERROR, "Unable to print XML data for LDAP groups.\r\n");
		return(FALSE);
	}

	if (PrintXMLDataOUs(hXMLOusFile, pLdapADInfos) != TRUE)
	{
		DEBUG_LOG(D_ERROR, "Unable to print XML data for LDAP OUs.\r\n");
		return(FALSE);
	}

	if (PrintXMLDataGPOs(hXMLGPOsFile, pLdapADInfos) != TRUE)
	{
		DEBUG_LOG(D_ERROR, "Unable to print XML data for LDAP GPOs.\r\n");
		return(FALSE);
	}

	return TRUE;
}

BOOL PrintXMLDataUsers(_In_ HANDLE hXMLUsersFile, _In_ PLDAP_AD_INFOS pLdapADInfos)
{
	DWORD dwDataRead = 0;

	if (hXMLUsersFile == INVALID_HANDLE_VALUE || !pLdapADInfos)
		return FALSE;

	for (DWORD i = 0; i < pLdapADInfos->dwNumberOfUser; ++i)
	{
		PLDAP_AD_USER pCurrUserData = pLdapADInfos->pUsers[i];
		PTCHAR tEscapedDN = EscapeXMLString(pCurrUserData->tDistinguishedName);
		PTCHAR tEscapedCN = EscapeXMLString(pCurrUserData->tCN);
		PTCHAR tEscapedLastLogon = EscapeXMLString(pCurrUserData->tLastLogon);
		PTCHAR tEscapedMemberOf = EscapeXMLString(pCurrUserData->tMemberOf);
		PTCHAR tEscapedPwdLastSet = EscapeXMLString(pCurrUserData->tPwdLastSet);
		PTCHAR tUserPrincipalName = EscapeXMLString(pCurrUserData->tUserPrincipalName);
		PTCHAR tEscapedSecurityDescriptor = EscapeXMLString(pCurrUserData->tSecurityDescriptor);
		PTCHAR tEscapedSid = EscapeXMLString(pCurrUserData->tSid);

		if (!pCurrUserData->tDistinguishedName)
			continue;

		if ((WriteFile(hXMLUsersFile, TEXT("\t<User name=\""), (DWORD)(_tcslen(TEXT("\t<User name=\"")) * sizeof (TCHAR)), &dwDataRead, NULL) == FALSE)
			|| (WriteFile(hXMLUsersFile, tEscapedDN, (DWORD)(_tcslen(tEscapedDN) * sizeof (WCHAR)), &dwDataRead, NULL) == FALSE)
			|| (WriteFile(hXMLUsersFile, TEXT("\">\r\n"), (DWORD)(_tcslen(TEXT("\">\r\n")) * sizeof (TCHAR)), &dwDataRead, NULL) == FALSE))
			goto writerror;

		if (pCurrUserData->tCN != NULL)
		{
			if ((WriteFile(hXMLUsersFile, TEXT("\t\t<CN>"), (DWORD)(_tcslen(TEXT("\t\t<CN>")) * sizeof (TCHAR)), &dwDataRead, NULL) == FALSE)
			|| (WriteFile(hXMLUsersFile, tEscapedCN, (DWORD)(_tcslen(tEscapedCN) * sizeof (WCHAR)), &dwDataRead, NULL) == FALSE)
			|| (WriteFile(hXMLUsersFile, TEXT("</CN>\r\n"), (DWORD)(_tcslen(TEXT("</CN>\r\n")) * sizeof (TCHAR)), &dwDataRead, NULL) == FALSE))
				goto writerror;
		}
		if (pCurrUserData->tLastLogon != NULL)
		{
			if ((WriteFile(hXMLUsersFile, TEXT("\t\t<Lastlogon>"), (DWORD)(_tcslen(TEXT("\t\t<Lastlogon>")) * sizeof (TCHAR)), &dwDataRead, NULL) == FALSE)
			|| (WriteFile(hXMLUsersFile, tEscapedLastLogon, (DWORD)(_tcslen(tEscapedLastLogon) * sizeof (WCHAR)), &dwDataRead, NULL) == FALSE)
			|| (WriteFile(hXMLUsersFile, TEXT("</Lastlogon>\r\n"), (DWORD)(_tcslen(TEXT("</Lastlogon>\r\n")) * sizeof (TCHAR)), &dwDataRead, NULL) == FALSE))
				goto writerror;
		}
		if (pCurrUserData->tMemberOf != NULL)
		{
			if ((WriteFile(hXMLUsersFile, TEXT("\t\t<Memberof>"), (DWORD)(_tcslen(TEXT("\t\t<Memberof>")) * sizeof (TCHAR)), &dwDataRead, NULL) == FALSE)
			|| (WriteFile(hXMLUsersFile, tEscapedMemberOf, (DWORD)(_tcslen(tEscapedMemberOf) * sizeof (WCHAR)), &dwDataRead, NULL) == FALSE)
			|| (WriteFile(hXMLUsersFile, TEXT("</Memberof>\r\n"), (DWORD)(_tcslen(TEXT("</Memberof>\r\n")) * sizeof (TCHAR)), &dwDataRead, NULL) == FALSE))
				goto writerror;
		}
		if (pCurrUserData->tPwdLastSet != NULL)
		{
			if ((WriteFile(hXMLUsersFile, TEXT("\t\t<Pwdlastset>"), (DWORD)(_tcslen(TEXT("\t\t<Pwdlastset>")) * sizeof (TCHAR)), &dwDataRead, NULL) == FALSE)
			|| (WriteFile(hXMLUsersFile, tEscapedPwdLastSet, (DWORD)(_tcslen(tEscapedPwdLastSet) * sizeof (WCHAR)), &dwDataRead, NULL) == FALSE)
			|| (WriteFile(hXMLUsersFile, TEXT("</Pwdlastset>\r\n"), (DWORD)(_tcslen(TEXT("</Pwdlastset>\r\n")) * sizeof (TCHAR)), &dwDataRead, NULL) == FALSE))
				goto writerror;
		}
		if (pCurrUserData->tUserPrincipalName != NULL)
		{
			if ((WriteFile(hXMLUsersFile, TEXT("\t<Userprincipalname>"), (DWORD)(_tcslen(TEXT("\t<Userprincipalname>")) * sizeof (TCHAR)), &dwDataRead, NULL) == FALSE)
			|| (WriteFile(hXMLUsersFile, tUserPrincipalName, (DWORD)(_tcslen(tUserPrincipalName) * sizeof (WCHAR)), &dwDataRead, NULL) == FALSE)
			|| (WriteFile(hXMLUsersFile, TEXT("</Userprincipalname>\r\n"), (DWORD)(_tcslen(TEXT("</Userprincipalname>\r\n")) * sizeof (TCHAR)), &dwDataRead, NULL) == FALSE))
				goto writerror;
		}
		if (pCurrUserData->tSecurityDescriptor != NULL)
		{
			if ((WriteFile(hXMLUsersFile, TEXT("\t<SecurityDescriptor>"), (DWORD)(_tcslen(TEXT("\t<SecurityDescriptor>")) * sizeof (TCHAR)), &dwDataRead, NULL) == FALSE)
				|| (WriteFile(hXMLUsersFile, tEscapedSecurityDescriptor, (DWORD)(_tcslen(tEscapedSecurityDescriptor) * sizeof (WCHAR)), &dwDataRead, NULL) == FALSE)
				|| (WriteFile(hXMLUsersFile, TEXT("</SecurityDescriptor>\r\n"), (DWORD)(_tcslen(TEXT("</SecurityDescriptor>\r\n")) * sizeof (TCHAR)), &dwDataRead, NULL) == FALSE))
				goto writerror;
		}
		if (pCurrUserData->tSid != NULL)
		{
			if ((WriteFile(hXMLUsersFile, TEXT("\t<Sid>"), (DWORD)(_tcslen(TEXT("\t<Sid>")) * sizeof (TCHAR)), &dwDataRead, NULL) == FALSE)
				|| (WriteFile(hXMLUsersFile, tEscapedSid, (DWORD)(_tcslen(tEscapedSid) * sizeof (WCHAR)), &dwDataRead, NULL) == FALSE)
				|| (WriteFile(hXMLUsersFile, TEXT("</Sid>\r\n"), (DWORD)(_tcslen(TEXT("</Sid>\r\n")) * sizeof (TCHAR)), &dwDataRead, NULL) == FALSE))
				goto writerror;
		}

		if (WriteFile(hXMLUsersFile, TEXT("\t</User>\r\n"), (DWORD)(_tcslen(TEXT("\t</User>\r\n")) * sizeof (TCHAR)), &dwDataRead, NULL) == FALSE)
			goto writerror;
		
		if (tEscapedDN)
			HeapFree(hCrawlerHeap, NULL, tEscapedDN);
		if (tEscapedCN)
			HeapFree(hCrawlerHeap, NULL, tEscapedCN);
		if (tEscapedLastLogon)
			HeapFree(hCrawlerHeap, NULL, tEscapedLastLogon);
		if (tEscapedMemberOf)
			HeapFree(hCrawlerHeap, NULL, tEscapedMemberOf);
		if (tEscapedPwdLastSet)
			HeapFree(hCrawlerHeap, NULL, tEscapedPwdLastSet);
		if (tUserPrincipalName)
			HeapFree(hCrawlerHeap, NULL, tUserPrincipalName);
		if (tEscapedSecurityDescriptor)
			HeapFree(hCrawlerHeap, NULL, tEscapedSecurityDescriptor);
		if (tEscapedSid)
			HeapFree(hCrawlerHeap, NULL, tEscapedSid);
	}

	CloseHandle(hXMLUsersFile);
	return TRUE;

writerror:
	DEBUG_LOG(D_WARNING, "Unable to write XML DATA.\r\nExiting now...");
	DoExit(D_ERROR);
	return FALSE;
}

BOOL PrintXMLDataGroups(_In_ HANDLE hXMLGroupsFile, _In_ PLDAP_AD_INFOS pLdapADInfos)
{
	DWORD dwDataRead = 0;

	if (hXMLGroupsFile == INVALID_HANDLE_VALUE || !pLdapADInfos)
		return FALSE;

	for (DWORD i = 0; i < pLdapADInfos->dwNumberOfGroup; ++i)
	{
		PLDAP_AD_GROUP pCurrGroupData = pLdapADInfos->pGroups[i];
		PTCHAR tEscapedDN = EscapeXMLString(pCurrGroupData->tDistinguishedName);
		PTCHAR tEscapedCN = EscapeXMLString(pCurrGroupData->tCN);
		PTCHAR tEscapedDescription = EscapeXMLString(pCurrGroupData->tDescription);
		PTCHAR tEscapedMember = EscapeXMLString(pCurrGroupData->tMember);
		PTCHAR tEscapedGroupType = EscapeXMLString(pCurrGroupData->tGroupType);
		PTCHAR tEscapedSecurityDescriptor = EscapeXMLString(pCurrGroupData->tSecurityDescriptor);
		PTCHAR tEscapedSid = EscapeXMLString(pCurrGroupData->tSid);

		if (!pCurrGroupData->tDistinguishedName)
			continue;

		if ((WriteFile(hXMLGroupsFile, TEXT("\t<Group name=\""), (DWORD)(_tcslen(TEXT("\t<Group name=\"")) * sizeof (TCHAR)), &dwDataRead, NULL) == FALSE)
			|| (WriteFile(hXMLGroupsFile, tEscapedDN, (DWORD)(_tcslen(tEscapedDN) * sizeof (WCHAR)), &dwDataRead, NULL) == FALSE)
			|| (WriteFile(hXMLGroupsFile, TEXT("\">\r\n"), (DWORD)(_tcslen(TEXT("\">\r\n")) * sizeof (TCHAR)), &dwDataRead, NULL) == FALSE))
			goto writerror;

		if (pCurrGroupData->tCN != NULL)
		{
			if ((WriteFile(hXMLGroupsFile, TEXT("\t\t<CN>"), (DWORD)(_tcslen(TEXT("\t\t<CN>")) * sizeof (TCHAR)), &dwDataRead, NULL) == FALSE)
			|| (WriteFile(hXMLGroupsFile, tEscapedCN, (DWORD)(_tcslen(tEscapedCN) * sizeof (WCHAR)), &dwDataRead, NULL) == FALSE)
			|| (WriteFile(hXMLGroupsFile, TEXT("</CN>\r\n"), (DWORD)(_tcslen(TEXT("</CN>\r\n")) * sizeof (TCHAR)), &dwDataRead, NULL) == FALSE))
				goto writerror;
		}
		if (pCurrGroupData->tDescription != NULL)
		{
			if ((WriteFile(hXMLGroupsFile, TEXT("\t\t<Description>"), (DWORD)(_tcslen(TEXT("\t\t<Description>")) * sizeof (TCHAR)), &dwDataRead, NULL) == FALSE)
			|| (WriteFile(hXMLGroupsFile, tEscapedDescription, (DWORD)(_tcslen(tEscapedDescription) * sizeof (WCHAR)), &dwDataRead, NULL) == FALSE)
			|| (WriteFile(hXMLGroupsFile, TEXT("</Description>\r\n"), (DWORD)(_tcslen(TEXT("</Description>\r\n")) * sizeof (TCHAR)), &dwDataRead, NULL) == FALSE))
				goto writerror;
		}
		if (pCurrGroupData->tMember != NULL)
		{
			if ((WriteFile(hXMLGroupsFile, TEXT("\t\t<Members>"), (DWORD)(_tcslen(TEXT("\t\t<Members>")) * sizeof (TCHAR)), &dwDataRead, NULL) == FALSE)
				|| (WriteFile(hXMLGroupsFile, tEscapedMember, (DWORD)(_tcslen(tEscapedMember) * sizeof (WCHAR)), &dwDataRead, NULL) == FALSE)
			|| (WriteFile(hXMLGroupsFile, TEXT("</Members>\r\n"), (DWORD)(_tcslen(TEXT("</Members>\r\n")) * sizeof (TCHAR)), &dwDataRead, NULL) == FALSE))
				goto writerror;
		}
		if (pCurrGroupData->tGroupType != NULL)
		{
			if ((WriteFile(hXMLGroupsFile, TEXT("\t\t<Grouptype>"), (DWORD)(_tcslen(TEXT("\t\t<Grouptype>")) * sizeof (TCHAR)), &dwDataRead, NULL) == FALSE)
			|| (WriteFile(hXMLGroupsFile, tEscapedGroupType, (DWORD)(_tcslen(tEscapedGroupType) * sizeof (WCHAR)), &dwDataRead, NULL) == FALSE)
			|| (WriteFile(hXMLGroupsFile, TEXT("</Grouptype>\r\n"), (DWORD)(_tcslen(TEXT("</Grouptype>\r\n")) * sizeof (TCHAR)), &dwDataRead, NULL) == FALSE))
				goto writerror;
		}
		if (pCurrGroupData->tSecurityDescriptor != NULL)
		{
			if ((WriteFile(hXMLGroupsFile, TEXT("\t\t<SecurityDescriptor>"), (DWORD)(_tcslen(TEXT("\t\t<SecurityDescriptor>")) * sizeof (TCHAR)), &dwDataRead, NULL) == FALSE)
				|| (WriteFile(hXMLGroupsFile, tEscapedSecurityDescriptor, (DWORD)(_tcslen(tEscapedSecurityDescriptor) * sizeof (WCHAR)), &dwDataRead, NULL) == FALSE)
				|| (WriteFile(hXMLGroupsFile, TEXT("</Grouptype>\r\n"), (DWORD)(_tcslen(TEXT("</Grouptype>\r\n")) * sizeof (TCHAR)), &dwDataRead, NULL) == FALSE))
				goto writerror;
		}
		if (pCurrGroupData->tSid != NULL)
		{
			if ((WriteFile(hXMLGroupsFile, TEXT("\t\t<Sid>"), (DWORD)(_tcslen(TEXT("\t\t<Sid>")) * sizeof (TCHAR)), &dwDataRead, NULL) == FALSE)
				|| (WriteFile(hXMLGroupsFile, tEscapedSid, (DWORD)(_tcslen(tEscapedSid) * sizeof (WCHAR)), &dwDataRead, NULL) == FALSE)
				|| (WriteFile(hXMLGroupsFile, TEXT("</Sid>\r\n"), (DWORD)(_tcslen(TEXT("</Sid>\r\n")) * sizeof (TCHAR)), &dwDataRead, NULL) == FALSE))
				goto writerror;
		}

		if (WriteFile(hXMLGroupsFile, TEXT("\t</Group>\r\n"), (DWORD)(_tcslen(TEXT("\t</Group>\r\n")) * sizeof (TCHAR)), &dwDataRead, NULL) == FALSE)
			goto writerror;
		
		if (tEscapedDN)
			HeapFree(hCrawlerHeap, NULL, tEscapedDN);
		if (tEscapedCN)
			HeapFree(hCrawlerHeap, NULL, tEscapedCN);
		if (tEscapedDescription)
			HeapFree(hCrawlerHeap, NULL, tEscapedDescription);
		if (tEscapedMember)
			HeapFree(hCrawlerHeap, NULL, tEscapedMember);
		if (tEscapedGroupType)
			HeapFree(hCrawlerHeap, NULL, tEscapedGroupType);
		if (tEscapedSecurityDescriptor)
			HeapFree(hCrawlerHeap, NULL, tEscapedSecurityDescriptor);
		if (tEscapedSid)
			HeapFree(hCrawlerHeap, NULL, tEscapedSid);
	}

	CloseHandle(hXMLGroupsFile);
	return TRUE;

writerror:
	DEBUG_LOG(D_WARNING, "Unable to write XML DATA.\r\nExiting now...");
	DoExit(D_ERROR);
	return FALSE;
}

BOOL PrintXMLDataOUs(_In_ HANDLE hXMLOusFile, _In_ PLDAP_AD_INFOS pLdapADInfos)
{
	DWORD dwDataRead = 0;

	if (hXMLOusFile == INVALID_HANDLE_VALUE || !pLdapADInfos)
		return FALSE;

	for (DWORD i = 0; i < pLdapADInfos->dwNumberOfOU; ++i)
	{
		PLDAP_AD_OU pCurrOUData = pLdapADInfos->pOUs[i];
		PTCHAR tEscapedDN = EscapeXMLString(pCurrOUData->tDistinguishedName);
		PTCHAR tEscapedDescription = EscapeXMLString(pCurrOUData->tDescription);
		PTCHAR tEscapedGpLink = EscapeXMLString(pCurrOUData->tGpLink);
		PTCHAR tEscapedGpOptions = EscapeXMLString(pCurrOUData->tGpOptions);
		PTCHAR tEscapedName = EscapeXMLString(pCurrOUData->tName);
		PTCHAR tEscapedOu = EscapeXMLString(pCurrOUData->tOu);
		PTCHAR tEscapedSecurityDescriptor = EscapeXMLString(pCurrOUData->tSecurityDescriptor);

		if (!pCurrOUData->tDistinguishedName)
			continue;

		if ((WriteFile(hXMLOusFile, TEXT("\t<OU name=\""), (DWORD)(_tcslen(TEXT("\t<OU name=\"")) * sizeof (TCHAR)), &dwDataRead, NULL) == FALSE)
			|| (WriteFile(hXMLOusFile, tEscapedDN, (DWORD)(_tcslen(tEscapedDN) * sizeof (WCHAR)), &dwDataRead, NULL) == FALSE)
			|| (WriteFile(hXMLOusFile, TEXT("\">\r\n"), (DWORD)(_tcslen(TEXT("\">\r\n")) * sizeof (TCHAR)), &dwDataRead, NULL) == FALSE))
			goto writerror;

		if (pCurrOUData->tDescription != NULL)
		{
			if ((WriteFile(hXMLOusFile, TEXT("\t\t<Description>"), (DWORD)(_tcslen(TEXT("\t\t<Description>")) * sizeof (TCHAR)), &dwDataRead, NULL) == FALSE)
			|| (WriteFile(hXMLOusFile, tEscapedDescription, (DWORD)(_tcslen(tEscapedDescription) * sizeof (WCHAR)), &dwDataRead, NULL) == FALSE)
			|| (WriteFile(hXMLOusFile, TEXT("</Description>\r\n"), (DWORD)(_tcslen(TEXT("</Description>\r\n")) * sizeof (TCHAR)), &dwDataRead, NULL) == FALSE))
				goto writerror;
		}
		if (pCurrOUData->tGpLink != NULL)
		{
			if ((WriteFile(hXMLOusFile, TEXT("\t\t<Gplink>"), (DWORD)(_tcslen(TEXT("\t\t<Gplink>")) * sizeof (TCHAR)), &dwDataRead, NULL) == FALSE)
			|| (WriteFile(hXMLOusFile, tEscapedGpLink, (DWORD)(_tcslen(tEscapedGpLink) * sizeof (WCHAR)), &dwDataRead, NULL) == FALSE)
			|| (WriteFile(hXMLOusFile, TEXT("</Gplink>\r\n"), (DWORD)(_tcslen(TEXT("</Gplink>\r\n")) * sizeof (TCHAR)), &dwDataRead, NULL) == FALSE))
				goto writerror;
		}
		if (pCurrOUData->tGpOptions != NULL)
		{
			if ((WriteFile(hXMLOusFile, TEXT("\t\t<GpOptions>"), (DWORD)(_tcslen(TEXT("\t\t<GpOptions>")) * sizeof (TCHAR)), &dwDataRead, NULL) == FALSE)
				|| (WriteFile(hXMLOusFile, tEscapedGpOptions, (DWORD)(_tcslen(tEscapedGpOptions) * sizeof (WCHAR)), &dwDataRead, NULL) == FALSE)
				|| (WriteFile(hXMLOusFile, TEXT("</GpOptions>\r\n"), (DWORD)(_tcslen(TEXT("</GpOptions>\r\n")) * sizeof (TCHAR)), &dwDataRead, NULL) == FALSE))
				goto writerror;
		}
		if (pCurrOUData->tName != NULL)
		{
			if ((WriteFile(hXMLOusFile, TEXT("\t\t<Name>"), (DWORD)(_tcslen(TEXT("\t\t<Name>")) * sizeof (TCHAR)), &dwDataRead, NULL) == FALSE)
			|| (WriteFile(hXMLOusFile, tEscapedName, (DWORD)(_tcslen(tEscapedName) * sizeof (WCHAR)), &dwDataRead, NULL) == FALSE)
			|| (WriteFile(hXMLOusFile, TEXT("</Name>\r\n"), (DWORD)(_tcslen(TEXT("</Name>\r\n")) * sizeof (TCHAR)), &dwDataRead, NULL) == FALSE))
				goto writerror;
		}
		if (pCurrOUData->tOu != NULL)
		{
			if ((WriteFile(hXMLOusFile, TEXT("\t\t<OUname>"), (DWORD)(_tcslen(TEXT("\t\t<OUname>")) * sizeof (TCHAR)), &dwDataRead, NULL) == FALSE)
			|| (WriteFile(hXMLOusFile, tEscapedOu, (DWORD)(_tcslen(tEscapedOu) * sizeof (WCHAR)), &dwDataRead, NULL) == FALSE)
			|| (WriteFile(hXMLOusFile, TEXT("</OUname>\r\n"), (DWORD)(_tcslen(TEXT("</OUname>\r\n")) * sizeof (TCHAR)), &dwDataRead, NULL) == FALSE))
				goto writerror;
		}
		if (pCurrOUData->tSecurityDescriptor != NULL)
		{
			if ((WriteFile(hXMLOusFile, TEXT("\t\t<SecurityDescriptor>"), (DWORD)(_tcslen(TEXT("\t\t<SecurityDescriptor>")) * sizeof (TCHAR)), &dwDataRead, NULL) == FALSE)
				|| (WriteFile(hXMLOusFile, tEscapedSecurityDescriptor, (DWORD)(_tcslen(tEscapedSecurityDescriptor) * sizeof (WCHAR)), &dwDataRead, NULL) == FALSE)
				|| (WriteFile(hXMLOusFile, TEXT("</SecurityDescriptor>\r\n"), (DWORD)(_tcslen(TEXT("</SecurityDescriptor>\r\n")) * sizeof (TCHAR)), &dwDataRead, NULL) == FALSE))
				goto writerror;
		}

		if (WriteFile(hXMLOusFile, TEXT("\t</OU>\r\n"), (DWORD)(_tcslen(TEXT("\t</OU>\r\n")) * sizeof (TCHAR)), &dwDataRead, NULL) == FALSE)
			goto writerror;

		if (tEscapedDN)
			HeapFree(hCrawlerHeap, NULL, tEscapedDN);
		if (tEscapedDescription)
			HeapFree(hCrawlerHeap, NULL, tEscapedDescription);
		if (tEscapedGpLink)
			HeapFree(hCrawlerHeap, NULL, tEscapedGpLink);
		if (tEscapedGpOptions)
			HeapFree(hCrawlerHeap, NULL, tEscapedGpOptions);
		if (tEscapedName)
			HeapFree(hCrawlerHeap, NULL, tEscapedName);
		if (tEscapedOu)
			HeapFree(hCrawlerHeap, NULL, tEscapedOu);
		if (tEscapedSecurityDescriptor)
			HeapFree(hCrawlerHeap, NULL, tEscapedSecurityDescriptor);
	}

	CloseHandle(hXMLOusFile);
	return TRUE;

writerror:
	DEBUG_LOG(D_WARNING, "Unable to write XML DATA.\r\nExiting now...");
	DoExit(D_ERROR);
	return FALSE;
}

BOOL PrintXMLDataGPOs(_In_ HANDLE hXMLGPOsFile, _In_ PLDAP_AD_INFOS pLdapADInfos)
{
	DWORD dwDataRead = 0;

	if (hXMLGPOsFile == INVALID_HANDLE_VALUE || !pLdapADInfos)
		return FALSE;

	for (DWORD i = 0; i < pLdapADInfos->dwNumberOfGPO; ++i)
	{
		PLDAP_AD_GPO pCurrGPOData = pLdapADInfos->pGPOs[i];
		PTCHAR tEscapedDN = EscapeXMLString(pCurrGPOData->tDistinguishedName);
		PTCHAR tEscapedCN = EscapeXMLString(pCurrGPOData->tCN);
		PTCHAR tEscapedCorePropagationData= EscapeXMLString(pCurrGPOData->tCorePropagationData);
		PTCHAR tEscapedDisplayName = EscapeXMLString(pCurrGPOData->tDisplayName);
		PTCHAR tEscapedSysPath = EscapeXMLString(pCurrGPOData->tFileSysPath);
		PTCHAR tEscapedFlags = EscapeXMLString(pCurrGPOData->tFlags);
		PTCHAR tEscapedFunctionalityVersion = EscapeXMLString(pCurrGPOData->tFunctionalityVersion);
		PTCHAR tEscapedMachineExtensionsNames = EscapeXMLString(pCurrGPOData->tMachineExtensionsNames);
		PTCHAR tEscapedUserExtensionsNames = EscapeXMLString(pCurrGPOData->tUserExtensionsNames);
		PTCHAR tEscapedVersionNumber = EscapeXMLString(pCurrGPOData->tVersionNumber);
		PTCHAR tEscapedWhenChanged = EscapeXMLString(pCurrGPOData->tWhenChanged);
		PTCHAR tEscapedWhenCreated = EscapeXMLString(pCurrGPOData->tWhenCreated);
		PTCHAR tEscapedWmiFilter = EscapeXMLString(pCurrGPOData->tWQLFilter);
		PTCHAR tEscapedSecurityDescriptor = EscapeXMLString(pCurrGPOData->tSecurityDescriptor);

		if (!pCurrGPOData->tDistinguishedName)
			continue;

		if ((WriteFile(hXMLGPOsFile, TEXT("\t<GPO name=\""), (DWORD)(_tcslen(TEXT("\t<GPO name=\"")) * sizeof (TCHAR)), &dwDataRead, NULL) == FALSE)
			|| (WriteFile(hXMLGPOsFile, tEscapedDN, (DWORD)(_tcslen(tEscapedDN) * sizeof (WCHAR)), &dwDataRead, NULL) == FALSE)
			|| (WriteFile(hXMLGPOsFile, TEXT("\">\r\n"), (DWORD)(_tcslen(TEXT("\">\r\n")) * sizeof (TCHAR)), &dwDataRead, NULL) == FALSE))
			goto writerror;

		if (pCurrGPOData->tCN != NULL)
		{
			if ((WriteFile(hXMLGPOsFile, TEXT("\t\t<CN>"), (DWORD)(_tcslen(TEXT("\t\t<CN>")) * sizeof (TCHAR)), &dwDataRead, NULL) == FALSE)
			|| (WriteFile(hXMLGPOsFile, tEscapedCN, (DWORD)(_tcslen(tEscapedCN) * sizeof (WCHAR)), &dwDataRead, NULL) == FALSE)
			|| (WriteFile(hXMLGPOsFile, TEXT("</CN>\r\n"), (DWORD)(_tcslen(TEXT("</CN>\r\n")) * sizeof (TCHAR)), &dwDataRead, NULL) == FALSE))
				goto writerror;
		}
		if (pCurrGPOData->tCorePropagationData != NULL)
		{
			if ((WriteFile(hXMLGPOsFile, TEXT("\t\t<Corepropagationdata>"), (DWORD)(_tcslen(TEXT("\t\t<Corepropagationdata>")) * sizeof (TCHAR)), &dwDataRead, NULL) == FALSE)
			|| (WriteFile(hXMLGPOsFile, tEscapedCorePropagationData, (DWORD)(_tcslen(tEscapedCorePropagationData) * sizeof (WCHAR)), &dwDataRead, NULL) == FALSE)
			|| (WriteFile(hXMLGPOsFile, TEXT("</Corepropagationdata>\r\n"), (DWORD)(_tcslen(TEXT("</Corepropagationdata>\r\n")) * sizeof (TCHAR)), &dwDataRead, NULL) == FALSE))
				goto writerror;
		}
		if (pCurrGPOData->tDisplayName != NULL)
		{
			if ((WriteFile(hXMLGPOsFile, TEXT("\t\t<Displayname>"), (DWORD)(_tcslen(TEXT("\t\t<Displayname>")) * sizeof (TCHAR)), &dwDataRead, NULL) == FALSE)
			|| (WriteFile(hXMLGPOsFile, tEscapedDisplayName, (DWORD)(_tcslen(tEscapedDisplayName) * sizeof (WCHAR)), &dwDataRead, NULL) == FALSE)
			|| (WriteFile(hXMLGPOsFile, TEXT("</Displayname>\r\n"), (DWORD)(_tcslen(TEXT("</Displayname>\r\n")) * sizeof (TCHAR)), &dwDataRead, NULL) == FALSE))
				goto writerror;
		}
		if (pCurrGPOData->tFileSysPath != NULL)
		{
			if ((WriteFile(hXMLGPOsFile, TEXT("\t\t<Filesyspath>"), (DWORD)(_tcslen(TEXT("\t\t<Filesyspath>")) * sizeof (TCHAR)), &dwDataRead, NULL) == FALSE)
			|| (WriteFile(hXMLGPOsFile, tEscapedSysPath, (DWORD)(_tcslen(tEscapedSysPath) * sizeof (WCHAR)), &dwDataRead, NULL) == FALSE)
			|| (WriteFile(hXMLGPOsFile, TEXT("</Filesyspath>\r\n"), (DWORD)(_tcslen(TEXT("</Filesyspath>\r\n")) * sizeof (TCHAR)), &dwDataRead, NULL) == FALSE))
				goto writerror;
		}
		if (pCurrGPOData->tFlags != NULL)
		{
			if ((WriteFile(hXMLGPOsFile, TEXT("\t\t<Flags>"), (DWORD)(_tcslen(TEXT("\t\t<Flags>")) * sizeof (TCHAR)), &dwDataRead, NULL) == FALSE)
			|| (WriteFile(hXMLGPOsFile, tEscapedFlags, (DWORD)(_tcslen(tEscapedFlags) * sizeof (WCHAR)), &dwDataRead, NULL) == FALSE)
			|| (WriteFile(hXMLGPOsFile, TEXT("</Flags>\r\n"), (DWORD)(_tcslen(TEXT("</Flags>\r\n")) * sizeof (TCHAR)), &dwDataRead, NULL) == FALSE))
				goto writerror;
		}
		if (pCurrGPOData->tFunctionalityVersion != NULL)
		{
			if ((WriteFile(hXMLGPOsFile, TEXT("\t\t<FunctionalityVersion>"), (DWORD)(_tcslen(TEXT("\t\t<FunctionalityVersion>")) * sizeof (TCHAR)), &dwDataRead, NULL) == FALSE)
			|| (WriteFile(hXMLGPOsFile, tEscapedFunctionalityVersion, (DWORD)(_tcslen(tEscapedFunctionalityVersion) * sizeof (WCHAR)), &dwDataRead, NULL) == FALSE)
			|| (WriteFile(hXMLGPOsFile, TEXT("</FunctionalityVersion>\r\n"), (DWORD)(_tcslen(TEXT("</FunctionalityVersion>\r\n")) * sizeof (TCHAR)), &dwDataRead, NULL) == FALSE))
				goto writerror;
		}
		if (pCurrGPOData->tMachineExtensionsNames != NULL)
		{
			if ((WriteFile(hXMLGPOsFile, TEXT("\t\t<MachineExtensionsNames>"), (DWORD)(_tcslen(TEXT("\t\t<MachineExtensionsNames>")) * sizeof (TCHAR)), &dwDataRead, NULL) == FALSE)
			|| (WriteFile(hXMLGPOsFile, tEscapedMachineExtensionsNames, (DWORD)(_tcslen(tEscapedMachineExtensionsNames) * sizeof (WCHAR)), &dwDataRead, NULL) == FALSE)
			|| (WriteFile(hXMLGPOsFile, TEXT("</MachineExtensionsNames>\r\n"), (DWORD)(_tcslen(TEXT("</MachineExtensionsNames>\r\n")) * sizeof (TCHAR)), &dwDataRead, NULL) == FALSE))
				goto writerror;
		}
		if (pCurrGPOData->tUserExtensionsNames != NULL)
		{
			if ((WriteFile(hXMLGPOsFile, TEXT("\t\t<UserExtensionsNames>"), (DWORD)(_tcslen(TEXT("\t\t<UserExtensionsNames>")) * sizeof (TCHAR)), &dwDataRead, NULL) == FALSE)
				|| (WriteFile(hXMLGPOsFile, tEscapedUserExtensionsNames, (DWORD)(_tcslen(tEscapedUserExtensionsNames) * sizeof (WCHAR)), &dwDataRead, NULL) == FALSE)
				|| (WriteFile(hXMLGPOsFile, TEXT("</UserExtensionsNames>\r\n"), (DWORD)(_tcslen(TEXT("</UserExtensionsNames>\r\n")) * sizeof (TCHAR)), &dwDataRead, NULL) == FALSE))
				goto writerror;
		}
		if (pCurrGPOData->tVersionNumber != NULL)
		{
			if ((WriteFile(hXMLGPOsFile, TEXT("\t\t<Versionnumber>"), (DWORD)(_tcslen(TEXT("\t\t<Versionnumber>")) * sizeof (TCHAR)), &dwDataRead, NULL) == FALSE)
			|| (WriteFile(hXMLGPOsFile, tEscapedVersionNumber, (DWORD)(_tcslen(tEscapedVersionNumber) * sizeof (WCHAR)), &dwDataRead, NULL) == FALSE)
			|| (WriteFile(hXMLGPOsFile, TEXT("</Versionnumber>\r\n"), (DWORD)(_tcslen(TEXT("</Versionnumber>\r\n")) * sizeof (TCHAR)), &dwDataRead, NULL) == FALSE))
				goto writerror;
		}
		if (pCurrGPOData->tWhenChanged != NULL)
		{
			if ((WriteFile(hXMLGPOsFile, TEXT("\t\t<Whenchanged>"), (DWORD)(_tcslen(TEXT("\t\t<Whenchanged>")) * sizeof (TCHAR)), &dwDataRead, NULL) == FALSE)
			|| (WriteFile(hXMLGPOsFile, tEscapedWhenChanged, (DWORD)(_tcslen(tEscapedWhenChanged) * sizeof (WCHAR)), &dwDataRead, NULL) == FALSE)
			|| (WriteFile(hXMLGPOsFile, TEXT("</Whenchanged>\r\n"), (DWORD)(_tcslen(TEXT("</Whenchanged>\r\n")) * sizeof (TCHAR)), &dwDataRead, NULL) == FALSE))
				goto writerror;
		}
		if (pCurrGPOData->tWhenCreated != NULL)
		{
			if ((WriteFile(hXMLGPOsFile, TEXT("\t\t<Whencreated>"), (DWORD)(_tcslen(TEXT("\t\t<Whencreated>")) * sizeof (TCHAR)), &dwDataRead, NULL) == FALSE)
			|| (WriteFile(hXMLGPOsFile, tEscapedWhenCreated, (DWORD)(_tcslen(tEscapedWhenCreated) * sizeof (WCHAR)), &dwDataRead, NULL) == FALSE)
			|| (WriteFile(hXMLGPOsFile, TEXT("</Whencreated>\r\n"), (DWORD)(_tcslen(TEXT("</Whencreated>\r\n")) * sizeof (TCHAR)), &dwDataRead, NULL) == FALSE))
				goto writerror;
		}
		if (pCurrGPOData->tWQLFilter != NULL)
		{
			if ((WriteFile(hXMLGPOsFile, TEXT("\t\t<WMIFilter>"), (DWORD)(_tcslen(TEXT("\t\t<WMIFilter>")) * sizeof (TCHAR)), &dwDataRead, NULL) == FALSE)
				|| (WriteFile(hXMLGPOsFile, tEscapedWmiFilter, (DWORD)(_tcslen(tEscapedWmiFilter) * sizeof (WCHAR)), &dwDataRead, NULL) == FALSE)
				|| (WriteFile(hXMLGPOsFile, TEXT("</WMIFilter>\r\n"), (DWORD)(_tcslen(TEXT("</WMIFilter>\r\n")) * sizeof (TCHAR)), &dwDataRead, NULL) == FALSE))
				goto writerror;
		}
		if (pCurrGPOData->tSecurityDescriptor != NULL)
		{
			if ((WriteFile(hXMLGPOsFile, TEXT("\t\t<SecurityDescriptor>"), (DWORD)(_tcslen(TEXT("\t\t<SecurityDescriptor>")) * sizeof (TCHAR)), &dwDataRead, NULL) == FALSE)
				|| (WriteFile(hXMLGPOsFile, tEscapedSecurityDescriptor, (DWORD)(_tcslen(tEscapedSecurityDescriptor) * sizeof (WCHAR)), &dwDataRead, NULL) == FALSE)
				|| (WriteFile(hXMLGPOsFile, TEXT("</SecurityDescriptor>\r\n"), (DWORD)(_tcslen(TEXT("</SecurityDescriptor>\r\n")) * sizeof (TCHAR)), &dwDataRead, NULL) == FALSE))
				goto writerror;
		}

		if (WriteFile(hXMLGPOsFile, TEXT("\t</GPO>\r\n"), (DWORD)(_tcslen(TEXT("\t</GPO>\r\n")) * sizeof (TCHAR)), &dwDataRead, NULL) == FALSE)
			goto writerror;

		if (tEscapedDN)
			HeapFree(hCrawlerHeap, NULL, tEscapedDN);
		if (tEscapedCN)
			HeapFree(hCrawlerHeap, NULL, tEscapedCN);
		if (tEscapedCorePropagationData)
			HeapFree(hCrawlerHeap, NULL, tEscapedCorePropagationData);
		if (tEscapedDisplayName)
			HeapFree(hCrawlerHeap, NULL, tEscapedDisplayName);
		if (tEscapedSysPath)
			HeapFree(hCrawlerHeap, NULL, tEscapedSysPath);
		if (tEscapedFlags)
			HeapFree(hCrawlerHeap, NULL, tEscapedFlags);
		if (tEscapedFunctionalityVersion)
			HeapFree(hCrawlerHeap, NULL, tEscapedFunctionalityVersion);
		if (tEscapedMachineExtensionsNames)
			HeapFree(hCrawlerHeap, NULL, tEscapedMachineExtensionsNames);
		if (tEscapedUserExtensionsNames)
			HeapFree(hCrawlerHeap, NULL, tEscapedUserExtensionsNames);
		if (tEscapedVersionNumber)
			HeapFree(hCrawlerHeap, NULL, tEscapedVersionNumber);
		if (tEscapedWhenChanged)
			HeapFree(hCrawlerHeap, NULL, tEscapedWhenChanged);
		if (tEscapedWhenCreated)
			HeapFree(hCrawlerHeap, NULL, tEscapedWhenCreated);
		if (tEscapedWmiFilter)
			HeapFree(hCrawlerHeap, NULL, tEscapedWmiFilter);
		if (tEscapedSecurityDescriptor)
			HeapFree(hCrawlerHeap, NULL, tEscapedSecurityDescriptor);
	}

	CloseHandle(hXMLGPOsFile);
	return TRUE;

writerror:
	DEBUG_LOG(D_WARNING, "Unable to write XML DATA.\r\nExiting now...");
	DoExit(D_ERROR);
	return FALSE;
}

BOOL PrintCSVData(_In_ PLDAP_AD_INFOS pLdapADInfos)
{
	HANDLE hCSVUsersFile = GetFileHandle(OUTPUT_FILE_CSV, OUTPUT_DIRECTORY_LDAP_FILE, OUTPUT_NAME_USERS_FILE);
	HANDLE hCSVGroupsFile = GetFileHandle(OUTPUT_FILE_CSV, OUTPUT_DIRECTORY_LDAP_FILE, OUTPUT_NAME_GROUPS_FILE);
	HANDLE hCSVOusFile = GetFileHandle(OUTPUT_FILE_CSV, OUTPUT_DIRECTORY_LDAP_FILE, OUTPUT_NAME_OUS_FILE);
	HANDLE hCSVGPOsFile = GetFileHandle(OUTPUT_FILE_CSV, OUTPUT_DIRECTORY_LDAP_FILE, OUTPUT_NAME_GPOS_FILE);
	
	if (!pLdapADInfos || hCSVUsersFile == INVALID_HANDLE_VALUE || hCSVGroupsFile == INVALID_HANDLE_VALUE || hCSVOusFile == INVALID_HANDLE_VALUE || hCSVGPOsFile == INVALID_HANDLE_VALUE)
	{
		DEBUG_LOG(D_WARNING, "PLDAP_AD_INFOS or handle invalid for LDAP extract.\r\n");
		return(FALSE);
	}

	if (PrintCSVDataUsers(hCSVUsersFile, pLdapADInfos) != TRUE)
	{
		DEBUG_LOG(D_ERROR, "Unable to print CSV data for LDAP users.\r\n");
		return(FALSE);
	}

	if (PrintCSVDataGroups(hCSVGroupsFile, pLdapADInfos) != TRUE)
	{
		DEBUG_LOG(D_ERROR, "Unable to print CSV data for LDAP groups.\r\n");
		return(FALSE);
	}

	if (PrintCSVDataOUs(hCSVOusFile, pLdapADInfos) != TRUE)
	{
		DEBUG_LOG(D_ERROR, "Unable to print CSV data for LDAP OUs.\r\n");
		return(FALSE);
	}

	if (PrintCSVDataGPOs(hCSVGPOsFile, pLdapADInfos) != TRUE)
	{
		DEBUG_LOG(D_ERROR, "Unable to print CSV data for LDAP GPOs.\r\n");
		return(FALSE);
	}

	return TRUE;
}

BOOL PrintCSVDataUsers(_In_ HANDLE hCSVUsersFile, _In_ PLDAP_AD_INFOS pLdapADInfos)
{
	DWORD dwDataRead = 0;

	if (hCSVUsersFile == INVALID_HANDLE_VALUE || !pLdapADInfos)
		return FALSE;

	for (DWORD i = 0; i < pLdapADInfos->dwNumberOfUser; ++i)
	{
		PLDAP_AD_USER pCurrUserData = pLdapADInfos->pUsers[i];
		PTCHAR tEscapedDN = EscapeCSVString(pCurrUserData->tDistinguishedName);
		PTCHAR tEscapedCN = EscapeCSVString(pCurrUserData->tCN);
		PTCHAR tEscapedPrincipalName = EscapeCSVString(pCurrUserData->tUserPrincipalName);
		PTCHAR tEscapedLastLogon = EscapeCSVString(pCurrUserData->tLastLogon);
		PTCHAR tEscapedPwdLastSet = EscapeCSVString(pCurrUserData->tPwdLastSet);
		PTCHAR tEscapedMemberOf = EscapeCSVString(pCurrUserData->tMemberOf);
		PTCHAR tEscapedSecurityDescriptor = EscapeCSVString(pCurrUserData->tSecurityDescriptor);
		PTCHAR tEscapedSid = EscapeCSVString(pCurrUserData->tSid);

		if (!pCurrUserData->tDistinguishedName)
			continue;

		if (pCurrUserData->tDistinguishedName != NULL)
		{
			if ((WriteFile(hCSVUsersFile, tEscapedDN, (DWORD)(_tcslen(tEscapedDN) * sizeof (TCHAR)), &dwDataRead, NULL) == FALSE)
				|| (WriteFile(hCSVUsersFile, TEXT(";"), (DWORD)(_tcslen(TEXT(";")) * sizeof (TCHAR)), &dwDataRead, NULL) == FALSE))
				goto writerror;
		}
		else
		{
			if (WriteFile(hCSVUsersFile, TEXT(";"), (DWORD)(_tcslen(TEXT(";")) * sizeof (TCHAR)), &dwDataRead, NULL) == FALSE)
				goto writerror;
		}
		if (pCurrUserData->tCN != NULL)
		{
			if ((WriteFile(hCSVUsersFile, tEscapedCN, (DWORD)(_tcslen(tEscapedCN) * sizeof (TCHAR)), &dwDataRead, NULL) == FALSE)
				|| (WriteFile(hCSVUsersFile, TEXT(";"), (DWORD)(_tcslen(TEXT(";")) * sizeof (TCHAR)), &dwDataRead, NULL) == FALSE))
				goto writerror;
		}
		else
		{
			if (WriteFile(hCSVUsersFile, TEXT(";"), (DWORD)(_tcslen(TEXT(";")) * sizeof (TCHAR)), &dwDataRead, NULL) == FALSE)
				goto writerror;
		}
		if (pCurrUserData->tUserPrincipalName != NULL)
		{
			if ((WriteFile(hCSVUsersFile, tEscapedPrincipalName, (DWORD)(_tcslen(tEscapedPrincipalName) * sizeof (TCHAR)), &dwDataRead, NULL) == FALSE)
				|| (WriteFile(hCSVUsersFile, TEXT(";"), (DWORD)(_tcslen(TEXT(";")) * sizeof (TCHAR)), &dwDataRead, NULL) == FALSE))
				goto writerror;
		}
		else
		{
			if (WriteFile(hCSVUsersFile, TEXT(";"), (DWORD)(_tcslen(TEXT(";")) * sizeof (TCHAR)), &dwDataRead, NULL) == FALSE)
				goto writerror;
		}
		if (pCurrUserData->tLastLogon != NULL)
		{
			if ((WriteFile(hCSVUsersFile, tEscapedLastLogon, (DWORD)(_tcslen(tEscapedLastLogon) * sizeof (TCHAR)), &dwDataRead, NULL) == FALSE)
				|| (WriteFile(hCSVUsersFile, TEXT(";"), (DWORD)(_tcslen(TEXT(";")) * sizeof (TCHAR)), &dwDataRead, NULL) == FALSE))
				goto writerror;
		}
		else
		{
			if (WriteFile(hCSVUsersFile, TEXT(";"), (DWORD)(_tcslen(TEXT(";")) * sizeof (TCHAR)), &dwDataRead, NULL) == FALSE)
				goto writerror;
		}
		if (pCurrUserData->tPwdLastSet != NULL)
		{
			if ((WriteFile(hCSVUsersFile, tEscapedPwdLastSet, (DWORD)(_tcslen(tEscapedPwdLastSet) * sizeof (TCHAR)), &dwDataRead, NULL) == FALSE)
				|| (WriteFile(hCSVUsersFile, TEXT(";"), (DWORD)(_tcslen(TEXT(";")) * sizeof (TCHAR)), &dwDataRead, NULL) == FALSE))
				goto writerror;
		}
		else
		{
			if (WriteFile(hCSVUsersFile, TEXT(";"), (DWORD)(_tcslen(TEXT(";")) * sizeof (TCHAR)), &dwDataRead, NULL) == FALSE)
				goto writerror;
		}
		if (pCurrUserData->tMemberOf != NULL)
		{
			if ((WriteFile(hCSVUsersFile, tEscapedMemberOf, (DWORD)(_tcslen(tEscapedMemberOf) * sizeof (TCHAR)), &dwDataRead, NULL) == FALSE)
				|| (WriteFile(hCSVUsersFile, TEXT(";"), (DWORD)(_tcslen(TEXT(";")) * sizeof (TCHAR)), &dwDataRead, NULL) == FALSE))
				goto writerror;
		}
		else
		{
			if (WriteFile(hCSVUsersFile, TEXT(";"), (DWORD)(_tcslen(TEXT(";")) * sizeof (TCHAR)), &dwDataRead, NULL) == FALSE)
				goto writerror;
		}
		if (pCurrUserData->tSecurityDescriptor != NULL)
		{
			if ((WriteFile(hCSVUsersFile, tEscapedSecurityDescriptor, (DWORD)(_tcslen(tEscapedSecurityDescriptor) * sizeof (TCHAR)), &dwDataRead, NULL) == FALSE)
				|| (WriteFile(hCSVUsersFile, TEXT(";"), (DWORD)(_tcslen(TEXT(";")) * sizeof (TCHAR)), &dwDataRead, NULL) == FALSE))
				goto writerror;
		}
		else
		{
			if (WriteFile(hCSVUsersFile, TEXT(";"), (DWORD)(_tcslen(TEXT(";")) * sizeof (TCHAR)), &dwDataRead, NULL) == FALSE)
				goto writerror;
		}
		if (pCurrUserData->tSid != NULL)
		{
			if ((WriteFile(hCSVUsersFile, tEscapedSid, (DWORD)(_tcslen(tEscapedSid) * sizeof (TCHAR)), &dwDataRead, NULL) == FALSE)
				|| (WriteFile(hCSVUsersFile, TEXT(";"), (DWORD)(_tcslen(TEXT(";")) * sizeof (TCHAR)), &dwDataRead, NULL) == FALSE))
				goto writerror;
		}
		else
		{
			if (WriteFile(hCSVUsersFile, TEXT(";"), (DWORD)(_tcslen(TEXT(";")) * sizeof (TCHAR)), &dwDataRead, NULL) == FALSE)
				goto writerror;
		}

		if (WriteFile(hCSVUsersFile, TEXT("\r\n"), (DWORD)(_tcslen(TEXT("\r\n")) * sizeof (TCHAR)), &dwDataRead, NULL) == FALSE)
			goto writerror;

		if (tEscapedDN)
			HeapFree(hCrawlerHeap, NULL, tEscapedDN);
		if (tEscapedCN)
			HeapFree(hCrawlerHeap, NULL, tEscapedCN);
		if (tEscapedPrincipalName)
			HeapFree(hCrawlerHeap, NULL, tEscapedPrincipalName);
		if (tEscapedLastLogon)
			HeapFree(hCrawlerHeap, NULL, tEscapedLastLogon);
		if (tEscapedMemberOf)
			HeapFree(hCrawlerHeap, NULL, tEscapedMemberOf);
		if (tEscapedPwdLastSet)
			HeapFree(hCrawlerHeap, NULL, tEscapedPwdLastSet);
		if (tEscapedSecurityDescriptor)
			HeapFree(hCrawlerHeap, NULL, tEscapedSecurityDescriptor);
		if (tEscapedSid)
			HeapFree(hCrawlerHeap, NULL, tEscapedSid);
	}

	CloseHandle(hCSVUsersFile);
	return TRUE;

writerror:
	DEBUG_LOG(D_WARNING, "Unable to write XML DATA.\r\nExiting now...");
	DoExit(D_ERROR);
	return FALSE;
}

BOOL PrintCSVDataGroups(_In_ HANDLE hCSVGroupsFile, _In_ PLDAP_AD_INFOS pLdapADInfos)
{
	DWORD dwDataRead = 0;

	if (hCSVGroupsFile == INVALID_HANDLE_VALUE || !pLdapADInfos)
		return FALSE;

	for (DWORD i = 0; i < pLdapADInfos->dwNumberOfGroup; ++i)
	{
		PLDAP_AD_GROUP pCurrGroupData = pLdapADInfos->pGroups[i];
		PTCHAR tEscapedDN = EscapeCSVString(pCurrGroupData->tDistinguishedName);
		PTCHAR tEscapedCN = EscapeCSVString(pCurrGroupData->tCN);
		PTCHAR tEscapedDescription = EscapeCSVString(pCurrGroupData->tDescription);
		PTCHAR tEscapedMember = EscapeCSVString(pCurrGroupData->tMember);
		PTCHAR tEscapedGroupType = EscapeCSVString(pCurrGroupData->tGroupType);
		PTCHAR tEscapedSecurityDescriptor = EscapeCSVString(pCurrGroupData->tSecurityDescriptor);
		PTCHAR tEscapeSid = EscapeCSVString(pCurrGroupData->tSid);

		if (!pCurrGroupData->tDistinguishedName)
			continue;

		if (pCurrGroupData->tDistinguishedName != NULL)
		{
			if ((WriteFile(hCSVGroupsFile, tEscapedDN, (DWORD)(_tcslen(tEscapedDN) * sizeof (TCHAR)), &dwDataRead, NULL) == FALSE)
				|| (WriteFile(hCSVGroupsFile, TEXT(";"), (DWORD)(_tcslen(TEXT(";")) * sizeof (TCHAR)), &dwDataRead, NULL) == FALSE))
				goto writerror;
		}
		else
		{
			if (WriteFile(hCSVGroupsFile, TEXT(";"), (DWORD)(_tcslen(TEXT(";")) * sizeof (TCHAR)), &dwDataRead, NULL) == FALSE)
				goto writerror;
		}
		if (pCurrGroupData->tCN != NULL)
		{
			if ((WriteFile(hCSVGroupsFile, tEscapedCN, (DWORD)(_tcslen(tEscapedCN) * sizeof (TCHAR)), &dwDataRead, NULL) == FALSE)
				|| (WriteFile(hCSVGroupsFile, TEXT(";"), (DWORD)(_tcslen(TEXT(";")) * sizeof (TCHAR)), &dwDataRead, NULL) == FALSE))
				goto writerror;
		}
		else
		{
			if (WriteFile(hCSVGroupsFile, TEXT(";"), (DWORD)(_tcslen(TEXT(";")) * sizeof (TCHAR)), &dwDataRead, NULL) == FALSE)
				goto writerror;
		}
		if (pCurrGroupData->tDescription != NULL)
		{
			if ((WriteFile(hCSVGroupsFile, tEscapedDescription, (DWORD)(_tcslen(tEscapedDescription) * sizeof (TCHAR)), &dwDataRead, NULL) == FALSE)
				|| (WriteFile(hCSVGroupsFile, TEXT(";"), (DWORD)(_tcslen(TEXT(";")) * sizeof (TCHAR)), &dwDataRead, NULL) == FALSE))
				goto writerror;
		}
		else
		{
			if (WriteFile(hCSVGroupsFile, TEXT(";"), (DWORD)(_tcslen(TEXT(";")) * sizeof (TCHAR)), &dwDataRead, NULL) == FALSE)
				goto writerror;
		}
		if (pCurrGroupData->tMember != NULL)
		{
			if ((WriteFile(hCSVGroupsFile, tEscapedMember, (DWORD)(_tcslen(tEscapedMember) * sizeof (TCHAR)), &dwDataRead, NULL) == FALSE)
				|| (WriteFile(hCSVGroupsFile, TEXT(";"), (DWORD)(_tcslen(TEXT(";")) * sizeof (TCHAR)), &dwDataRead, NULL) == FALSE))
				goto writerror;
		}
		else
		{
			if (WriteFile(hCSVGroupsFile, TEXT(";"), (DWORD)(_tcslen(TEXT(";")) * sizeof (TCHAR)), &dwDataRead, NULL) == FALSE)
				goto writerror;
		}
		if (pCurrGroupData->tGroupType != NULL)
		{
			if ((WriteFile(hCSVGroupsFile, tEscapedGroupType, (DWORD)(_tcslen(tEscapedGroupType) * sizeof (TCHAR)), &dwDataRead, NULL) == FALSE)
				|| (WriteFile(hCSVGroupsFile, TEXT(";"), (DWORD)(_tcslen(TEXT(";")) * sizeof (TCHAR)), &dwDataRead, NULL) == FALSE))
				goto writerror;
		}
		else
		{
			if (WriteFile(hCSVGroupsFile, TEXT(";"), (DWORD)(_tcslen(TEXT(";")) * sizeof (TCHAR)), &dwDataRead, NULL) == FALSE)
				goto writerror;
		}
		if (pCurrGroupData->tSecurityDescriptor != NULL)
		{
			if ((WriteFile(hCSVGroupsFile, tEscapedSecurityDescriptor, (DWORD)(_tcslen(tEscapedSecurityDescriptor) * sizeof (TCHAR)), &dwDataRead, NULL) == FALSE)
				|| (WriteFile(hCSVGroupsFile, TEXT(";"), (DWORD)(_tcslen(TEXT(";")) * sizeof (TCHAR)), &dwDataRead, NULL) == FALSE))
				goto writerror;
		}
		else
		{
			if (WriteFile(hCSVGroupsFile, TEXT(";"), (DWORD)(_tcslen(TEXT(";")) * sizeof (TCHAR)), &dwDataRead, NULL) == FALSE)
				goto writerror;
		}
		if (pCurrGroupData->tSid != NULL)
		{
			if ((WriteFile(hCSVGroupsFile, tEscapeSid, (DWORD)(_tcslen(tEscapeSid) * sizeof (TCHAR)), &dwDataRead, NULL) == FALSE)
				|| (WriteFile(hCSVGroupsFile, TEXT(";"), (DWORD)(_tcslen(TEXT(";")) * sizeof (TCHAR)), &dwDataRead, NULL) == FALSE))
				goto writerror;
		}
		else
		{
			if (WriteFile(hCSVGroupsFile, TEXT(";"), (DWORD)(_tcslen(TEXT(";")) * sizeof (TCHAR)), &dwDataRead, NULL) == FALSE)
				goto writerror;
		}

		if (WriteFile(hCSVGroupsFile, TEXT("\r\n"), (DWORD)(_tcslen(TEXT("\r\n")) * sizeof (TCHAR)), &dwDataRead, NULL) == FALSE)
			goto writerror;

		if (tEscapedDN)
			HeapFree(hCrawlerHeap, NULL, tEscapedDN);
		if (tEscapedCN)
			HeapFree(hCrawlerHeap, NULL, tEscapedCN);
		if (tEscapedDescription)
			HeapFree(hCrawlerHeap, NULL, tEscapedDescription);
		if (tEscapedMember)
			HeapFree(hCrawlerHeap, NULL, tEscapedMember);
		if (tEscapedGroupType)
			HeapFree(hCrawlerHeap, NULL, tEscapedGroupType);
		if (tEscapedSecurityDescriptor)
			HeapFree(hCrawlerHeap, NULL, tEscapedSecurityDescriptor);
		if (tEscapeSid)
			HeapFree(hCrawlerHeap, NULL, tEscapeSid);
	}

	CloseHandle(hCSVGroupsFile);
	return TRUE;

writerror:
	DEBUG_LOG(D_WARNING, "Unable to write XML DATA.\r\nExiting now...");
	DoExit(D_ERROR);
	return FALSE;
}

BOOL PrintCSVDataOUs(_In_ HANDLE hCSVOusFile, _In_ PLDAP_AD_INFOS pLdapADInfos)
{
	DWORD dwDataRead = 0;

	if (hCSVOusFile == INVALID_HANDLE_VALUE || !pLdapADInfos)
		return FALSE;

	for (DWORD i = 0; i < pLdapADInfos->dwNumberOfOU; ++i)
	{
		PLDAP_AD_OU pCurrOUData = pLdapADInfos->pOUs[i];
		PTCHAR tEscapedDN = EscapeCSVString(pCurrOUData->tDistinguishedName);
		PTCHAR tEscapedDescription = EscapeCSVString(pCurrOUData->tDescription);
		PTCHAR tEscapedGpLink = EscapeCSVString(pCurrOUData->tGpLink);
		PTCHAR tEscapedGpOptions = EscapeCSVString(pCurrOUData->tGpOptions);
		PTCHAR tEscapedName = EscapeCSVString(pCurrOUData->tName);
		PTCHAR tEscapedOu = EscapeCSVString(pCurrOUData->tOu);
		PTCHAR tEscapedSecurityDescriptor = EscapeCSVString(pCurrOUData->tSecurityDescriptor);

		if (!pCurrOUData->tDistinguishedName)
			continue;

		if (pCurrOUData->tDistinguishedName != NULL)
		{
			if ((WriteFile(hCSVOusFile, tEscapedDN, (DWORD)(_tcslen(tEscapedDN) * sizeof (TCHAR)), &dwDataRead, NULL) == FALSE)
				|| (WriteFile(hCSVOusFile, TEXT(";"), (DWORD)(_tcslen(TEXT(";")) * sizeof (TCHAR)), &dwDataRead, NULL) == FALSE))
				goto writerror;
		}
		else
		{
			if (WriteFile(hCSVOusFile, TEXT(";"), (DWORD)(_tcslen(TEXT(";")) * sizeof (TCHAR)), &dwDataRead, NULL) == FALSE)
				goto writerror;
		}
		if (pCurrOUData->tOu != NULL)
		{
			if ((WriteFile(hCSVOusFile, tEscapedOu, (DWORD)(_tcslen(tEscapedOu) * sizeof (TCHAR)), &dwDataRead, NULL) == FALSE)
				|| (WriteFile(hCSVOusFile, TEXT(";"), (DWORD)(_tcslen(TEXT(";")) * sizeof (TCHAR)), &dwDataRead, NULL) == FALSE))
				goto writerror;
		}
		else
		{
			if (WriteFile(hCSVOusFile, TEXT(";"), (DWORD)(_tcslen(TEXT(";")) * sizeof (TCHAR)), &dwDataRead, NULL) == FALSE)
				goto writerror;
		}
		if (pCurrOUData->tDescription != NULL)
		{
			if ((WriteFile(hCSVOusFile, tEscapedDescription, (DWORD)(_tcslen(tEscapedDescription) * sizeof (TCHAR)), &dwDataRead, NULL) == FALSE)
				|| (WriteFile(hCSVOusFile, TEXT(";"), (DWORD)(_tcslen(TEXT(";")) * sizeof (TCHAR)), &dwDataRead, NULL) == FALSE))
				goto writerror;
		}
		else
		{
			if (WriteFile(hCSVOusFile, TEXT(";"), (DWORD)(_tcslen(TEXT(";")) * sizeof (TCHAR)), &dwDataRead, NULL) == FALSE)
				goto writerror;
		}
		if (pCurrOUData->tName != NULL)
		{
			if ((WriteFile(hCSVOusFile, tEscapedName, (DWORD)(_tcslen(tEscapedName) * sizeof (TCHAR)), &dwDataRead, NULL) == FALSE)
				|| (WriteFile(hCSVOusFile, TEXT(";"), (DWORD)(_tcslen(TEXT(";")) * sizeof (TCHAR)), &dwDataRead, NULL) == FALSE))
				goto writerror;
		}
		else
		{
			if (WriteFile(hCSVOusFile, TEXT(";"), (DWORD)(_tcslen(TEXT(";")) * sizeof (TCHAR)), &dwDataRead, NULL) == FALSE)
				goto writerror;
		}
		if (pCurrOUData->tGpLink != NULL)
		{
			if ((WriteFile(hCSVOusFile, tEscapedGpLink, (DWORD)(_tcslen(tEscapedGpLink) * sizeof (TCHAR)), &dwDataRead, NULL) == FALSE)
				|| (WriteFile(hCSVOusFile, TEXT(";"), (DWORD)(_tcslen(TEXT(";")) * sizeof (TCHAR)), &dwDataRead, NULL) == FALSE))
				goto writerror;
		}
		else
		{
			if (WriteFile(hCSVOusFile, TEXT(";"), (DWORD)(_tcslen(TEXT(";")) * sizeof (TCHAR)), &dwDataRead, NULL) == FALSE)
				goto writerror;
		}
		if (pCurrOUData->tGpOptions != NULL)
		{
			if ((WriteFile(hCSVOusFile, tEscapedGpOptions, (DWORD)(_tcslen(tEscapedGpOptions) * sizeof (TCHAR)), &dwDataRead, NULL) == FALSE)
				|| (WriteFile(hCSVOusFile, TEXT(";"), (DWORD)(_tcslen(TEXT(";")) * sizeof (TCHAR)), &dwDataRead, NULL) == FALSE))
				goto writerror;
		}
		else
		{
			if (WriteFile(hCSVOusFile, TEXT(";"), (DWORD)(_tcslen(TEXT(";")) * sizeof (TCHAR)), &dwDataRead, NULL) == FALSE)
				goto writerror;
		}
		if (pCurrOUData->tSecurityDescriptor != NULL)
		{
			if ((WriteFile(hCSVOusFile, tEscapedSecurityDescriptor, (DWORD)(_tcslen(tEscapedSecurityDescriptor) * sizeof (TCHAR)), &dwDataRead, NULL) == FALSE)
				|| (WriteFile(hCSVOusFile, TEXT(";"), (DWORD)(_tcslen(TEXT(";")) * sizeof (TCHAR)), &dwDataRead, NULL) == FALSE))
				goto writerror;
		}
		else
		{
			if (WriteFile(hCSVOusFile, TEXT(";"), (DWORD)(_tcslen(TEXT(";")) * sizeof (TCHAR)), &dwDataRead, NULL) == FALSE)
				goto writerror;
		}

		if (WriteFile(hCSVOusFile, TEXT("\r\n"), (DWORD)(_tcslen(TEXT("\r\n")) * sizeof (TCHAR)), &dwDataRead, NULL) == FALSE)
			goto writerror;

		if (tEscapedDN)
			HeapFree(hCrawlerHeap, NULL, tEscapedDN);
		if (tEscapedDescription)
			HeapFree(hCrawlerHeap, NULL, tEscapedDescription);
		if (tEscapedGpLink)
			HeapFree(hCrawlerHeap, NULL, tEscapedGpLink);
		if (tEscapedGpOptions)
			HeapFree(hCrawlerHeap, NULL, tEscapedGpOptions);
		if (tEscapedName)
			HeapFree(hCrawlerHeap, NULL, tEscapedName);
		if (tEscapedOu)
			HeapFree(hCrawlerHeap, NULL, tEscapedOu);
		if (tEscapedSecurityDescriptor)
			HeapFree(hCrawlerHeap, NULL, tEscapedSecurityDescriptor);
	}

	CloseHandle(hCSVOusFile);
	return TRUE;

writerror:
	DEBUG_LOG(D_WARNING, "Unable to write XML DATA.\r\nExiting now...");
	DoExit(D_ERROR);
	return FALSE;
}

BOOL PrintCSVDataGPOs(_In_ HANDLE hCSVGPOsFile, _In_ PLDAP_AD_INFOS pLdapADInfos)
{
	DWORD dwDataRead = 0;

	if (hCSVGPOsFile == INVALID_HANDLE_VALUE || !pLdapADInfos)
		return FALSE;

	for (DWORD i = 0; i < pLdapADInfos->dwNumberOfGPO; ++i)
	{
		PLDAP_AD_GPO pCurrGPOData = pLdapADInfos->pGPOs[i];
		PTCHAR tEscapedDN = EscapeCSVString(pCurrGPOData->tDistinguishedName);
		PTCHAR tEscapedCN = EscapeCSVString(pCurrGPOData->tCN);
		PTCHAR tEscapedCorePropagationData = EscapeCSVString(pCurrGPOData->tCorePropagationData);
		PTCHAR tEscapedDisplayName = EscapeCSVString(pCurrGPOData->tDisplayName);
		PTCHAR tEscapedSysPath = EscapeCSVString(pCurrGPOData->tFileSysPath);
		PTCHAR tEscapedFlags = EscapeCSVString(pCurrGPOData->tFlags);
		PTCHAR tEscapedFunctionalityVersion = EscapeCSVString(pCurrGPOData->tFunctionalityVersion);
		PTCHAR tEscapedMachineExtensionsNames = EscapeCSVString(pCurrGPOData->tMachineExtensionsNames);
		PTCHAR tEscapedUserExtensionsNames = EscapeCSVString(pCurrGPOData->tUserExtensionsNames);
		PTCHAR tEscapedVersionNumber = EscapeCSVString(pCurrGPOData->tVersionNumber);
		PTCHAR tEscapedWhenChanged = EscapeCSVString(pCurrGPOData->tWhenChanged);
		PTCHAR tEscapedWhenCreated = EscapeCSVString(pCurrGPOData->tWhenCreated);
		PTCHAR tEscapedWmiFilter = EscapeCSVString(pCurrGPOData->tWQLFilter);
		PTCHAR tEscapedSecurityDescriptor = EscapeCSVString(pCurrGPOData->tSecurityDescriptor);

		if (!pCurrGPOData->tDistinguishedName)
			continue;

		if (pCurrGPOData->tDistinguishedName != NULL)
		{
			if ((WriteFile(hCSVGPOsFile, tEscapedDN, (DWORD)(_tcslen(tEscapedDN) * sizeof (TCHAR)), &dwDataRead, NULL) == FALSE)
				|| (WriteFile(hCSVGPOsFile, TEXT(";"), (DWORD)(_tcslen(TEXT(";")) * sizeof (TCHAR)), &dwDataRead, NULL) == FALSE))
				goto writerror;
		}
		else
		{
			if (WriteFile(hCSVGPOsFile, TEXT(";"), (DWORD)(_tcslen(TEXT(";")) * sizeof (TCHAR)), &dwDataRead, NULL) == FALSE)
				goto writerror;
		}
		if (pCurrGPOData->tCN != NULL)
		{
			if ((WriteFile(hCSVGPOsFile, tEscapedCN, (DWORD)(_tcslen(tEscapedCN) * sizeof (TCHAR)), &dwDataRead, NULL) == FALSE)
				|| (WriteFile(hCSVGPOsFile, TEXT(";"), (DWORD)(_tcslen(TEXT(";")) * sizeof (TCHAR)), &dwDataRead, NULL) == FALSE))
				goto writerror;
		}
		else
		{
			if (WriteFile(hCSVGPOsFile, TEXT(";"), (DWORD)(_tcslen(TEXT(";")) * sizeof (TCHAR)), &dwDataRead, NULL) == FALSE)
				goto writerror;
		}
		if (pCurrGPOData->tWhenCreated != NULL)
		{
			if ((WriteFile(hCSVGPOsFile, tEscapedWhenCreated, (DWORD)(_tcslen(tEscapedWhenCreated) * sizeof (TCHAR)), &dwDataRead, NULL) == FALSE)
				|| (WriteFile(hCSVGPOsFile, TEXT(";"), (DWORD)(_tcslen(TEXT(";")) * sizeof (TCHAR)), &dwDataRead, NULL) == FALSE))
				goto writerror;
		}
		else
		{
			if (WriteFile(hCSVGPOsFile, TEXT(";"), (DWORD)(_tcslen(TEXT(";")) * sizeof (TCHAR)), &dwDataRead, NULL) == FALSE)
				goto writerror;
		}
		if (pCurrGPOData->tWhenChanged != NULL)
		{
			if ((WriteFile(hCSVGPOsFile, tEscapedWhenChanged, (DWORD)(_tcslen(tEscapedWhenChanged) * sizeof (TCHAR)), &dwDataRead, NULL) == FALSE)
				|| (WriteFile(hCSVGPOsFile, TEXT(";"), (DWORD)(_tcslen(TEXT(";")) * sizeof (TCHAR)), &dwDataRead, NULL) == FALSE))
				goto writerror;
		}
		else
		{
			if (WriteFile(hCSVGPOsFile, TEXT(";"), (DWORD)(_tcslen(TEXT(";")) * sizeof (TCHAR)), &dwDataRead, NULL) == FALSE)
				goto writerror;
		}
		if (pCurrGPOData->tDisplayName != NULL)
		{
			if ((WriteFile(hCSVGPOsFile, tEscapedDisplayName, (DWORD)(_tcslen(tEscapedDisplayName) * sizeof (TCHAR)), &dwDataRead, NULL) == FALSE)
				|| (WriteFile(hCSVGPOsFile, TEXT(";"), (DWORD)(_tcslen(TEXT(";")) * sizeof (TCHAR)), &dwDataRead, NULL) == FALSE))
				goto writerror;
		}
		else
		{
			if (WriteFile(hCSVGPOsFile, TEXT(";"), (DWORD)(_tcslen(TEXT(";")) * sizeof (TCHAR)), &dwDataRead, NULL) == FALSE)
				goto writerror;
		}
		if (pCurrGPOData->tFlags != NULL)
		{
			if ((WriteFile(hCSVGPOsFile, tEscapedFlags, (DWORD)(_tcslen(tEscapedFlags) * sizeof (TCHAR)), &dwDataRead, NULL) == FALSE)
				|| (WriteFile(hCSVGPOsFile, TEXT(";"), (DWORD)(_tcslen(TEXT(";")) * sizeof (TCHAR)), &dwDataRead, NULL) == FALSE))
				goto writerror;
		}
		else
		{
			if (WriteFile(hCSVGPOsFile, TEXT(";"), (DWORD)(_tcslen(TEXT(";")) * sizeof (TCHAR)), &dwDataRead, NULL) == FALSE)
				goto writerror;
		}
		if (pCurrGPOData->tCorePropagationData != NULL)
		{
			if ((WriteFile(hCSVGPOsFile, tEscapedCorePropagationData, (DWORD)(_tcslen(tEscapedCorePropagationData) * sizeof (TCHAR)), &dwDataRead, NULL) == FALSE)
				|| (WriteFile(hCSVGPOsFile, TEXT(";"), (DWORD)(_tcslen(TEXT(";")) * sizeof (TCHAR)), &dwDataRead, NULL) == FALSE))
				goto writerror;
		}
		else
		{
			if (WriteFile(hCSVGPOsFile, TEXT(";"), (DWORD)(_tcslen(TEXT(";")) * sizeof (TCHAR)), &dwDataRead, NULL) == FALSE)
				goto writerror;
		}
		if (pCurrGPOData->tFileSysPath != NULL)
		{
			if ((WriteFile(hCSVGPOsFile, tEscapedSysPath, (DWORD)(_tcslen(tEscapedSysPath) * sizeof (TCHAR)), &dwDataRead, NULL) == FALSE)
				|| (WriteFile(hCSVGPOsFile, TEXT(";"), (DWORD)(_tcslen(TEXT(";")) * sizeof (TCHAR)), &dwDataRead, NULL) == FALSE))
				goto writerror;
		}
		else
		{
			if (WriteFile(hCSVGPOsFile, TEXT(";"), (DWORD)(_tcslen(TEXT(";")) * sizeof (TCHAR)), &dwDataRead, NULL) == FALSE)
				goto writerror;
		}
		if (pCurrGPOData->tFunctionalityVersion != NULL)
		{
			if ((WriteFile(hCSVGPOsFile, tEscapedFunctionalityVersion, (DWORD)(_tcslen(tEscapedFunctionalityVersion) * sizeof (TCHAR)), &dwDataRead, NULL) == FALSE)
				|| (WriteFile(hCSVGPOsFile, TEXT(";"), (DWORD)(_tcslen(TEXT(";")) * sizeof (TCHAR)), &dwDataRead, NULL) == FALSE))
				goto writerror;
		}
		else
		{
			if (WriteFile(hCSVGPOsFile, TEXT(";"), (DWORD)(_tcslen(TEXT(";")) * sizeof (TCHAR)), &dwDataRead, NULL) == FALSE)
				goto writerror;
		}
		if (pCurrGPOData->tMachineExtensionsNames != NULL)
		{
			if ((WriteFile(hCSVGPOsFile, tEscapedMachineExtensionsNames, (DWORD)(_tcslen(tEscapedMachineExtensionsNames) * sizeof (TCHAR)), &dwDataRead, NULL) == FALSE)
				|| (WriteFile(hCSVGPOsFile, TEXT(";"), (DWORD)(_tcslen(TEXT(";")) * sizeof (TCHAR)), &dwDataRead, NULL) == FALSE))
				goto writerror;
		}
		else
		{
			if (WriteFile(hCSVGPOsFile, TEXT(";"), (DWORD)(_tcslen(TEXT(";")) * sizeof (TCHAR)), &dwDataRead, NULL) == FALSE)
				goto writerror;
		}
		if (pCurrGPOData->tUserExtensionsNames != NULL)
		{
			if ((WriteFile(hCSVGPOsFile, tEscapedUserExtensionsNames, (DWORD)(_tcslen(tEscapedUserExtensionsNames) * sizeof (TCHAR)), &dwDataRead, NULL) == FALSE)
				|| (WriteFile(hCSVGPOsFile, TEXT(";"), (DWORD)(_tcslen(TEXT(";")) * sizeof (TCHAR)), &dwDataRead, NULL) == FALSE))
				goto writerror;
		}
		else
		{
			if (WriteFile(hCSVGPOsFile, TEXT(";"), (DWORD)(_tcslen(TEXT(";")) * sizeof (TCHAR)), &dwDataRead, NULL) == FALSE)
				goto writerror;
		}
		if (pCurrGPOData->tVersionNumber != NULL)
		{
			if ((WriteFile(hCSVGPOsFile, tEscapedVersionNumber, (DWORD)(_tcslen(tEscapedVersionNumber) * sizeof (TCHAR)), &dwDataRead, NULL) == FALSE)
				|| (WriteFile(hCSVGPOsFile, TEXT(";"), (DWORD)(_tcslen(TEXT(";")) * sizeof (TCHAR)), &dwDataRead, NULL) == FALSE))
				goto writerror;
		}
		else
		{
			if (WriteFile(hCSVGPOsFile, TEXT(";"), (DWORD)(_tcslen(TEXT(";")) * sizeof (TCHAR)), &dwDataRead, NULL) == FALSE)
				goto writerror;
		}
		if (pCurrGPOData->tWQLFilter != NULL)
		{
			if ((WriteFile(hCSVGPOsFile, tEscapedWmiFilter, (DWORD)(_tcslen(tEscapedWmiFilter) * sizeof (TCHAR)), &dwDataRead, NULL) == FALSE)
				|| (WriteFile(hCSVGPOsFile, TEXT(";"), (DWORD)(_tcslen(TEXT(";")) * sizeof (TCHAR)), &dwDataRead, NULL) == FALSE))
				goto writerror;
		}
		else
		{
			if (WriteFile(hCSVGPOsFile, TEXT(";"), (DWORD)(_tcslen(TEXT(";")) * sizeof (TCHAR)), &dwDataRead, NULL) == FALSE)
				goto writerror;
		}
		if (pCurrGPOData->tSecurityDescriptor != NULL)
		{
			if ((WriteFile(hCSVGPOsFile, tEscapedSecurityDescriptor, (DWORD)(_tcslen(tEscapedSecurityDescriptor) * sizeof (TCHAR)), &dwDataRead, NULL) == FALSE)
				|| (WriteFile(hCSVGPOsFile, TEXT(";"), (DWORD)(_tcslen(TEXT(";")) * sizeof (TCHAR)), &dwDataRead, NULL) == FALSE))
				goto writerror;
		}
		else
		{
			if (WriteFile(hCSVGPOsFile, TEXT(";"), (DWORD)(_tcslen(TEXT(";")) * sizeof (TCHAR)), &dwDataRead, NULL) == FALSE)
				goto writerror;
		}

		if (WriteFile(hCSVGPOsFile, TEXT("\r\n"), (DWORD)(_tcslen(TEXT("\r\n")) * sizeof (TCHAR)), &dwDataRead, NULL) == FALSE)
			goto writerror;

		if (tEscapedDN)
			HeapFree(hCrawlerHeap, NULL, tEscapedDN);
		if (tEscapedCN)
			HeapFree(hCrawlerHeap, NULL, tEscapedCN);
		if (tEscapedCorePropagationData)
			HeapFree(hCrawlerHeap, NULL, tEscapedCorePropagationData);
		if (tEscapedDisplayName)
			HeapFree(hCrawlerHeap, NULL, tEscapedDisplayName);
		if (tEscapedSysPath)
			HeapFree(hCrawlerHeap, NULL, tEscapedSysPath);
		if (tEscapedFlags)
			HeapFree(hCrawlerHeap, NULL, tEscapedFlags);
		if (tEscapedFunctionalityVersion)
			HeapFree(hCrawlerHeap, NULL, tEscapedFunctionalityVersion);
		if (tEscapedMachineExtensionsNames)
			HeapFree(hCrawlerHeap, NULL, tEscapedMachineExtensionsNames);
		if (tEscapedUserExtensionsNames)
			HeapFree(hCrawlerHeap, NULL, tEscapedUserExtensionsNames);
		if (tEscapedVersionNumber)
			HeapFree(hCrawlerHeap, NULL, tEscapedVersionNumber);
		if (tEscapedWhenChanged)
			HeapFree(hCrawlerHeap, NULL, tEscapedWhenChanged);
		if (tEscapedWhenCreated)
			HeapFree(hCrawlerHeap, NULL, tEscapedWhenCreated);
		if (tEscapedWmiFilter)
			HeapFree(hCrawlerHeap, NULL, tEscapedWmiFilter);
		if (tEscapedSecurityDescriptor)
			HeapFree(hCrawlerHeap, NULL, tEscapedSecurityDescriptor);
	}

	CloseHandle(hCSVGPOsFile);
	return TRUE;

writerror:
	DEBUG_LOG(D_WARNING, "Unable to write XML DATA.\r\nExiting now...");
	DoExit(D_ERROR);
	return FALSE;
}

BOOL PrintSTDOUTData(_In_ PLDAP_AD_INFOS pLdapADInfos)
{
	if (PrintSTDOUTDataUsers(pLdapADInfos) != TRUE)
	{
		DEBUG_LOG(D_ERROR, "Unable to print STDOUT data for LDAP users.\r\n");
		return(FALSE);
	}

	if (PrintSTDOUTDataGroups(pLdapADInfos) != TRUE)
	{
		DEBUG_LOG(D_ERROR, "Unable to print STDOUT data for LDAP groups.\r\n");
		return(FALSE);
	}

	if (PrintSTDOUTDataOUs(pLdapADInfos) != TRUE)
	{
		DEBUG_LOG(D_ERROR, "Unable to print STDOUT data for LDAP OUs.\r\n");
		return(FALSE);
	}

	if (PrintSTDOUTDataGPOs(pLdapADInfos) != TRUE)
	{
		DEBUG_LOG(D_ERROR, "Unable to print STDOUT data for LDAP GPO.\r\n");
		return(FALSE);
	}

	return TRUE;
}

BOOL PrintSTDOUTDataUsers(_In_ PLDAP_AD_INFOS pLdapADInfos)
{

	for (DWORD i = 0; i < pLdapADInfos->dwNumberOfUser; ++i)
	{
		PLDAP_AD_USER pCurrUserData = pLdapADInfos->pUsers[i];

		if (!pCurrUserData->tDistinguishedName)
			continue;

		printf("[USER] DistinguishedName=%ws CN=%ws UserPrincipalName=%ws LastLogon=%ws PwdLastSet=%ws MemberOf=%ws SecurityDescriptor=%ws SID=%ws\r\n", pCurrUserData->tDistinguishedName, pCurrUserData->tCN, pCurrUserData->tUserPrincipalName, pCurrUserData->tLastLogon, pCurrUserData->tPwdLastSet, pCurrUserData->tMemberOf, pCurrUserData->tSecurityDescriptor, pCurrUserData->tSid);
	}

	return TRUE;
}

BOOL PrintSTDOUTDataGroups(_In_ PLDAP_AD_INFOS pLdapADInfos)
{

	for (DWORD i = 0; i < pLdapADInfos->dwNumberOfGroup; ++i)
	{
		PLDAP_AD_GROUP pCurrGroupData = pLdapADInfos->pGroups[i];

		if (!pCurrGroupData->tDistinguishedName)
			continue;

		printf("[GROUP] DistinguishedName=%ws CN=%ws Description=%ws Members=%ws GroupType=%ws SecurityDescriptor=%ws SID=%ws\r\n", pCurrGroupData->tDistinguishedName, pCurrGroupData->tCN, pCurrGroupData->tDescription, pCurrGroupData->tMember, pCurrGroupData->tGroupType, pCurrGroupData->tSecurityDescriptor, pCurrGroupData->tSid);
	}

	return TRUE;
}

BOOL PrintSTDOUTDataOUs(_In_ PLDAP_AD_INFOS pLdapADInfos)
{

	for (DWORD i = 0; i < pLdapADInfos->dwNumberOfOU; ++i)
	{
		PLDAP_AD_OU pCurrOuData = pLdapADInfos->pOUs[i];

		if (!pCurrOuData->tDistinguishedName)
			continue;

		printf("[OU] DistinguishedName=%ws OU=%ws Description=%ws Name=%ws GPLink=%ws GPOptions=%ws SecurityDescriptor=%ws\r\n", pCurrOuData->tDistinguishedName, pCurrOuData->tOu, pCurrOuData->tDescription, pCurrOuData->tName, pCurrOuData->tGpLink, pCurrOuData->tGpOptions, pCurrOuData->tSecurityDescriptor);
	}

	return TRUE;
}

BOOL PrintSTDOUTDataGPOs(_In_ PLDAP_AD_INFOS pLdapADInfos)
{

	for (DWORD i = 0; i < pLdapADInfos->dwNumberOfGPO; ++i)
	{
		PLDAP_AD_GPO pCurrGpoData = pLdapADInfos->pGPOs[i];

		if (!pCurrGpoData->tDistinguishedName)
			continue;

		printf("[GPO] DistinguishedName=%ws CN=%ws CreatedDate=%ws ChangeDate=%ws DisplayName=%ws Flags=%ws CorePropagationData=%ws FileSysPath=%ws FunctionalityVersion=%ws MachineExtensionName=%ws UserExtensionName=%ws VersionNumber=%ws WMIFilter=%ws SecurityDescriptor=%ws\r\n", pCurrGpoData->tDistinguishedName, pCurrGpoData->tCN, pCurrGpoData->tWhenChanged, pCurrGpoData->tWhenChanged, pCurrGpoData->tDisplayName, pCurrGpoData->tFlags, pCurrGpoData->tCorePropagationData, pCurrGpoData->tFileSysPath, pCurrGpoData->tFunctionalityVersion, pCurrGpoData->tMachineExtensionsNames, pCurrGpoData->tUserExtensionsNames, pCurrGpoData->tVersionNumber, pCurrGpoData->tWQLFilter, pCurrGpoData->tSecurityDescriptor);
	}

	return TRUE;
}
