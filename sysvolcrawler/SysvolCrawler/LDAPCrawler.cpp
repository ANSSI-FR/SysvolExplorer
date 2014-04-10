/**************************************************
* SysvolCrawler - LDAPBrowser.cpp
* AUTHOR: Luc Delsalle	
*
* Extract GPO metadata from LDAP Directory
*	
* ANSSI/COSSI/DTO/BAI - 2014
***************************************************/

#include "LDAPCrawler.h"

PLDAP_CONNECT_INFO pLDAPConnectInfo = NULL;

BOOL InitToLDAP(_In_ PTCHAR ptHostName, _In_ ULONG dwPortNumber)
{
	DWORD dwRes = LDAP_SUCCESS;
	ULONG version = LDAP_VERSION3;
	ULONG size = LDAP_NO_LIMIT;
	ULONG time = LDAP_NO_LIMIT;
	ULONG ssl = (ULONG) LDAP_OPT_ON;

	if (!ptHostName || (dwPortNumber == 0))
	{
		DEBUG_LOG(D_ERROR, "LDAP credentials invalid.\r\n");
		return FALSE;
	}

	if (pLDAPConnectInfo)
	{
		DEBUG_LOG(D_WARNING, "An LDAP connection seems to be already registered.\r\n");
		return TRUE;
	}

	pLDAPConnectInfo = (PLDAP_CONNECT_INFO) HeapAlloc(hCrawlerHeap, HEAP_ZERO_MEMORY, sizeof(LDAP_CONNECT_INFOS));
	if (!pLDAPConnectInfo)
	{
		DEBUG_LOG(D_WARNING, "Unable to allocate LDAP_CONNECT_INFOS structure.\r\n");
		return FALSE;
	}
	pLDAPConnectInfo->hLDAPConnection = NULL;
	pLDAPConnectInfo->ptLDAPDomainDN = NULL;

	// Connection initialization
	pLDAPConnectInfo->hLDAPConnection = ldap_init(ptHostName, dwPortNumber);
	if (!pLDAPConnectInfo->hLDAPConnection)
	{
		DEBUG_LOG(D_ERROR, "Unable to init LDAP connection for %s:%d (ErrorCode: 0x%x).\r\n", ptHostName, dwPortNumber, LdapGetLastError());
		if (pLDAPConnectInfo->hLDAPConnection)
			ldap_unbind(pLDAPConnectInfo->hLDAPConnection);
		return FALSE;
	}

	// Define connection options
	dwRes = ldap_set_option(pLDAPConnectInfo->hLDAPConnection, LDAP_OPT_PROTOCOL_VERSION, (void*) &version);
	if(dwRes != LDAP_SUCCESS)
	{
		DEBUG_LOG(D_ERROR, "Unable to set LDAP protocole option (ErrorCode: 0x%0lX).\r\n", dwRes);
		if (pLDAPConnectInfo->hLDAPConnection)
			ldap_unbind(pLDAPConnectInfo->hLDAPConnection);
		return FALSE;
	}
	dwRes = ldap_set_option(pLDAPConnectInfo->hLDAPConnection, LDAP_OPT_SIZELIMIT, (void*) &size);
	if(dwRes != LDAP_SUCCESS)
	{
		DEBUG_LOG(D_ERROR, "Unable to set LDAP size option (ErrorCode: 0x%0lX).\r\n", dwRes);
		if (pLDAPConnectInfo->hLDAPConnection)
			ldap_unbind(pLDAPConnectInfo->hLDAPConnection);
		return FALSE;
	}
	dwRes = ldap_set_option(pLDAPConnectInfo->hLDAPConnection, LDAP_OPT_TIMELIMIT, (void*) &time);
	if(dwRes != LDAP_SUCCESS)
	{
		DEBUG_LOG(D_ERROR, "Unable to set LDAP size option (ErrorCode: 0x%0lX).\r\n", dwRes);
		if (pLDAPConnectInfo->hLDAPConnection)
			ldap_unbind(pLDAPConnectInfo->hLDAPConnection);
		return FALSE;
	}

	// Connect to server
	dwRes = ldap_connect(pLDAPConnectInfo->hLDAPConnection, NULL);
	if (dwRes != LDAP_SUCCESS)
	{
		DEBUG_LOG(D_ERROR, "Unable to connect to LDAP server (ErrorCode: 0x%0lX).\r\n", dwRes);
		if (pLDAPConnectInfo->hLDAPConnection)
			ldap_unbind(pLDAPConnectInfo->hLDAPConnection);
      return FALSE;
	}
    
	return TRUE;
}

BOOL BindToLDAP(_In_ PTCHAR ptUserName, _In_ PTCHAR ptPassword)
{
	DWORD dwRes = LDAP_SUCCESS;
	SEC_WINNT_AUTH_IDENTITY sSecIdent;

	if (!pLDAPConnectInfo || !pLDAPConnectInfo->hLDAPConnection || !pLDAPConnectInfo->ptLDAPDomainName)
	{
		DEBUG_LOG(D_ERROR, "LDAP connection invalid.\r\n");
		return FALSE;
	}

	if (ptUserName == NULL || ptPassword == NULL)
	{
		sSecIdent.User = NULL;
		sSecIdent.UserLength = 0;
		sSecIdent.Password = NULL;
		sSecIdent.PasswordLength = 0;
		sSecIdent.Domain = NULL;
		sSecIdent.DomainLength = 0;
		sSecIdent.Flags = SEC_WINNT_AUTH_IDENTITY_ANSI;
	}
	else
	{
		// Init bind data
		sSecIdent.User = (unsigned short*)PtcharToCStr(ptUserName);
		sSecIdent.UserLength = (unsigned long)_tcslen(ptUserName);
		sSecIdent.Password = (unsigned short*)PtcharToCStr(ptPassword);
		sSecIdent.PasswordLength = (unsigned long)_tcslen(ptPassword);
		sSecIdent.Domain = (unsigned short*)PtcharToCStr(pLDAPConnectInfo->ptLDAPDomainName);
		sSecIdent.DomainLength = (unsigned long)_tcslen(pLDAPConnectInfo->ptLDAPDomainName);
		sSecIdent.Flags = SEC_WINNT_AUTH_IDENTITY_ANSI;
	}


	// Server bind
	dwRes = ldap_bind_s(pLDAPConnectInfo->hLDAPConnection, pLDAPConnectInfo->ptLDAPDomainDN, (PTCHAR)&sSecIdent, LDAP_AUTH_NEGOTIATE);
	if (dwRes != LDAP_SUCCESS)
	{
		DEBUG_LOG(D_ERROR, "Unable to bind to LDAP server (ErrorCode: 0x%0lX).\r\n", dwRes);
		if (pLDAPConnectInfo->hLDAPConnection)
			ldap_unbind(pLDAPConnectInfo->hLDAPConnection);
      return FALSE;
	}

	if (sSecIdent.User)
		HeapFree(hCrawlerHeap, NULL, sSecIdent.User);
	if (sSecIdent.Password)
		HeapFree(hCrawlerHeap, NULL, sSecIdent.Password);
	if (sSecIdent.Domain)
		HeapFree(hCrawlerHeap, NULL, sSecIdent.Domain);

	return TRUE;
}

BOOL ExtractDomainNamingContext()
{
	PLDAPMessage plmsgSearchResponse = NULL;
	LDAPMessage* pFirstEntry = NULL;
	DWORD dwRes = LDAP_SUCCESS;
	PTCHAR *pNamingContextTmp = NULL, *pLdapServiceName = NULL;
	DWORD dwStrSize = 0, dnsDomainLen = 0;
	BOOL bIsMissingDnsName = FALSE;

	if (!pLDAPConnectInfo || !pLDAPConnectInfo->hLDAPConnection)
	{
		DEBUG_LOG(D_ERROR, "LDAP connection or pNamingContext invalid.\r\n");
      return FALSE;
	}

	// Annonym bind
	dwRes = ldap_bind_s(pLDAPConnectInfo->hLDAPConnection, NULL, NULL, LDAP_AUTH_SIMPLE);
	if (dwRes != LDAP_SUCCESS)
	{
		DEBUG_LOG(D_ERROR, "Unable to bind anonymously to LDAP server in order to extract naming context (ErrorCode: 0x%0lX).\r\n", dwRes);
		if (pLDAPConnectInfo->hLDAPConnection)
			ldap_unbind(pLDAPConnectInfo->hLDAPConnection);
      return FALSE;
	}

	// Determine naming context from rootDSE
	dwRes = ldap_search_s(pLDAPConnectInfo->hLDAPConnection, NULL, LDAP_SCOPE_BASE, NULL, NULL, 0, &plmsgSearchResponse);
	if (dwRes != LDAP_SUCCESS)
	{
		DEBUG_LOG(D_ERROR, "Unable to search root dse (ErrorCode: 0x%0lX).\r\n", dwRes);
		ldap_unbind(pLDAPConnectInfo->hLDAPConnection);
		if(plmsgSearchResponse != NULL)
			ldap_msgfree(plmsgSearchResponse);
      return FALSE;
	}

	pFirstEntry = ldap_first_entry(pLDAPConnectInfo->hLDAPConnection, plmsgSearchResponse); 
	if (!pFirstEntry)
	{
		DEBUG_LOG(D_ERROR, "Unable to extract domain name from root dse.\r\n");
		ldap_unbind(pLDAPConnectInfo->hLDAPConnection);
		if(plmsgSearchResponse != NULL)
			ldap_msgfree(plmsgSearchResponse);
      return FALSE;
	}

	pNamingContextTmp = ldap_get_values(pLDAPConnectInfo->hLDAPConnection, pFirstEntry, TEXT("defaultNamingContext")); 
	if (!pNamingContextTmp)
	{
		DEBUG_LOG(D_ERROR, "Unable to extract domain name from root dse, pNamingContext invalid.\r\n");
		ldap_unbind(pLDAPConnectInfo->hLDAPConnection);
		if(plmsgSearchResponse != NULL)
			ldap_msgfree(plmsgSearchResponse);
      return FALSE;
	}

	dwStrSize = (DWORD)_tcslen(*pNamingContextTmp);
	pLDAPConnectInfo->ptLDAPDomainDN = (PTCHAR) HeapAlloc(hCrawlerHeap, HEAP_ZERO_MEMORY, (dwStrSize  + 1) * sizeof(TCHAR));
	if (!pLDAPConnectInfo->ptLDAPDomainDN)
	{
		DEBUG_LOG(D_WARNING, "Unable to allocate ptLDAPDomainDN structure.\r\n");
		return FALSE;
	}	
	
	if (_tcscpy_s(pLDAPConnectInfo->ptLDAPDomainDN, dwStrSize + 1, *pNamingContextTmp))
	{
		DEBUG_LOG(D_WARNING, "Unable to copy pNamingContext structure.\r\n");
		return FALSE;
	}

	// Determine domain DNS name from rootDSE
	pLdapServiceName = ldap_get_values(pLDAPConnectInfo->hLDAPConnection, pFirstEntry, TEXT("ldapServiceName")); 
	if (!pLdapServiceName)
	{
		// If the domain DNS name is missing (eg: when mounting ntds.dit export) , we can use -s option to manually set the dns name
		if (pSyscrwlrOptions->tDNSName)
		{
			PTCHAR pDomainDnsName = NULL;
			DWORD dwDnsDomainNameLen = (DWORD) _tcslen(pSyscrwlrOptions->tDNSName);

			bIsMissingDnsName = TRUE;
			pDomainDnsName = (PTCHAR)HeapAlloc(hCrawlerHeap, HEAP_ZERO_MEMORY, (dwDnsDomainNameLen + 1) *sizeof(TCHAR));
			if (!pDomainDnsName)
			{
				DEBUG_LOG(D_WARNING, "Unable to allocate pLdapServiceName structure.\r\n");
				return FALSE;
			}

			if (memcpy_s(pDomainDnsName, (dwDnsDomainNameLen + 1) * sizeof(TCHAR), pSyscrwlrOptions->tDNSName, dwDnsDomainNameLen * sizeof(TCHAR)))
			{
				DEBUG_LOG(D_WARNING, "Unable to copy ptLDAPDomainName structure.\r\n");
				return FALSE;
			}
			pDomainDnsName[dwDnsDomainNameLen] = '\0';
			pLdapServiceName = &pDomainDnsName;
		}
		else
		{
			DEBUG_LOG(D_ERROR, "Unable to extract dns domain name from root dse, pLdapServiceName invalid.\r\n");
			ldap_unbind(pLDAPConnectInfo->hLDAPConnection);
			if (plmsgSearchResponse != NULL)
				ldap_msgfree(plmsgSearchResponse);
			return FALSE;
		}
	}

	if (bIsMissingDnsName == FALSE)
	{
		dnsDomainLen = ((DWORD)(_tcschr(*pLdapServiceName, TEXT(':'))) - (DWORD)(*pLdapServiceName)) / (DWORD) sizeof(TCHAR);
		if (dnsDomainLen == 0)
		{
			DEBUG_LOG(D_ERROR, "Unable to extract dns domain name from root dse, dnsDomainLen invalid.\r\n");
			ldap_unbind(pLDAPConnectInfo->hLDAPConnection);
			if (plmsgSearchResponse != NULL)
				ldap_msgfree(plmsgSearchResponse);
			return FALSE;
		}
	}
	else
	{
		dnsDomainLen = (DWORD)_tcslen(*pLdapServiceName);
	}

	pLDAPConnectInfo->ptLDAPDomainName = (PTCHAR) HeapAlloc(hCrawlerHeap, HEAP_ZERO_MEMORY, (dnsDomainLen  + 1) * sizeof(TCHAR));
	if (!pLDAPConnectInfo->ptLDAPDomainName)
	{
		DEBUG_LOG(D_WARNING, "Unable to allocate ptLDAPDomainName structure.\r\n");
		return FALSE;
	}	

	if (memcpy_s(pLDAPConnectInfo->ptLDAPDomainName, (dnsDomainLen  + 1) * sizeof(TCHAR), *pLdapServiceName, dnsDomainLen * sizeof(TCHAR)))
	{
		DEBUG_LOG(D_WARNING, "Unable to copy ptLDAPDomainName structure.\r\n");
		return FALSE;
	}
	pLDAPConnectInfo->ptLDAPDomainName[dnsDomainLen] = '\0';

	if (bIsMissingDnsName)
		HeapFree(hCrawlerHeap, NULL, *pLdapServiceName);

	return TRUE;
}

BOOL DisconnectFromLDAP()
{
	if (!pLDAPConnectInfo)
	{
		DEBUG_LOG(D_ERROR, "LDAP_CONNECT_INFO pointer invalid.\r\nExiting now...");
		DoExit(D_ERROR);
	}

	if (pLDAPConnectInfo->hLDAPConnection)
		ldap_unbind_s(pLDAPConnectInfo->hLDAPConnection);
	pLDAPConnectInfo->hLDAPConnection = NULL;
	
	if (pLDAPConnectInfo->ptLDAPDomainDN)
		HeapFree(hCrawlerHeap, NULL, pLDAPConnectInfo->ptLDAPDomainDN);
	if (pLDAPConnectInfo->ptLDAPDomainName)
		HeapFree(hCrawlerHeap, NULL, pLDAPConnectInfo->ptLDAPDomainName);
	
	HeapFree(hCrawlerHeap, NULL, pLDAPConnectInfo);
	return TRUE;
}

BOOL FreeLDAPInfo(_Inout_ PLDAP_AD_INFOS pLdapADInfos)
{
	if (pLdapADInfos == NULL)
	{
		DEBUG_LOG(D_ERROR, "LDAP_AD_INFOS pointer invalid.\r\nExiting now...");
		DoExit(D_ERROR);
	}

	if (FreeLDAPUsersInfo(pLdapADInfos) == FALSE
		|| FreeLDAPGroupsInfo(pLdapADInfos) == FALSE
		|| FreeLDAPOUsInfo(pLdapADInfos) == FALSE
		|| FreeLDAPGPOsInfo(pLdapADInfos) == FALSE)
	{
		DEBUG_LOG(D_ERROR, "Unable to free LDAP object correctly.\r\nExiting now...");
		DoExit(D_ERROR);
	}

	HeapFree(hCrawlerHeap, NULL, pLdapADInfos);
	return TRUE;
}

BOOL FreeLDAPUsersInfo(_Inout_ PLDAP_AD_INFOS pLdapADInfos)
{
	PLDAP_AD_USER pUsers = NULL;

	if (pLdapADInfos == NULL)
	{
		DEBUG_LOG(D_ERROR, "LDAP_AD_INFOS pointer invalid.\r\nExiting now...");
		DoExit(D_ERROR);
	}

	for (DWORD i = 0; i < pLdapADInfos->dwNumberOfUser; ++i)
	{
		pUsers = pLdapADInfos->pUsers[i];
		if (!pUsers)
			continue;

		if (pUsers->tLastLogon)
			HeapFree(hCrawlerHeap, NULL, pUsers->tLastLogon);
		if (pUsers->tCN)
			HeapFree(hCrawlerHeap, NULL, pUsers->tCN);
		if (pUsers->tName)
			HeapFree(hCrawlerHeap, NULL, pUsers->tName);
		if (pUsers->tDistinguishedName)
			ldap_memfree(pUsers->tDistinguishedName);
		if (pUsers->tMemberOf)
			HeapFree(hCrawlerHeap, NULL, pUsers->tMemberOf);
		if (pUsers->tPwdLastSet)
			HeapFree(hCrawlerHeap, NULL, pUsers->tPwdLastSet);
		if (pUsers->tUserPrincipalName)
			HeapFree(hCrawlerHeap, NULL, pUsers->tUserPrincipalName);
		if (pUsers->tSecurityDescriptor)
			LocalFree(pUsers->tSecurityDescriptor);
		if (pUsers->tSid)
			LocalFree(pUsers->tSid);
		HeapFree(hCrawlerHeap, NULL, pUsers);
		pLdapADInfos->pUsers[i] = NULL;
	}
	pLdapADInfos->dwNumberOfUser = 0;
	if (pLdapADInfos->pUsers)
		HeapFree(hCrawlerHeap, NULL, pLdapADInfos->pUsers);
	return TRUE;
}

BOOL FreeLDAPGroupsInfo(_Inout_ PLDAP_AD_INFOS pLdapADInfos)
{
	PLDAP_AD_GROUP pGroups = NULL;

	if (pLdapADInfos == NULL)
	{
		DEBUG_LOG(D_ERROR, "LDAP_AD_INFOS pointer invalid.\r\nExiting now...");
		DoExit(D_ERROR);
	}

	for (DWORD i = 0; i < pLdapADInfos->dwNumberOfGroup; ++i)
	{
		pGroups = pLdapADInfos->pGroups[i];
		if (!pGroups)
			continue;

		if (pGroups->tCN)
			HeapFree(hCrawlerHeap, NULL, pGroups->tCN);
		if (pGroups->tName)
			HeapFree(hCrawlerHeap, NULL, pGroups->tName);
		if (pGroups->tDescription)
			HeapFree(hCrawlerHeap, NULL, pGroups->tDescription);
		if (pGroups->tDistinguishedName)
			ldap_memfree(pGroups->tDistinguishedName);
		if (pGroups->tGroupType)
			HeapFree(hCrawlerHeap, NULL, pGroups->tGroupType);
		if (pGroups->tMember)
			HeapFree(hCrawlerHeap, NULL, pGroups->tMember);
		if (pGroups->tSecurityDescriptor)
			LocalFree(pGroups->tSecurityDescriptor);
		if (pGroups->tSid)
			LocalFree(pGroups->tSid);
		HeapFree(hCrawlerHeap, NULL, pGroups);
		pLdapADInfos->pGroups[i] = NULL;
	}
	pLdapADInfos->dwNumberOfGroup = 0;
	if (pLdapADInfos->pGroups)
		HeapFree(hCrawlerHeap, NULL, pLdapADInfos->pGroups);
	return TRUE;
}

BOOL FreeLDAPOUsInfo(_Inout_ PLDAP_AD_INFOS pLdapADInfos)
{
	PLDAP_AD_OU pOUs = NULL;

	if (pLdapADInfos == NULL)
	{
		DEBUG_LOG(D_ERROR, "LDAP_AD_INFOS pointer invalid.\r\nExiting now...");
		DoExit(D_ERROR);
	}

	for (DWORD i = 0; i < pLdapADInfos->dwNumberOfOU; ++i)
	{
		pOUs = pLdapADInfos->pOUs[i];
		if (!pOUs)
			continue;

		if (pOUs->tDescription)
			HeapFree(hCrawlerHeap, NULL, pOUs->tDescription);
		if (pOUs->tDistinguishedName)
			ldap_memfree(pOUs->tDistinguishedName);
		if (pOUs->tGpLink)
			HeapFree(hCrawlerHeap, NULL, pOUs->tGpLink);
		if (pOUs->tGpOptions)
			HeapFree(hCrawlerHeap, NULL, pOUs->tGpOptions);
		if (pOUs->tName)
			HeapFree(hCrawlerHeap, NULL, pOUs->tName);
		if (pOUs->tOu)
			HeapFree(hCrawlerHeap, NULL, pOUs->tOu);
		if (pOUs->tSecurityDescriptor)
			LocalFree(pOUs->tSecurityDescriptor);
		HeapFree(hCrawlerHeap, NULL, pOUs);
		pLdapADInfos->pOUs[i] = NULL;
	}
	pLdapADInfos->dwNumberOfOU = 0;
	if (pLdapADInfos->pOUs)
		HeapFree(hCrawlerHeap, NULL, pLdapADInfos->pOUs);
	return TRUE;
}

BOOL FreeLDAPGPOsInfo(_Inout_ PLDAP_AD_INFOS pLdapADInfos)
{
	PLDAP_AD_GPO pGPOs = NULL;

	if (pLdapADInfos == NULL)
	{
		DEBUG_LOG(D_ERROR, "LDAP_AD_INFOS pointer invalid.\r\nExiting now...");
		DoExit(D_ERROR);
	}

	for (DWORD i = 0; i < pLdapADInfos->dwNumberOfGPO; ++i)
	{
		pGPOs = pLdapADInfos->pGPOs[i];
		if (!pGPOs)
			continue;

		if (pGPOs->tCN)
			HeapFree(hCrawlerHeap, NULL, pGPOs->tCN);
		if (pGPOs->tCorePropagationData)
			HeapFree(hCrawlerHeap, NULL, pGPOs->tCorePropagationData);
		if (pGPOs->tDisplayName)
			HeapFree(hCrawlerHeap, NULL, pGPOs->tDisplayName);
		if (pGPOs->tDistinguishedName)
			ldap_memfree(pGPOs->tDistinguishedName);
		if (pGPOs->tFileSysPath)
			HeapFree(hCrawlerHeap, NULL, pGPOs->tFileSysPath);
		if (pGPOs->tFlags)
			HeapFree(hCrawlerHeap, NULL, pGPOs->tFlags);
		if (pGPOs->tFunctionalityVersion)
			HeapFree(hCrawlerHeap, NULL, pGPOs->tFunctionalityVersion);
		if (pGPOs->tMachineExtensionsNames)
			HeapFree(hCrawlerHeap, NULL, pGPOs->tMachineExtensionsNames);
		if (pGPOs->tUserExtensionsNames)
			HeapFree(hCrawlerHeap, NULL, pGPOs->tUserExtensionsNames);
		if (pGPOs->tVersionNumber)
			HeapFree(hCrawlerHeap, NULL, pGPOs->tVersionNumber);
		if (pGPOs->tWhenChanged)
			HeapFree(hCrawlerHeap, NULL, pGPOs->tWhenChanged);
		if (pGPOs->tWhenCreated)
			HeapFree(hCrawlerHeap, NULL, pGPOs->tWhenCreated);
		if (pGPOs->tWQLFilter)
			HeapFree(hCrawlerHeap, NULL, pGPOs->tWQLFilter);
		if (pGPOs->tSecurityDescriptor)
			LocalFree(pGPOs->tSecurityDescriptor);
		HeapFree(hCrawlerHeap, NULL, pGPOs);
		pLdapADInfos->pGPOs[i] = NULL;
	}
	pLdapADInfos->dwNumberOfGPO = 0;
	if (pLdapADInfos->pGPOs)
		HeapFree(hCrawlerHeap, NULL, pLdapADInfos->pGPOs);
	return TRUE;
}

BOOL LDAPExtractDomainUsers(_Inout_ PLDAP_AD_INFOS pLdapADInfos)
{
	PTCHAR pAttributes[7] = { LDAP_USER_TARGETED_INFO_CN, LDAP_USER_TARGETED_INFO_UPN, LDAP_USER_TARGETED_INFO_LLOGON, LDAP_USER_TARGETED_INFO_PWD_LS, LDAP_USER_TARGETED_INFO_MOF, LDAP_USER_TARGETED_INFO_SD, LDAP_USER_TARGETED_INFO_SID };
	PLDAP_AD_USER *pAllADUsers = NULL;
	DWORD dwADUsersCount = 0;

	if (!pLDAPConnectInfo->hLDAPConnection || !pLDAPConnectInfo->ptLDAPDomainDN)
	{
		DEBUG_LOG(D_ERROR, "LDAP connection invalid.\r\n");
		return FALSE;
	}

	if (!pLdapADInfos)
	{
		DEBUG_LOG(D_ERROR, "PLDAP_AD_INFOS invalid.\r\n");
		return FALSE;
	}

	for (DWORD i = 0; i < 7; ++i)
	{
		PLDAP_RETRIEVED_DATA *pRetrievedResults = NULL;
		DWORD dwResultsCount = 0;

		DEBUG_LOG(D_INFO, "Currently processing %ws attribute from user objects\r\n", pAttributes[i]);

		if (LDAPDoPageSearch(LDAP_SEARCH_USERS_FILTER, pAttributes[i], &pRetrievedResults, &dwResultsCount) != TRUE)
		{
			DEBUG_LOG(D_ERROR, "Unable to request LDAP server with parameters: %ws, %ws.\r\n", LDAP_SEARCH_USERS_FILTER, LDAP_USER_TARGETED_INFO_CN);
			return FALSE;
		}

		for (DWORD j = 0; j < dwResultsCount; ++j)
		{
			PLDAP_AD_USER pCurrentUser = GetLDAPADUser(pAllADUsers, dwADUsersCount, pRetrievedResults[j]->tDN);

			if (!pCurrentUser)
			{
				pCurrentUser = (PLDAP_AD_USER)HeapAlloc(hCrawlerHeap, HEAP_ZERO_MEMORY, sizeof(LDAP_AD_USER));
				if (!pCurrentUser)
				{
					DEBUG_LOG(D_ERROR, "Unable to allocate PLDAP_AD_USER.\r\n");
					return FALSE;
				}
				ZeroMemory(pCurrentUser, sizeof(LDAP_AD_USER));

				pAllADUsers = pAllADUsers ? (PLDAP_AD_USER *)HeapReAlloc(hCrawlerHeap, HEAP_ZERO_MEMORY, pAllADUsers, sizeof(PLDAP_AD_USER) * (dwADUsersCount + 1))
					: (PLDAP_AD_USER *)HeapAlloc(hCrawlerHeap, HEAP_ZERO_MEMORY, sizeof(PLDAP_AD_USER));
				if (!pAllADUsers)
				{
					DEBUG_LOG(D_ERROR, "Unable to allocate PLDAP_AD_USER *.\r\n");
					return FALSE;
				}
				pAllADUsers[dwADUsersCount] = pCurrentUser;
				dwADUsersCount++;
			}

			pCurrentUser->tDistinguishedName = pRetrievedResults[j]->tDN;
			if (i == 0)
				pCurrentUser->tCN = CStrToPtchar(pRetrievedResults[j]->ppbData[0], (DWORD) strlen((PCHAR)pRetrievedResults[j]->ppbData[0]));
			else if (i == 1)
				pCurrentUser->tUserPrincipalName = CStrToPtchar(pRetrievedResults[j]->ppbData[0], (DWORD) strlen((PCHAR)pRetrievedResults[j]->ppbData[0]));
			else if (i == 2)
				pCurrentUser->tLastLogon = CStrToPtchar(pRetrievedResults[j]->ppbData[0], (DWORD) strlen((PCHAR)pRetrievedResults[j]->ppbData[0]));
			else if (i == 3)
				pCurrentUser->tPwdLastSet = CStrToPtchar(pRetrievedResults[j]->ppbData[0], (DWORD) strlen((PCHAR)pRetrievedResults[j]->ppbData[0]));
			else if (i == 4)
			{
				PTCHAR tOutputValues = NULL;
				DWORD dwOutputValuesLen = 0;

				for (DWORD k = 0; k < pRetrievedResults[j]->dwElementCount; ++k)
				{
					PTCHAR tOutputValuesTmp = NULL;

					dwOutputValuesLen += pRetrievedResults[j]->pdwDataSize[k] + 2 * sizeof(TCHAR); //+1 for coma + whitespace
					tOutputValuesTmp = (PTCHAR)HeapAlloc(hCrawlerHeap, NULL, sizeof(TCHAR)* (dwOutputValuesLen + 1));
					if (!tOutputValuesTmp)
					{
						DEBUG_LOG(D_ERROR, "Unable to allocate buffer.\r\n");
						return FALSE;
					}
					ZeroMemory(tOutputValuesTmp, sizeof(TCHAR)* (dwOutputValuesLen + 1));

					if (tOutputValues)
					{
						if (!_stprintf_s(tOutputValuesTmp, (dwOutputValuesLen + 1), TEXT("%ws, %S"), tOutputValues, (PCHAR)pRetrievedResults[j]->ppbData[k]))
						{
							DEBUG_LOG(D_ERROR, "Unable to copy string buffer.\r\n");
							return FALSE;
						}
					}
					else
					{
						if (!_stprintf_s(tOutputValuesTmp, (dwOutputValuesLen + 1), TEXT("%S  "), (PCHAR)pRetrievedResults[j]->ppbData[k]))
						{
							DEBUG_LOG(D_ERROR, "Unable to copy string buffer.\r\n");
							return FALSE;
						}
					}
					if (pRetrievedResults[j]->ppbData[k])
					{
						HeapFree(hCrawlerHeap, NULL, pRetrievedResults[j]->ppbData[k]);
						pRetrievedResults[j]->ppbData[k] = NULL;
					}
					if (tOutputValues)
						HeapFree(hCrawlerHeap, NULL, tOutputValues);
					tOutputValues = tOutputValuesTmp;
				}
				pCurrentUser->tMemberOf = tOutputValues;
			}				
			else if (i == 5)
			{
				PTCHAR tLocalSd = NULL;
				if (!ConvertSecurityDescriptorToStringSecurityDescriptor((PSECURITY_DESCRIPTOR)pRetrievedResults[j]->ppbData[0], SDDL_REVISION_1, DACL_SECURITY_INFORMATION | OWNER_SECURITY_INFORMATION | SACL_SECURITY_INFORMATION, &tLocalSd, NULL))
				{
					DEBUG_LOG(D_ERROR, "Unable to convert sid to string sid.\r\n");
					return FALSE;
				}
				pCurrentUser->tSecurityDescriptor = tLocalSd;
			}
			else if (i == 6)
			{
				PTCHAR tLocalSid = NULL;
				if (!ConvertSidToStringSid((PSID)pRetrievedResults[j]->ppbData[0], &tLocalSid))
				{
					DEBUG_LOG(D_ERROR, "Unable to convert sid to string sid.\r\n");
					return FALSE;
				}
				pCurrentUser->tSid = tLocalSid;
			}
			else
				DEBUG_LOG(D_ERROR, "Unable to match USER attribute: %S.\r\n", (DWORD) (PCHAR)pRetrievedResults[j]->ppbData[0]);

			if (pRetrievedResults[j]->ppbData[0])
				HeapFree(hCrawlerHeap, NULL, pRetrievedResults[j]->ppbData[0]);
			if (pRetrievedResults[j]->pdwDataSize)
				HeapFree(hCrawlerHeap, NULL, pRetrievedResults[j]->pdwDataSize);
			if (pRetrievedResults[j]->ppbData)
				HeapFree(hCrawlerHeap, NULL, pRetrievedResults[j]->ppbData);
			if (pRetrievedResults[j])
				HeapFree(hCrawlerHeap, NULL, pRetrievedResults[j]);
		}
		if (pRetrievedResults)
		{
			HeapFree(hCrawlerHeap, NULL, pRetrievedResults);
			pRetrievedResults = NULL;
		}
	}
	
	pLdapADInfos->dwNumberOfUser = dwADUsersCount;
	pLdapADInfos->pUsers = pAllADUsers;
	return TRUE;
}

BOOL LDAPExtractDomainGroups(_Inout_ PLDAP_AD_INFOS pLdapADInfos)
{
	PTCHAR pAttributes[7] = { LDAP_GROUP_TARGETED_INFO_CN, LDAP_GROUP_TARGETED_INFO_DESC, LDAP_GROUP_TARGETED_INFO_GTYPE, LDAP_GROUP_TARGETED_INFO_MEMBER, LDAP_GROUP_TARGETED_INFO_NAME, LDAP_GROUP_TARGETED_INFO_SD, LDAP_GROUP_TARGETED_INFO_SID };
	PLDAP_AD_GROUP *pAllADGroups = NULL;
	DWORD dwADGroupsCount = 0;

	if (!pLDAPConnectInfo->hLDAPConnection || !pLDAPConnectInfo->ptLDAPDomainDN)
	{
		DEBUG_LOG(D_ERROR, "LDAP connection invalid.\r\n");
		return FALSE;
	}

	if (!pLdapADInfos)
	{
		DEBUG_LOG(D_ERROR, "PLDAP_AD_INFOS invalid.\r\n");
		return FALSE;
	}

	for (DWORD i = 0; i < 7; ++i)
	{
		PLDAP_RETRIEVED_DATA *pRetrievedResults = NULL;
		DWORD dwResultsCount = 0;

		DEBUG_LOG(D_INFO, "Currently processing %ws attribute from domain group objects\r\n", pAttributes[i]);

		if (LDAPDoPageSearch(LDAP_SEARCH_GROUPS_FILTER, pAttributes[i], &pRetrievedResults, &dwResultsCount) != TRUE)
		{
			DEBUG_LOG(D_ERROR, "Unable to request LDAP server with parameters: %ws, %ws.\r\n", LDAP_SEARCH_GROUPS_FILTER, LDAP_GROUP_TARGETED_INFO_CN);
			return FALSE;
		}

		for (DWORD j = 0; j < dwResultsCount; ++j)
		{
			PLDAP_AD_GROUP pCurrentGroup = GetLDAPADGroup(pAllADGroups, dwADGroupsCount, pRetrievedResults[j]->tDN);
			
			if (!pCurrentGroup)
			{
				pCurrentGroup = (PLDAP_AD_GROUP)HeapAlloc(hCrawlerHeap, HEAP_ZERO_MEMORY, sizeof(LDAP_AD_GROUP));
				if (!pCurrentGroup)
				{
					DEBUG_LOG(D_ERROR, "Unable to allocate PLDAP_AD_GROUP.\r\n");
					return FALSE;
				}
				ZeroMemory(pCurrentGroup, sizeof(LDAP_AD_GROUP));

				pAllADGroups = pAllADGroups ? (PLDAP_AD_GROUP *)HeapReAlloc(hCrawlerHeap, HEAP_ZERO_MEMORY, pAllADGroups, sizeof(PLDAP_AD_GROUP) * (dwADGroupsCount + 1))
					: (PLDAP_AD_GROUP *)HeapAlloc(hCrawlerHeap, HEAP_ZERO_MEMORY, sizeof(PLDAP_AD_GROUP));
				if (!pAllADGroups)
				{
					DEBUG_LOG(D_ERROR, "Unable to allocate PLDAP_AD_GROUP *.\r\n");
					return FALSE;
				}
				pAllADGroups[dwADGroupsCount] = pCurrentGroup;
				dwADGroupsCount++;
			}

			pCurrentGroup->tDistinguishedName = pRetrievedResults[j]->tDN;
			if (i == 0)
				pCurrentGroup->tCN = CStrToPtchar(pRetrievedResults[j]->ppbData[0], (DWORD) strlen((PCHAR)pRetrievedResults[j]->ppbData[0]));
			else if (i == 1)
				pCurrentGroup->tDescription = CStrToPtchar(pRetrievedResults[j]->ppbData[0], (DWORD) strlen((PCHAR)pRetrievedResults[j]->ppbData[0]));
			else if (i == 2)
				pCurrentGroup->tGroupType = CStrToPtchar(pRetrievedResults[j]->ppbData[0], (DWORD) strlen((PCHAR)pRetrievedResults[j]->ppbData[0]));
			else if (i == 3)
			{
				PTCHAR tOutputValues = NULL;
				DWORD dwOutputValuesLen = 0;

				for (DWORD k = 0; k < pRetrievedResults[j]->dwElementCount; ++k)
				{
					PTCHAR tOutputValuesTmp = NULL;

					dwOutputValuesLen += pRetrievedResults[j]->pdwDataSize[k] + 2 * sizeof(TCHAR); //+1 for coma + whitespace
					tOutputValuesTmp = (PTCHAR)HeapAlloc(hCrawlerHeap, NULL, sizeof(TCHAR)* (dwOutputValuesLen + 1));
					if (!tOutputValuesTmp)
					{
						DEBUG_LOG(D_ERROR, "Unable to allocate buffer.\r\n");
						return FALSE;
					}
					ZeroMemory(tOutputValuesTmp, sizeof(TCHAR)* (dwOutputValuesLen + 1));

					if (tOutputValues)
					{
						if (!_stprintf_s(tOutputValuesTmp, (dwOutputValuesLen + 1), TEXT("%ws, %S"), tOutputValues, (PCHAR)pRetrievedResults[j]->ppbData[k]))
						{
							DEBUG_LOG(D_ERROR, "Unable to copy string buffer.\r\n");
							return FALSE;
						}
					}
					else
					{
						if (!_stprintf_s(tOutputValuesTmp, (dwOutputValuesLen + 1), TEXT("%S  "), (PCHAR)pRetrievedResults[j]->ppbData[k]))
						{
							DEBUG_LOG(D_ERROR, "Unable to copy string buffer.\r\n");
							return FALSE;
						}
					}
					if (pRetrievedResults[j]->ppbData[k])
					{
						HeapFree(hCrawlerHeap, NULL, pRetrievedResults[j]->ppbData[k]);
						pRetrievedResults[j]->ppbData[k] = NULL;
					}
					if (tOutputValues)
						HeapFree(hCrawlerHeap, NULL, tOutputValues);
					tOutputValues = tOutputValuesTmp;
				}
				pCurrentGroup->tMember = tOutputValues;
			}
			else if (i == 4)
				pCurrentGroup->tName = CStrToPtchar(pRetrievedResults[j]->ppbData[0], (DWORD) strlen((PCHAR)pRetrievedResults[j]->ppbData[0]));
			else if (i == 5)
			{
				PTCHAR tLocalSd = NULL;
				if (!ConvertSecurityDescriptorToStringSecurityDescriptor((PSECURITY_DESCRIPTOR)pRetrievedResults[j]->ppbData[0], SDDL_REVISION_1, DACL_SECURITY_INFORMATION | OWNER_SECURITY_INFORMATION | SACL_SECURITY_INFORMATION, &tLocalSd, NULL))
				{
					DEBUG_LOG(D_ERROR, "Unable to convert sid to string sid.\r\n");
					return FALSE;
				}
				pCurrentGroup->tSecurityDescriptor = tLocalSd;
			}
			else if (i == 6)
			{
				PTCHAR tLocalSid = NULL;
				if (!ConvertSidToStringSid((PSID)pRetrievedResults[j]->ppbData[0], &tLocalSid))
				{
					DEBUG_LOG(D_ERROR, "Unable to convert sid to string sid.\r\n");
					return FALSE;
				}
				pCurrentGroup->tSid = tLocalSid;
			}
			else
				DEBUG_LOG(D_ERROR, "Unable to match GROUP attribute: %S.\r\n", (PCHAR) pRetrievedResults[j]->ppbData[0]);

			if (pRetrievedResults[j]->ppbData[0])
				HeapFree(hCrawlerHeap, NULL, pRetrievedResults[j]->ppbData[0]);
			if (pRetrievedResults[j]->pdwDataSize)
				HeapFree(hCrawlerHeap, NULL, pRetrievedResults[j]->pdwDataSize);
			if (pRetrievedResults[j]->ppbData)
				HeapFree(hCrawlerHeap, NULL, pRetrievedResults[j]->ppbData);
			if (pRetrievedResults[j])
				HeapFree(hCrawlerHeap, NULL, pRetrievedResults[j]);
		}
		if (pRetrievedResults)
		{
			HeapFree(hCrawlerHeap, NULL, pRetrievedResults);
			pRetrievedResults = NULL;
		}
	}

	pLdapADInfos->dwNumberOfGroup = dwADGroupsCount;
	pLdapADInfos->pGroups = pAllADGroups;
	return TRUE;
}

BOOL LDAPExtractOrganizationalUnits(_Inout_ PLDAP_AD_INFOS pLdapADInfos)
{
	PTCHAR pAttributes[6] = {LDAP_OU_TARGETED_INFO_OU, LDAP_OU_TARGETED_INFO_DESC, LDAP_OU_TARGETED_INFO_NAME, LDAP_OU_TARGETED_INFO_GPLINK, LDAP_OU_TARGETED_INFO_GPOPTIONS, LDAP_OU_TARGETED_INFO_SD };
	PLDAP_AD_OU *pAllADOus = NULL;
	DWORD dwADOusCount = 0;

	if (!pLDAPConnectInfo->hLDAPConnection || !pLDAPConnectInfo->ptLDAPDomainDN)
	{
		DEBUG_LOG(D_ERROR, "LDAP connection invalid.\r\n");
		return FALSE;
	}

	if (!pLdapADInfos)
	{
		DEBUG_LOG(D_ERROR, "PLDAP_AD_INFOS invalid.\r\n");
		return FALSE;
	}

	for (DWORD i = 0; i < 6; ++i)
	{
		PLDAP_RETRIEVED_DATA *pRetrievedResults = NULL;
		DWORD dwResultsCount = 0;

		DEBUG_LOG(D_INFO, "Currently processing %ws attribute from domain organizational units objects\r\n", pAttributes[i]);

		if (LDAPDoPageSearch(LDAP_SEARCH_OUS_FILTER, pAttributes[i], &pRetrievedResults, &dwResultsCount) != TRUE)
		{
			DEBUG_LOG(D_ERROR, "Unable to request LDAP server with parameters: %ws, %ws.\r\n", LDAP_SEARCH_OUS_FILTER, LDAP_OU_TARGETED_INFO_OU);
			return FALSE;
		}

		for (DWORD j = 0; j < dwResultsCount; ++j)
		{
			PLDAP_AD_OU pCurrentOu = GetLDAPADOu(pAllADOus, dwADOusCount, pRetrievedResults[j]->tDN);

			if (!pCurrentOu)
			{
				pCurrentOu = (PLDAP_AD_OU)HeapAlloc(hCrawlerHeap, HEAP_ZERO_MEMORY, sizeof(LDAP_AD_OU));
				if (!pCurrentOu)
				{
					DEBUG_LOG(D_ERROR, "Unable to allocate PLDAP_AD_OU.\r\n");
					return FALSE;
				}
				ZeroMemory(pCurrentOu, sizeof(LDAP_AD_OU));

				pAllADOus = pAllADOus ? (PLDAP_AD_OU *)HeapReAlloc(hCrawlerHeap, HEAP_ZERO_MEMORY, pAllADOus, sizeof(PLDAP_AD_OU)* (dwADOusCount + 1))
					: (PLDAP_AD_OU *)HeapAlloc(hCrawlerHeap, HEAP_ZERO_MEMORY, sizeof(PLDAP_AD_OU));
				if (!pAllADOus)
				{
					DEBUG_LOG(D_ERROR, "Unable to allocate PLDAP_AD_OU *.\r\n");
					return FALSE;
				}
				pAllADOus[dwADOusCount] = pCurrentOu;
				dwADOusCount++;
			}

			pCurrentOu->tDistinguishedName = pRetrievedResults[j]->tDN;
			if (i == 0)
				pCurrentOu->tOu = CStrToPtchar(pRetrievedResults[j]->ppbData[0], (DWORD) strlen((PCHAR)pRetrievedResults[j]->ppbData[0]));
			else if (i == 1)
				pCurrentOu->tDescription = CStrToPtchar(pRetrievedResults[j]->ppbData[0], (DWORD) strlen((PCHAR)pRetrievedResults[j]->ppbData[0]));
			else if (i == 2)
				pCurrentOu->tName = CStrToPtchar(pRetrievedResults[j]->ppbData[0], (DWORD) strlen((PCHAR)pRetrievedResults[j]->ppbData[0]));
			else if (i == 3)
				pCurrentOu->tGpLink = CStrToPtchar(pRetrievedResults[j]->ppbData[0], (DWORD) strlen((PCHAR)pRetrievedResults[j]->ppbData[0]));
			else if (i == 4)
				pCurrentOu->tGpOptions = CStrToPtchar(pRetrievedResults[j]->ppbData[0], (DWORD) strlen((PCHAR)pRetrievedResults[j]->ppbData[0]));
			else if (i == 5)
			{
				PTCHAR tLocalSd = NULL;
				if (!ConvertSecurityDescriptorToStringSecurityDescriptor((PSECURITY_DESCRIPTOR)pRetrievedResults[j]->ppbData[0], SDDL_REVISION_1, DACL_SECURITY_INFORMATION | OWNER_SECURITY_INFORMATION | SACL_SECURITY_INFORMATION, &tLocalSd, NULL))
				{
					DEBUG_LOG(D_ERROR, "Unable to convert sid to string sid.\r\n");
					return FALSE;
				}
				pCurrentOu->tSecurityDescriptor = tLocalSd;
			}
			else
				DEBUG_LOG(D_ERROR, "Unable to match OU attribute: %S.\r\n", (PCHAR) pRetrievedResults[j]->ppbData[0]);

			if (pRetrievedResults[j]->ppbData[0])
				HeapFree(hCrawlerHeap, NULL, pRetrievedResults[j]->ppbData[0]);
			if (pRetrievedResults[j]->pdwDataSize)
				HeapFree(hCrawlerHeap, NULL, pRetrievedResults[j]->pdwDataSize);
			if (pRetrievedResults[j]->ppbData)
				HeapFree(hCrawlerHeap, NULL, pRetrievedResults[j]->ppbData);
			if (pRetrievedResults[j])
				HeapFree(hCrawlerHeap, NULL, pRetrievedResults[j]);
		}
		if (pRetrievedResults)
		{
			HeapFree(hCrawlerHeap, NULL, pRetrievedResults);
			pRetrievedResults = NULL;
		}
	}

	pLdapADInfos->dwNumberOfOU = dwADOusCount;
	pLdapADInfos->pOUs = pAllADOus;
	return TRUE;
}

BOOL LDAPExtractGPOs(_Inout_ PLDAP_AD_INFOS pLdapADInfos)
{
	PTCHAR pAttributes[13] = {LDAP_GPO_TARGETED_INFO_CN, LDAP_GPO_TARGETED_INFO_CREATED, LDAP_GPO_TARGETED_INFO_MODIFIED, LDAP_GPO_TARGETED_INFO_DISPLAY, LDAP_GPO_TARGETED_INFO_FLAGS, LDAP_GPO_TARGETED_INFO_VERSION, LDAP_GPO_TARGETED_INFO_FUNCVER, LDAP_GPO_TARGETED_INFO_FILEPATH, LDAP_GPO_TARGETED_INFO_EXT_COMP, LDAP_GPO_TARGETED_INFO_EXT_USR, LDAP_GPO_TARGETED_INFO_PROPAG, LDAP_GPO_TARGETED_INFO_WQLFILTER, LDAP_GPO_TARGETED_INFO_SD};
	PLDAP_AD_GPO *pAllADGpos = NULL;
	DWORD dwADGposCount = 0;

	if (!pLDAPConnectInfo->hLDAPConnection || !pLDAPConnectInfo->ptLDAPDomainDN)
	{
		DEBUG_LOG(D_ERROR, "LDAP connection invalid.\r\n");
		return FALSE;
	}

	if (!pLdapADInfos)
	{
		DEBUG_LOG(D_ERROR, "PLDAP_AD_INFOS invalid.\r\n");
		return FALSE;
	}

	for (DWORD i = 0; i < 13; ++i)
	{
		PLDAP_RETRIEVED_DATA *pRetrievedResults = NULL;
		DWORD dwResultsCount = 0;

		DEBUG_LOG(D_INFO, "Currently processing %ws attribute from domain gpo objects\r\n", pAttributes[i]);

		if (LDAPDoPageSearch(LDAP_SEARCH_GPO_FILTER, pAttributes[i], &pRetrievedResults, &dwResultsCount) != TRUE)
		{
			DEBUG_LOG(D_ERROR, "Unable to request LDAP server with parameters: %ws, %ws.\r\n", LDAP_SEARCH_GPO_FILTER, LDAP_GPO_TARGETED_INFO_CN);
			return FALSE;
		}

		for (DWORD j = 0; j < dwResultsCount; ++j)
		{
			PLDAP_AD_GPO pCurrentGpo = GetLDAPADGpo(pAllADGpos, dwADGposCount, pRetrievedResults[j]->tDN);

			if (!pCurrentGpo)
			{
				pCurrentGpo = (PLDAP_AD_GPO) HeapAlloc(hCrawlerHeap, HEAP_ZERO_MEMORY, sizeof(LDAP_AD_GPO));
				if (!pCurrentGpo)
				{
					DEBUG_LOG(D_ERROR, "Unable to allocate PLDAP_AD_GPO.\r\n");
					return FALSE;
				}
				ZeroMemory(pCurrentGpo, sizeof(LDAP_AD_GPO));

				pAllADGpos = pAllADGpos ? (PLDAP_AD_GPO *)HeapReAlloc(hCrawlerHeap, HEAP_ZERO_MEMORY, pAllADGpos, sizeof(PLDAP_AD_GPO)* (dwADGposCount + 1))
					: (PLDAP_AD_GPO *)HeapAlloc(hCrawlerHeap, HEAP_ZERO_MEMORY, sizeof(PLDAP_AD_GPO));
				if (!pAllADGpos)
				{
					DEBUG_LOG(D_ERROR, "Unable to allocate PLDAP_AD_GPO *.\r\n");
					return FALSE;
				}
				pAllADGpos[dwADGposCount] = pCurrentGpo;
				dwADGposCount++;
			}

			pCurrentGpo->tDistinguishedName = pRetrievedResults[j]->tDN;
			if (i == 0)
				pCurrentGpo->tCN = CStrToPtchar(pRetrievedResults[j]->ppbData[0], (DWORD) strlen((PCHAR)pRetrievedResults[j]->ppbData[0]));
			else if (i == 1)
				pCurrentGpo->tWhenCreated = CStrToPtchar(pRetrievedResults[j]->ppbData[0], (DWORD) strlen((PCHAR)pRetrievedResults[j]->ppbData[0]));
			else if (i == 2)
				pCurrentGpo->tWhenChanged = CStrToPtchar(pRetrievedResults[j]->ppbData[0], (DWORD) strlen((PCHAR)pRetrievedResults[j]->ppbData[0]));
			else if (i == 3)
				pCurrentGpo->tDisplayName = CStrToPtchar(pRetrievedResults[j]->ppbData[0], (DWORD) strlen((PCHAR)pRetrievedResults[j]->ppbData[0]));
			else if (i == 4)
				pCurrentGpo->tFlags = CStrToPtchar(pRetrievedResults[j]->ppbData[0], (DWORD) strlen((PCHAR)pRetrievedResults[j]->ppbData[0]));
			else if (i == 5)
				pCurrentGpo->tVersionNumber = CStrToPtchar(pRetrievedResults[j]->ppbData[0], (DWORD) strlen((PCHAR)pRetrievedResults[j]->ppbData[0]));
			else if (i == 6)
				pCurrentGpo->tFunctionalityVersion = CStrToPtchar(pRetrievedResults[j]->ppbData[0], (DWORD) strlen((PCHAR)pRetrievedResults[j]->ppbData[0]));
			else if (i == 7)
				pCurrentGpo->tFileSysPath = CStrToPtchar(pRetrievedResults[j]->ppbData[0], (DWORD) strlen((PCHAR)pRetrievedResults[j]->ppbData[0]));
			else if (i == 8)
				pCurrentGpo->tMachineExtensionsNames = CStrToPtchar(pRetrievedResults[j]->ppbData[0], (DWORD) strlen((PCHAR)pRetrievedResults[j]->ppbData[0]));
			else if (i == 9)
				pCurrentGpo->tUserExtensionsNames = CStrToPtchar(pRetrievedResults[j]->ppbData[0], (DWORD) strlen((PCHAR)pRetrievedResults[j]->ppbData[0]));
			else if (i == 10)
			{
				PTCHAR tOutputValues = NULL;
				DWORD dwOutputValuesLen = 0;

				for (DWORD k = 0; k < pRetrievedResults[j]->dwElementCount; ++k)
				{
					PTCHAR tOutputValuesTmp = NULL;

					dwOutputValuesLen += pRetrievedResults[j]->pdwDataSize[k] + 2 * sizeof(TCHAR); //+1 for coma + whitespace
					tOutputValuesTmp = (PTCHAR)HeapAlloc(hCrawlerHeap, NULL, sizeof(TCHAR)* (dwOutputValuesLen + 1));
					if (!tOutputValuesTmp)
					{
						DEBUG_LOG(D_ERROR, "Unable to allocate buffer.\r\n");
						return FALSE;
					}
					ZeroMemory(tOutputValuesTmp, sizeof(TCHAR)* (dwOutputValuesLen + 1));

					if (tOutputValues)
					{
						if (!_stprintf_s(tOutputValuesTmp, (dwOutputValuesLen + 1), TEXT("%ws, %S"), tOutputValues, (PCHAR)pRetrievedResults[j]->ppbData[k]))
						{
							DEBUG_LOG(D_ERROR, "Unable to copy string buffer.\r\n");
							return FALSE;
						}
					}
					else
					{
						if (!_stprintf_s(tOutputValuesTmp, (dwOutputValuesLen + 1), TEXT("%S  "), (PCHAR)pRetrievedResults[j]->ppbData[k]))
						{
							DEBUG_LOG(D_ERROR, "Unable to copy string buffer.\r\n");
							return FALSE;
						}
					}
					if (pRetrievedResults[j]->ppbData[k])
					{
						HeapFree(hCrawlerHeap, NULL, pRetrievedResults[j]->ppbData[k]);
						pRetrievedResults[j]->ppbData[k] = NULL;
					}
					if (tOutputValues)
						HeapFree(hCrawlerHeap, NULL, tOutputValues);
					tOutputValues = tOutputValuesTmp;
				}
				pCurrentGpo->tCorePropagationData = tOutputValues;
			}
			else if (i == 11)
				pCurrentGpo->tWQLFilter = CStrToPtchar(pRetrievedResults[j]->ppbData[0], (DWORD) strlen((PCHAR)pRetrievedResults[j]->ppbData[0]));
			else if (i == 12)
			{
				PTCHAR tLocalSd = NULL;
				if (!ConvertSecurityDescriptorToStringSecurityDescriptor((PSECURITY_DESCRIPTOR)pRetrievedResults[j]->ppbData[0], SDDL_REVISION_1, DACL_SECURITY_INFORMATION | OWNER_SECURITY_INFORMATION | SACL_SECURITY_INFORMATION, &tLocalSd, NULL))
				{
					DEBUG_LOG(D_ERROR, "Unable to convert sid to string sid.\r\n");
					return FALSE;
				}
				pCurrentGpo->tSecurityDescriptor = tLocalSd;
			}
			else
				DEBUG_LOG(D_ERROR, "Unable to match GPO attribute: %S.\r\n", (PCHAR) pRetrievedResults[j]->ppbData[0]);

			if (pRetrievedResults[j]->ppbData[0])
				HeapFree(hCrawlerHeap, NULL, pRetrievedResults[j]->ppbData[0]);
			if (pRetrievedResults[j]->pdwDataSize)
				HeapFree(hCrawlerHeap, NULL, pRetrievedResults[j]->pdwDataSize);
			if (pRetrievedResults[j]->ppbData)
				HeapFree(hCrawlerHeap, NULL, pRetrievedResults[j]->ppbData);
			if (pRetrievedResults[j])
				HeapFree(hCrawlerHeap, NULL, pRetrievedResults[j]);
		}
		if (pRetrievedResults)
		{
			HeapFree(hCrawlerHeap, NULL, pRetrievedResults);
			pRetrievedResults = NULL;
		}
	}

	pLdapADInfos->dwNumberOfGPO = dwADGposCount;
	pLdapADInfos->pGPOs = pAllADGpos;
	return TRUE;
}

PLDAP_AD_USER GetLDAPADUser(PLDAP_AD_USER *pADUsers, DWORD dwADUsersCount, PTCHAR tDN)
{
	if (!pADUsers || !tDN)
		return NULL;

	for (DWORD i = 0; i < dwADUsersCount; ++i)
	{
		PLDAP_AD_USER pCurrUser = pADUsers[i];
		if (pCurrUser && pCurrUser->tDistinguishedName && !_tcsicmp(pCurrUser->tDistinguishedName, tDN))
			return pCurrUser;
	}
	return NULL;
}

PLDAP_AD_GROUP GetLDAPADGroup(PLDAP_AD_GROUP *pADGroups, DWORD dwADGroupsCount, PTCHAR tDN)
{
	if (!pADGroups || !tDN)
		return NULL;

	for (DWORD i = 0; i < dwADGroupsCount; ++i)
	{
		PLDAP_AD_GROUP pCurrGroups = pADGroups[i];
		if (pCurrGroups && pCurrGroups->tDistinguishedName && !_tcsicmp(pCurrGroups->tDistinguishedName, tDN))
			return pCurrGroups;
	}
	return NULL;
}

PLDAP_AD_OU GetLDAPADOu(PLDAP_AD_OU *pADOus, DWORD dwADOusCount, PTCHAR tDN)
{
	if (!pADOus || !tDN)
		return NULL;

	for (DWORD i = 0; i < dwADOusCount; ++i)
	{
		PLDAP_AD_OU pCurrOu = pADOus[i];
		if (pCurrOu && pCurrOu->tDistinguishedName && !_tcsicmp(pCurrOu->tDistinguishedName, tDN))
			return pCurrOu;
	}
	return NULL;
}

PLDAP_AD_GPO GetLDAPADGpo(PLDAP_AD_GPO *pADGpos, DWORD dwADGposCount, PTCHAR tDN)
{
	if (!pADGpos || !tDN)
		return NULL;

	for (DWORD i = 0; i < dwADGposCount; ++i)
	{
		PLDAP_AD_GPO pCurrGpo = pADGpos[i];
		if (pCurrGpo && pCurrGpo->tDistinguishedName && !_tcsicmp(pCurrGpo->tDistinguishedName, tDN))
			return pCurrGpo;
	}
	return NULL;
}

BOOL LDAPDoPageSearch(_In_ PTCHAR tLdapFilter, _In_ PTCHAR tOrigAttribute, _Inout_ PLDAP_RETRIEVED_DATA **ppRetrievedResults, _Inout_ PDWORD dwResultsCount)
{
	DWORD dwRes = LDAP_SUCCESS, dwCountEntries = 0, dwAllRowCpt = 0;
	ULONG iterateSearch = 0;
	PLDAPMessage pLDAPSearchResult = NULL;
	PLDAP_AD_GROUP *pLDAPGroups = NULL;
	PLDAP_AD_GROUP pLdapADGroup = NULL;
	PLDAPMessage pCurrentEntry = NULL;
	PTCHAR ptCurrAttribute = NULL;
	BerElement* pBerElt = NULL;
	PLDAPSearch pLDAPSearch = NULL;
	struct l_timeval timeout;
	PTCHAR *pptValues = NULL;
	DWORD dwAllValuesCount = 0;
	PTCHAR pAttributes[2] = { tOrigAttribute, NULL };
	PLDAP_RETRIEVED_DATA *pRetrievedAllData = NULL;

	if (!tLdapFilter || !tOrigAttribute)
	{
		DEBUG_LOG(D_ERROR, "Invalid LDAP filter or attribute.\r\n");
		return FALSE;
	}

	pLDAPSearch = ldap_search_init_page(pLDAPConnectInfo->hLDAPConnection, pLDAPConnectInfo->ptLDAPDomainDN,
		LDAP_SCOPE_SUBTREE, tLdapFilter, pAttributes, FALSE, NULL, NULL, LDAP_NO_LIMIT, LDAP_NO_LIMIT, NULL);
	if (pLDAPSearch == NULL)
	{
		DEBUG_LOG(D_ERROR, "Unable to create ldap search structure (ErrorCode: 0x%0lX).\r\n", LdapGetLastError());
		ldap_unbind(pLDAPConnectInfo->hLDAPConnection);
		return FALSE;
	}

	// We loop until all the LDAP page has been returned
	while (TRUE)
	{
		DWORD dwPageEntries = 0;
		PLDAP_RETRIEVED_DATA *pRetrievedPageEntriesData = NULL;
		BOOL bIsAttributeFound = FALSE;

		timeout.tv_sec = 100000;
		dwRes = ldap_get_next_page_s(pLDAPConnectInfo->hLDAPConnection, pLDAPSearch, &timeout, AD_LDAP_SEARCH_LIMIT, &dwCountEntries, &pLDAPSearchResult);
		if (dwRes != LDAP_SUCCESS)
		{
			if (dwRes == LDAP_NO_RESULTS_RETURNED)
				break;

			DEBUG_LOG(D_ERROR, "Unable to retrieve paged count (ErrorCode: 0x%0lX).\r\n", dwRes);
			ldap_unbind(pLDAPConnectInfo->hLDAPConnection);
			return FALSE;
		}

		dwPageEntries = ldap_count_entries(pLDAPConnectInfo->hLDAPConnection, pLDAPSearchResult);
		if (!dwPageEntries)
			continue;

		pRetrievedPageEntriesData = (PLDAP_RETRIEVED_DATA *)HeapAlloc(hCrawlerHeap, HEAP_ZERO_MEMORY, sizeof (PLDAP_RETRIEVED_DATA)* (dwPageEntries));
		if (!pRetrievedPageEntriesData)
		{
			DEBUG_LOG(D_ERROR, "Unable to allocate PLDAP_RETRIEVED_DATA* memory.\r\n");
			ldap_unbind(pLDAPConnectInfo->hLDAPConnection);
			return FALSE;
		}
		ZeroMemory(pRetrievedPageEntriesData, sizeof (PLDAP_RETRIEVED_DATA)* (dwPageEntries));

		pCurrentEntry = ldap_first_entry(pLDAPConnectInfo->hLDAPConnection, pLDAPSearchResult);
		if (!pCurrentEntry)
		{
			DEBUG_LOG(D_ERROR, "Unable to retrieve pCurrentEntry.\r\n");
			ldap_unbind(pLDAPConnectInfo->hLDAPConnection);
			return FALSE;
		}

		// We loop until all returned object has been process
		for (DWORD i = 0; i < dwPageEntries; ++i)
		{
			PLDAP_RETRIEVED_DATA pRetrievedData = NULL;

			ptCurrAttribute = ldap_first_attribute(pLDAPConnectInfo->hLDAPConnection, pCurrentEntry, &pBerElt);
			ldap_next_attribute(pLDAPConnectInfo->hLDAPConnection, pCurrentEntry, pBerElt);
			if (!ptCurrAttribute)
				continue;
			else
				bIsAttributeFound = TRUE;

			// Select the right method to retrieve attributes (directly or by range)
			if (!_tcsicmp(tOrigAttribute, ptCurrAttribute))
			{
				if (LDAPExtractAttributes(pCurrentEntry, ptCurrAttribute, &pRetrievedData) != TRUE)
				{
					DEBUG_LOG(D_ERROR, "Unable to extract LDAP attributes.\r\n");
					ldap_unbind(pLDAPConnectInfo->hLDAPConnection);
					return FALSE;
				}
			}
			else
			{
				if (LDAPExtractRangedAttributes(pCurrentEntry, tOrigAttribute, ptCurrAttribute, &pRetrievedData) != TRUE)
				{
					DEBUG_LOG(D_ERROR, "Unable to extract LDAP rangedattributes.\r\n");
					ldap_unbind(pLDAPConnectInfo->hLDAPConnection);
					return FALSE;
				}
			}

			if (ptCurrAttribute)
			{
				ldap_memfree(ptCurrAttribute);
				ptCurrAttribute = NULL;
			}
			pRetrievedPageEntriesData[i] = pRetrievedData;
		}

		if (bIsAttributeFound)
		{
			pRetrievedAllData = (dwAllValuesCount) ? (PLDAP_RETRIEVED_DATA *)HeapReAlloc(hCrawlerHeap, HEAP_ZERO_MEMORY, pRetrievedAllData, sizeof(PLDAP_RETRIEVED_DATA)* (dwAllValuesCount + dwPageEntries))
				: (PLDAP_RETRIEVED_DATA *)HeapAlloc(hCrawlerHeap, HEAP_ZERO_MEMORY, sizeof(PLDAP_RETRIEVED_DATA)* (dwPageEntries));

			for (DWORD k = 0; k < dwPageEntries; ++k)
			{
				pRetrievedAllData[dwAllValuesCount + k] = pRetrievedPageEntriesData[k];
			}
			dwAllValuesCount += dwPageEntries;
		}
		if (pRetrievedPageEntriesData)
			HeapFree(hCrawlerHeap, NULL, pRetrievedPageEntriesData);
	}

	*ppRetrievedResults = pRetrievedAllData;
	*dwResultsCount = dwAllValuesCount;
	return TRUE;
}

BOOL LDAPExtractAttributes(_In_ PLDAPMessage pCurrentEntry, _In_ PTCHAR tAttribute, _Inout_ PLDAP_RETRIEVED_DATA *ppRetrievedData)
{
	DWORD dwAttributeCount = 0;
	PTCHAR tCurrentDN = ldap_get_dn(pLDAPConnectInfo->hLDAPConnection, pCurrentEntry);
	PBYTE *ppbData = NULL;
	PDWORD pdwDataSize = NULL;
	struct berval **bvals = NULL;

	if (!pCurrentEntry || !tAttribute)
	{
		DEBUG_LOG(D_ERROR, "Unable to extract attributes.\r\n");
		return FALSE;
	}

	if (!tCurrentDN)
	{
		DEBUG_LOG(D_ERROR, "Unable to retrieve DN for current entry.\r\n");
		ldap_unbind(pLDAPConnectInfo->hLDAPConnection);
		return FALSE;
	}

	bvals = ldap_get_values_len(pLDAPConnectInfo->hLDAPConnection, pCurrentEntry, tAttribute);
	if (!bvals)
	{
		DEBUG_LOG(D_WARNING, "Unable to retrieve bvals attributes for DN: %ws.\r\n", tCurrentDN);
		goto FreeAndContinue;
	}

	for (DWORD j = 0; bvals[j] != NULL; ++j)
	{
		PBYTE pbTmpData = NULL;

		ppbData = ppbData ? (PBYTE *)HeapReAlloc(hCrawlerHeap, NULL, ppbData, sizeof(PBYTE) * (j + 1))
			: (PBYTE *)HeapAlloc(hCrawlerHeap, NULL, sizeof(PBYTE));
		pdwDataSize = pdwDataSize ? (PDWORD)HeapReAlloc(hCrawlerHeap, NULL, pdwDataSize, sizeof(DWORD)* (j + 1))
			: (PDWORD)HeapAlloc(hCrawlerHeap, NULL, sizeof(DWORD));
		pbTmpData = (PBYTE)HeapAlloc(hCrawlerHeap, NULL, sizeof(BYTE)* (bvals[j]->bv_len + 1));
		if (!ppbData || !pdwDataSize || !pbTmpData)
		{
			DEBUG_LOG(D_ERROR, "Unable to allocate memory for extracted attribute.\r\n");
			return FALSE;
		}
		ZeroMemory(pbTmpData, sizeof(BYTE)* (bvals[j]->bv_len + 1));

		if (memcpy_s(pbTmpData, sizeof(BYTE)* (bvals[j]->bv_len + 1), bvals[j]->bv_val, sizeof(BYTE)* (bvals[j]->bv_len)))
		{
			DEBUG_LOG(D_WARNING, "Unable to copy attribute data.\r\n");
			return FALSE;
		}

		ppbData[j] = pbTmpData;
		pdwDataSize[j] = bvals[j]->bv_len;
		dwAttributeCount++;
	}

FreeAndContinue:
	if (bvals != NULL)
	{
		ldap_value_free_len(bvals);
		bvals = NULL;
	}

	*ppRetrievedData = (PLDAP_RETRIEVED_DATA)HeapAlloc(hCrawlerHeap, NULL, sizeof(LDAP_RETRIEVED_DATA));
	if (!*ppRetrievedData)
	{
		DEBUG_LOG(D_ERROR, "Unable to allocate LDAP_RETRIEVED_DATA.\r\n");
		return FALSE;
	}
	ZeroMemory(*ppRetrievedData, sizeof(LDAP_RETRIEVED_DATA));
	(*ppRetrievedData)->dwElementCount = dwAttributeCount;
	(*ppRetrievedData)->ppbData = ppbData;
	(*ppRetrievedData)->pdwDataSize = pdwDataSize;
	(*ppRetrievedData)->tDN = tCurrentDN;

	return TRUE;
}

BOOL LDAPExtractRangedAttributes(_In_ PLDAPMessage pCurrentEntry, _In_ PTCHAR tOrigAttribute, _In_ PTCHAR tAttribute, _Inout_ PLDAP_RETRIEVED_DATA *ppRetrievedData)
{
	LDAPMessage *pLDAPSearchResult, *pLDAPEntry;
	PTCHAR pNewRangeAttributes[2] = { tAttribute, NULL };
	PTCHAR tTmpNewAttribute = NULL;
	DWORD dwStart = 0, dwEnd = 0, dwRangesize = 0;
	PTCHAR tCurrentDN = ldap_get_dn(pLDAPConnectInfo->hLDAPConnection, pCurrentEntry);
	DWORD dwAttributeCount = 0;
	PBYTE *ppbData = NULL;
	PDWORD pdwDataSize = NULL;

	if (!pCurrentEntry || !tAttribute || !tCurrentDN)
	{
		DEBUG_LOG(D_ERROR, "Invalid LDAP ranged attribute request.\r\n");
		return FALSE;
	}

	do
	{
		PTCHAR tExtractedAttrValues = NULL, tAllValuesTmp = NULL;
		DWORD dwCurrAttributeCount = 0;
		PBYTE *ppbCurrData = NULL;
		PDWORD pdwCurrDataSize = NULL;

		if (ldap_search_s(pLDAPConnectInfo->hLDAPConnection, tCurrentDN, LDAP_SCOPE_BASE, TEXT("(objectClass=*)"), pNewRangeAttributes, FALSE, &pLDAPSearchResult) != LDAP_SUCCESS)
		{
			DEBUG_LOG(D_ERROR, "Unable to search for range attribute (err=%d).\r\n", LdapGetLastError());
			DoExit(D_ERROR);
		}

		pLDAPEntry = ldap_first_entry(pLDAPConnectInfo->hLDAPConnection, pLDAPSearchResult);
		if (!pLDAPEntry)
		{
			DEBUG_LOG(D_ERROR, "Unable to retrieve entry for range attributes (err=%d).\r\n", LdapGetLastError());
			DoExit(D_ERROR);
		}

		if (GetRangeValues(pLDAPEntry, tAttribute, &dwCurrAttributeCount, &ppbCurrData, &pdwCurrDataSize) == FALSE)
		{
			DEBUG_LOG(D_ERROR, "Unable to extract range values for range attributes.\r\n");
			DoExit(D_ERROR);
		}

		ppbData = ppbData ? (PBYTE *)HeapReAlloc(hCrawlerHeap, NULL, ppbData, sizeof(PBYTE)* (dwAttributeCount + dwCurrAttributeCount))
			: (PBYTE *)HeapAlloc(hCrawlerHeap, NULL, sizeof(PBYTE) * dwCurrAttributeCount);
		pdwDataSize = pdwDataSize ? (PDWORD)HeapReAlloc(hCrawlerHeap, NULL, pdwDataSize, sizeof(DWORD)* (dwAttributeCount + dwCurrAttributeCount))
			: (PDWORD)HeapAlloc(hCrawlerHeap, NULL, sizeof(DWORD) * dwCurrAttributeCount);

		for (DWORD j = 0; j < dwCurrAttributeCount; ++j)
		{
			ppbData[dwAttributeCount + j] = ppbCurrData[j];
			pdwDataSize[dwAttributeCount + j] = pdwCurrDataSize[j];
		}
		dwAttributeCount += dwCurrAttributeCount;

		if (ppbCurrData)
			HeapFree(hCrawlerHeap, NULL, ppbCurrData);
		if (pdwCurrDataSize)
			HeapFree(hCrawlerHeap, NULL, pdwCurrDataSize);

		if (ParseRange(tOrigAttribute, pNewRangeAttributes[0], &dwStart, &dwEnd) == FALSE)
		{
			DEBUG_LOG(D_ERROR, "Unable to parse current range.\r\n");
			DoExit(D_ERROR);
		}

		if (tTmpNewAttribute)
			HeapFree(hCrawlerHeap, NULL, tTmpNewAttribute);
		if (ConstructRangeAtt(tOrigAttribute, dwEnd + 1, -1, &tTmpNewAttribute) == FALSE)
		{
			DEBUG_LOG(D_ERROR, "Unable to construct new range.\r\n");
			DoExit(D_ERROR);
		}
		pNewRangeAttributes[0] = tTmpNewAttribute;

	} while (dwEnd != -1);
	if (tTmpNewAttribute)
		HeapFree(hCrawlerHeap, NULL, tTmpNewAttribute);

	*ppRetrievedData = (PLDAP_RETRIEVED_DATA)HeapAlloc(hCrawlerHeap, NULL, sizeof(LDAP_RETRIEVED_DATA));
	if (!*ppRetrievedData)
	{
		DEBUG_LOG(D_ERROR, "Unable to allocate LDAP_RETRIEVED_DATA.\r\n");
		return FALSE;
	}
	ZeroMemory(*ppRetrievedData, sizeof(LDAP_RETRIEVED_DATA));
	(*ppRetrievedData)->dwElementCount = dwAttributeCount;
	(*ppRetrievedData)->ppbData = ppbData;
	(*ppRetrievedData)->pdwDataSize = pdwDataSize;
	(*ppRetrievedData)->tDN = tCurrentDN;
	return TRUE;
}

BOOL GetRangeValues(_Inout_ PLDAPMessage pEntry, _In_ PTCHAR tOriginalAttribute, _Inout_ PDWORD pdwAttributeCount, _Inout_ PBYTE **pppbData, _Inout_ PDWORD *ppdwDataSize)
{
	PTCHAR ptAttribute = NULL;
	BerElement *pBerElt = NULL;
	DWORD dwAttributeCount = 0;
	PBYTE *ppbData = NULL;
	PDWORD pdwDataSize = NULL;
	struct berval **bvals = NULL;

	ptAttribute = ldap_first_attribute(pLDAPConnectInfo->hLDAPConnection, pEntry, &pBerElt);
	if (!ptAttribute)
	{
		DEBUG_LOG(D_ERROR, "Unable to extract ranged attribute (err=%d).\r\n", LdapGetLastError());
		DoExit(D_ERROR);
	}

	bvals = ldap_get_values_len(pLDAPConnectInfo->hLDAPConnection, pEntry, ptAttribute);
	if (!bvals)
	{
		DEBUG_LOG(D_WARNING, "Unable to retrieve bvals attributes.\r\n");
		goto FreeAndContinue;
	}

	for (DWORD j = 0; bvals[j] != NULL; ++j)
	{
		PBYTE pbTmpData = NULL;

		ppbData = ppbData ? (PBYTE *)HeapReAlloc(hCrawlerHeap, NULL, ppbData, sizeof(PBYTE)* (j + 1))
			: (PBYTE *)HeapAlloc(hCrawlerHeap, NULL, sizeof(PBYTE));
		pdwDataSize = pdwDataSize ? (PDWORD)HeapReAlloc(hCrawlerHeap, NULL, pdwDataSize, sizeof(DWORD)* (j + 1))
			: (PDWORD)HeapAlloc(hCrawlerHeap, NULL, sizeof(DWORD));
		pbTmpData = (PBYTE)HeapAlloc(hCrawlerHeap, NULL, sizeof(BYTE)* (bvals[j]->bv_len + 1));
		if (!ppbData || !pdwDataSize || !pbTmpData)
		{
			DEBUG_LOG(D_ERROR, "Unable to allocate memory for extracted attribute.\r\n");
			return FALSE;
		}
		ZeroMemory(pbTmpData, sizeof(BYTE)* (bvals[j]->bv_len + 1));

		if (memcpy_s(pbTmpData, sizeof(BYTE)* (bvals[j]->bv_len + 1), bvals[j]->bv_val, sizeof(BYTE)* (bvals[j]->bv_len)))
		{
			DEBUG_LOG(D_WARNING, "Unable to copy attribute data.\r\n");
			return FALSE;
		}

		ppbData[j] = pbTmpData;
		pdwDataSize[j] = bvals[j]->bv_len;
		dwAttributeCount++;
	}

FreeAndContinue:
	if (bvals != NULL)
	{
		ldap_value_free_len(bvals);
		bvals = NULL;
	}
	if (ptAttribute)
	{
		ldap_memfree(ptAttribute);
		ptAttribute = NULL;
	}
	if (pBerElt)
		ber_free(pBerElt, NULL);

	*pdwAttributeCount = dwAttributeCount;
	*pppbData = ppbData;
	*ppdwDataSize = pdwDataSize;
	return TRUE;
}

BOOL ParseRange(_In_ PTCHAR tAtttype, _In_ PTCHAR tAttdescr, _Inout_ PDWORD pdwStart, _Inout_ PDWORD pdwEnd)
{
	PTCHAR tRangeStr = TEXT("range=");
	PTCHAR tStartstring = NULL, tEndstring = NULL, tOptionstart = NULL;
	INT iAtttypeLen = 0, iAttdescrLen = 0;
	DWORD dwRangestringlen = 0, dwCpt = 0;
	BOOL bRangefound = FALSE;

	dwRangestringlen = (DWORD)_tcslen(tRangeStr);

	if (!_tcsicmp(tAtttype, tAttdescr))
	{
		// The attribute was returned without options.
		*pdwStart = 0;
		*pdwEnd = -1;
		return TRUE;
	}

	iAtttypeLen = (INT) _tcslen(tAtttype);
	iAttdescrLen = (INT) _tcslen(tAttdescr);

	if ((iAtttypeLen > iAttdescrLen) || (';' != tAttdescr[iAtttypeLen]) || (_tcsnicmp(tAtttype, tAttdescr, iAtttypeLen)))
		return FALSE;

	// It is the correct attribute type. Verify that if there is a range option.
	*pdwStart = 0;
	*pdwEnd = -1;
	tOptionstart = tAttdescr + iAtttypeLen + 1;
	do
	{
		if ((iAttdescrLen - (tOptionstart - tAttdescr)) < (INT) dwRangestringlen)
		{
			// No space left in the string for range option.
			tOptionstart = tAttdescr + iAttdescrLen;
		}
		else if (!_tcsncicmp(tOptionstart, tRangeStr, dwRangestringlen))
		{
			// Found a range string. Ensure that it looks like what is expected and then parse it.
			tStartstring = tOptionstart + dwRangestringlen;
			for (dwCpt = 0; isdigit(tStartstring[dwCpt]); dwCpt++);

			if ((0 != dwCpt) && ('-' == tStartstring[dwCpt]) && ((tStartstring[dwCpt + 1] == '*') || isdigit(tStartstring[dwCpt + 1])))
			{
				// Acceptable. Finish parsing.
				tEndstring = &tStartstring[dwCpt + 1];
				*pdwStart = _tcstol(tStartstring, NULL, 10);
				if (tEndstring[0] == '*')
					*pdwEnd = -1;
				else
					*pdwEnd = _tcstol(tEndstring, NULL, 10);
				bRangefound = TRUE;
			}
		}

		// If necessary, advance to the next option.
		if (!bRangefound)
		{
			while ((*tOptionstart != '\0') && (*tOptionstart != ';'))
				tOptionstart++;

			// Skip the semicolon.
			tOptionstart++;
		}
	} while (!bRangefound && (iAttdescrLen > (tOptionstart - tAttdescr)));
	return TRUE;
}

BOOL ConstructRangeAtt(_In_ PTCHAR tAtttype, _In_ DWORD dwStart, _In_ INT iEnd, _Inout_ PTCHAR* tOutputRangeAttr)
{
	TCHAR startstring[11], endstring[11];
	DWORD requiredlen = 0;
	PTCHAR tOutbuff = NULL;

	startstring[10] = TEXT('\0');
	endstring[10] = TEXT('\0');

	// Calculate buffer space required.
	_sntprintf_s(startstring, 10, TEXT("%u"), dwStart);
	if (iEnd == -1)
		_tcscpy_s(endstring, TEXT("*"));
	else
		_sntprintf_s(endstring, 10, TEXT("%u"), iEnd);

	// Add in space for ';range=' and '-' and the terminating null.
	requiredlen = (DWORD) (_tcslen(tAtttype) + _tcslen(startstring) + _tcslen(endstring));
	requiredlen += 9;

	// Verify that enough space was passed in.
	tOutbuff = (PTCHAR)HeapAlloc(hCrawlerHeap, HEAP_ZERO_MEMORY, (requiredlen + 1) * sizeof(TCHAR));
	if (!tOutbuff)
	{
		DEBUG_LOG(D_ERROR, "Unable to allocate raneg attribute.\r\n");
		DoExit(D_ERROR);
	}
	_sntprintf_s(tOutbuff, (requiredlen + 1), requiredlen, TEXT("%ws;range=%ws-%ws"), tAtttype, startstring, endstring);
	tOutbuff[requiredlen] = '\0';

	*tOutputRangeAttr = tOutbuff;
	return TRUE;
}
