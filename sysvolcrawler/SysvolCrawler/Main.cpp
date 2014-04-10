/**************************************************
* SysvolCrawler - main.c
* AUTHOR: Luc Delsalle
*
* Projet entry point
*
* ANSSI/COSSI/DTO/BAI - 2014
***************************************************/
#include "main.h"

/* Application heap */
HANDLE hCrawlerHeap = NULL;

/* Launch option */
PSYSCRWLR_OPTIONS pSyscrwlrOptions = NULL;

INT main(_In_ INT argc, _In_ PCHAR argv[])
{
	DWORD dwRes = 0;
	printf("SysvolCrawler v%ws - L. Delsalle - ANSSI/COSSI/DTO/Bureau Audits et Inspections\r\n\r\n", CRAWLER_VERSION);

	// Heap creation
	hCrawlerHeap = HeapCreate(HEAP_NO_SERIALIZE, 0, 0);
	if (!hCrawlerHeap)
	{
		DEBUG_LOG(D_ERROR, "Error during heap allocation\r\n");
		DoExit(D_ERROR);
	}

	pSyscrwlrOptions = (PSYSCRWLR_OPTIONS) HeapAlloc(hCrawlerHeap, HEAP_ZERO_MEMORY, sizeof(SYSCRWLR_OPTIONS));
	if (!pSyscrwlrOptions)
	{
		DEBUG_LOG(D_WARNING, "Unable to allocate pSyscrwlrOptions structure.\r\n");
		return FALSE;
	}	
	ZeroMemory(pSyscrwlrOptions, sizeof(SYSCRWLR_OPTIONS));
	pSyscrwlrOptions->bShouldDumpLDAP = FALSE;
	pSyscrwlrOptions->bShouldDumpSYSVOL = TRUE;
	pSyscrwlrOptions->bShouldPrintXML = TRUE;
	pSyscrwlrOptions->bShouldPrintCSV = TRUE;
	pSyscrwlrOptions->bShouldPrintSTDOUT = FALSE;
	pSyscrwlrOptions->dwDebugLevel = DEFAULT_DEBUG_LEVEL;
	pSyscrwlrOptions->tLogFilePath = DEFAULT_LOGFILE;
	pSyscrwlrOptions->dwLDAPPort = DEFAULT_LDAP_PORT;

	// Parse command line options
	ParseCmdLineOption(argc, argv);

	if (pSyscrwlrOptions->bShouldDumpSYSVOL)
	{
		PCHAR pszSysvolFolderPath = PtcharToCStr(pSyscrwlrOptions->tSysvolFolderPath);
		if (LaunchSysvolCrawling(pszSysvolFolderPath) != TRUE)
			dwRes += 1;
		if (pszSysvolFolderPath)
			HeapFree(hCrawlerHeap, NULL, pszSysvolFolderPath);
	}

	if (pSyscrwlrOptions->bShouldDumpLDAP)
	{
		if (LaunchLDAPCrawling() != TRUE)
			dwRes += 1;
	}

	if (pSyscrwlrOptions->tOutputFolderPath)
		HeapFree(hCrawlerHeap, NULL, pSyscrwlrOptions->tOutputFolderPath);
	if (pSyscrwlrOptions->tSysvolFolderPath)
		HeapFree(hCrawlerHeap, NULL, pSyscrwlrOptions->tSysvolFolderPath);
	if (pSyscrwlrOptions->tLDAPServer)
		HeapFree(hCrawlerHeap, NULL, pSyscrwlrOptions->tLDAPServer);
	if (pSyscrwlrOptions->tADLogin)
		HeapFree(hCrawlerHeap, NULL, pSyscrwlrOptions->tADLogin);
	if (pSyscrwlrOptions->tADPassword)
		HeapFree(hCrawlerHeap, NULL, pSyscrwlrOptions->tADPassword);
	if (pSyscrwlrOptions->tDNSName)
		HeapFree(hCrawlerHeap, NULL, pSyscrwlrOptions->tDNSName);
	if (pSyscrwlrOptions)
		HeapFree(hCrawlerHeap, NULL, pSyscrwlrOptions);
	HeapDestroy(hCrawlerHeap);
	return dwRes;
}

BOOL LaunchSysvolCrawling(_In_ PCHAR pSysvolPath)
{
	PTCHAR tSysvolPath = NULL;
	DWORD dwBuffLen = 0;

	dwBuffLen = (DWORD) strlen(pSysvolPath) + 1;

	tSysvolPath = (PTCHAR) HeapAlloc(hCrawlerHeap, HEAP_ZERO_MEMORY, sizeof (TCHAR) * MAX_PATH);	
	if (!tSysvolPath)
	{
		DEBUG_LOG(D_ERROR, "Error during string allocation\r\n");
		DoExit(D_ERROR);
	}

	if (!MultiByteToWideChar(CP_ACP, MB_PRECOMPOSED, pSysvolPath, dwBuffLen, tSysvolPath, dwBuffLen))
	{
		DEBUG_LOG(D_ERROR, "Error during string conversion\r\n");
		DoExit(D_ERROR);
	}

	// Launch folder crawling
	if ((InitDispatcher() == FALSE)
		|| (BrowseAndDispatch(tSysvolPath, 0) == FALSE)
		|| (FreeDispatcher() == FALSE))
	{
		DEBUG_LOG(D_ERROR, "Error sysvol crawling.\r\n Exiting Now...");
		DoExit(D_ERROR);
	}

	// Ugly hack to close XML root element
	if (pSyscrwlrOptions->bShouldPrintXML)
	{
		TCHAR tPath[MAX_PATH];

		if ((StringCchCopy(tPath, MAX_PATH, pSyscrwlrOptions->tOutputFolderPath) != S_OK)
			|| (StringCchCat(tPath, MAX_PATH, TEXT("\\")) != S_OK)
			|| (StringCchCat(tPath, MAX_PATH, OUTPUT_FOLDER_NAME) != S_OK))
		{
			DEBUG_LOG(D_WARNING, "Unable to compute output path folder.\r\nExiting now...");
			DoExit(D_ERROR);
		}
		CloseXMLRootElement(tPath);
	}
		

	if (tSysvolPath)
		HeapFree(hCrawlerHeap, NULL, tSysvolPath);
	return TRUE;
}

BOOL LaunchLDAPCrawling()
{
	PLDAP_AD_INFOS pLdapAdInfos = NULL;

	if ((InitToLDAP(pSyscrwlrOptions->tLDAPServer, pSyscrwlrOptions->dwLDAPPort) != TRUE)
		|| (ExtractDomainNamingContext() != TRUE)
		|| (BindToLDAP(pSyscrwlrOptions->tADLogin, pSyscrwlrOptions->tADPassword) != TRUE))
	{
		DEBUG_LOG(D_ERROR, "Unable to init connexion to domain controler LDAP.\r\n Exiting Now...\r\n");
		DoExit(D_ERROR);
	}

	pLdapAdInfos = (PLDAP_AD_INFOS) HeapAlloc(hCrawlerHeap, HEAP_ZERO_MEMORY, sizeof(LDAP_AD_INFOS));
	if (!pLdapAdInfos)
	{
		DEBUG_LOG(D_WARNING, "Unable to allocate LDAP_AD_INFOS structure.\r\n");
		return FALSE;
	}	
	ZeroMemory(pLdapAdInfos, sizeof(LDAP_AD_INFOS));

	if (PrintLdapDataHeader() != TRUE)
	{
		DEBUG_LOG(D_ERROR, "Unable to extract print LDAP header.\r\n Exiting Now...\r\n");
		DoExit(D_ERROR);
	}

	if (LDAPExtractDomainUsers(pLdapAdInfos) != TRUE)
	{
		DEBUG_LOG(D_ERROR, "Unable to extract users from domain controller.\r\n Exiting Now...\r\n");
		DoExit(D_ERROR);
	}

	if (PrintSpecifiedData(pLdapAdInfos, LDAP_USER_TARGETED) != TRUE)
	{
		DEBUG_LOG(D_ERROR, "Unable to print users from domain controller.\r\n Exiting Now...\r\n");
		DoExit(D_ERROR);
	}

	if (FreeLDAPUsersInfo(pLdapAdInfos) != TRUE)
	{
		DEBUG_LOG(D_ERROR, "Unable to free LDAP users correctly.\r\nExiting now...");
		DoExit(D_ERROR);
	}

	if (LDAPExtractDomainGroups(pLdapAdInfos) != TRUE)
	{
		DEBUG_LOG(D_ERROR, "Unable to extract groups from domain controller.\r\n Exiting Now...\r\n");
		DoExit(D_ERROR);
	}

	if (PrintSpecifiedData(pLdapAdInfos, LDAP_GROUP_TARGETED) != TRUE)
	{
		DEBUG_LOG(D_ERROR, "Unable to print groups from domain controller.\r\n Exiting Now...\r\n");
		DoExit(D_ERROR);
	}

	if (FreeLDAPGroupsInfo(pLdapAdInfos) != TRUE)
	{
		DEBUG_LOG(D_ERROR, "Unable to free LDAP groups correctly.\r\nExiting now...");
		DoExit(D_ERROR);
	}

	if (LDAPExtractOrganizationalUnits(pLdapAdInfos) != TRUE)
	{
		DEBUG_LOG(D_ERROR, "Unable to extract OUs from domain controller.\r\n Exiting Now...\r\n");
		DoExit(D_ERROR);
	}

	if (PrintSpecifiedData(pLdapAdInfos, LDAP_OU_TARGETED) != TRUE)
	{
		DEBUG_LOG(D_ERROR, "Unable to print OUs from domain controller.\r\n Exiting Now...\r\n");
		DoExit(D_ERROR);
	}

	if (FreeLDAPOUsInfo(pLdapAdInfos) != TRUE)
	{
		DEBUG_LOG(D_ERROR, "Unable to free LDAP OUs correctly.\r\nExiting now...");
		DoExit(D_ERROR);
	}

	if (LDAPExtractGPOs(pLdapAdInfos) != TRUE)
	{
		DEBUG_LOG(D_ERROR, "Unable to extract GPOs from domain controller.\r\n Exiting Now...\r\n");
		DoExit(D_ERROR);
	}

	if (PrintSpecifiedData(pLdapAdInfos, LDAP_GPO_TARGETED) != TRUE)
	{
		DEBUG_LOG(D_ERROR, "Unable to print GPOs from domain controller.\r\n Exiting Now...\r\n");
		DoExit(D_ERROR);
	}

	if (FreeLDAPGPOsInfo(pLdapAdInfos) != TRUE)
	{
		DEBUG_LOG(D_ERROR, "Unable to free LDAP GPOs correctly.\r\nExiting now...");
		DoExit(D_ERROR);
	}

	if (DisconnectFromLDAP() != TRUE)
		DEBUG_LOG(D_WARNING, "Unable to disconnect LDAP directory.");

	if (PrintLdapDataFooter() != TRUE)
	{
		DEBUG_LOG(D_ERROR, "Unable to extract print LDAP footer.\r\n Exiting Now...\r\n");
		DoExit(D_ERROR);
	}

	if (pLdapAdInfos)
		HeapFree(hCrawlerHeap, NULL, pLdapAdInfos);

	return TRUE;
}

VOID ParseCmdLineOption(_In_ INT argc, _In_ PCHAR *argv)
{
	PCHAR outOptArg = NULL;
	INT opt = 0, cpt = 0;

	if (!pSyscrwlrOptions)
	{
		printf("internal error.\r\n");
		DoExit(D_ERROR);
	}

	if (argc == 1)
	{
		SysCrwlrUsage(argv[0], TRUE);
		DoExit(D_WARNING);
	}
	else if (argc <= 2)
	{
		printf("error: not enough parameters.\r\n\r\n", opt);
		SysCrwlrUsage(argv[0], FALSE);
		DoExit(D_WARNING);
	}

	while ((opt = GetOpt(argc, argv, "d:l:p:n:o:e:r:s:", &outOptArg, &cpt)) != EOF)
	{
		if ((opt == 'd') && outOptArg)
		{
			pSyscrwlrOptions->bShouldDumpLDAP = TRUE;
			pSyscrwlrOptions->tLDAPServer = CStrToPtchar((PBYTE)outOptArg, (DWORD) strlen(outOptArg));
		}
		else if ((opt == 'l') && outOptArg)
			pSyscrwlrOptions->tADLogin = CStrToPtchar((PBYTE)outOptArg, (DWORD) strlen(outOptArg));
		else if ((opt == 'p') && outOptArg)
			pSyscrwlrOptions->tADPassword = CStrToPtchar((PBYTE)outOptArg, (DWORD) strlen(outOptArg));
		else if ((opt == 'e') && outOptArg)
			pSyscrwlrOptions->dwDebugLevel = atoi(outOptArg);
		else if ((opt == 'r') && outOptArg)
			pSyscrwlrOptions->dwLDAPPort = atoi(outOptArg);
		else if ((opt == 'o') && outOptArg)
			DefineOutputFormat(outOptArg);
		else if (opt == 'n')
			pSyscrwlrOptions->bShouldDumpSYSVOL = FALSE;
		else if ((opt == 's') && outOptArg)
			pSyscrwlrOptions->tDNSName = CStrToPtchar((PBYTE)outOptArg, (DWORD)strlen(outOptArg));
		else if (outOptArg && (outOptArg[0] != '-') && (!pSyscrwlrOptions->tOutputFolderPath))
			pSyscrwlrOptions->tOutputFolderPath = CStrToPtchar((PBYTE)outOptArg, (DWORD)strlen(outOptArg));
		else if (outOptArg && (outOptArg[0] != '-') && (pSyscrwlrOptions->tOutputFolderPath))
			pSyscrwlrOptions->tSysvolFolderPath = CStrToPtchar((PBYTE)outOptArg, (DWORD)strlen(outOptArg));
		else if (((opt == 'd') || (opt == 'l') || (opt == 'p') || (opt == 'o') || (opt == 'r') || (opt == 'c')) && (!outOptArg))
		{
			printf("invalid argument for option: %c\r\n\r\n", opt);
			SysCrwlrUsage(argv[0], FALSE);
			DoExit(D_WARNING);
		}
		else
		{
			printf("unknown option found.\r\n\r\n", opt);
			SysCrwlrUsage(argv[0], FALSE);
			DoExit(D_WARNING);
		}
	}

	if ((pSyscrwlrOptions->bShouldDumpSYSVOL || pSyscrwlrOptions->bShouldDumpLDAP) && (!pSyscrwlrOptions->tOutputFolderPath))
	{
			printf("invalid output folder path.\r\n\r\n", opt);
			SysCrwlrUsage(argv[0], FALSE);
			DoExit(D_WARNING);
	}

	if ((pSyscrwlrOptions->bShouldDumpSYSVOL) && (!pSyscrwlrOptions->tSysvolFolderPath))
	{
			printf("invalid sysvol folder path.\r\n\r\n", opt);
			SysCrwlrUsage(argv[0], FALSE);
			DoExit(D_WARNING);
	}
}

INT GetOpt(_In_ INT argc, _In_ PCHAR *argv, _In_ PCHAR optstring, _Out_ PCHAR *outOptArg, _Out_ PINT pOptInd)
{
	TCHAR c;
	PCHAR cp;
	static PCHAR next = NULL;

	if (*pOptInd == 0)
		next = NULL;

	*outOptArg = NULL;

	if (*pOptInd >= argc)
		return EOF;

	if (next == NULL || *next == '\0')
	{
		if (*pOptInd == 0)
			(*pOptInd)++;

		if (*pOptInd >= argc || argv[*pOptInd][0] != '-' || argv[*pOptInd][1] == '\0')
		{
			*outOptArg = NULL;
			if (*pOptInd < argc)
			{
				*outOptArg = argv[*pOptInd];
				(*pOptInd)++;
			}
			return '?';
		}

		if (strcmp(argv[*pOptInd], "--") == 0)
		{
			(*pOptInd)++;
			*outOptArg = NULL;
			if (*pOptInd < argc)
				*outOptArg = argv[*pOptInd];
			return EOF;
		}

		next = argv[*pOptInd];
		next++;
		(*pOptInd)++;
	}

	c = *next++;
	cp = strchr(optstring, c);

	if (cp == NULL || c == ':')
		return '?';

	cp++;
	if (*cp == ':')
	{
		if (*next != '\0')
		{
			*outOptArg = next;
			next = NULL;
		}
		else if ((*pOptInd) < argc)
		{
			*outOptArg = argv[(*pOptInd)];

			if ((*outOptArg)[0] == '-')
				(*outOptArg) = NULL;
			else
				(*pOptInd)++;
		}
		else
		{
			(*outOptArg) = NULL;
		}
	}

	return c;
}

VOID SysCrwlrUsage(_In_ PCHAR pSyscrwlrName, _In_ BOOL bSouldPrintInfo)
{
	printf("usage: %s [-d DC_IP] [-l AD_PRIV_LOGIN -p AD_PRIV_PWD] [-o OUTPUT_FORMAT] [-e DEBUG_LEVEL] [-n] [-r LDAP_PORT] OUTPUT_PATH SYSVOL_FOLDER_PATH\r\n\r\n", pSyscrwlrName);
	if (bSouldPrintInfo == TRUE)
	printf("A fast, lightweight and almost complete Sysvol files parser.\r\n\r\n");
	printf("required arguments:\r\n");
	printf("OUTPUT_PATH\t\twrite output results there (ex: C:\\GpoAssessment\\)\r\n");
	printf("SYSVOL_FOLDER_PATH\tpath to sysvol repository (ex: \\\\dc1\\sysvol\\domain\\policies)\r\n");

	printf("\r\n");
	printf("optional arguments:\r\n");
	printf("-d AD_LDAP_DIRECTORY\tdump gpo infos from LDAP server\r\n");
	printf("-n\t\t\tdisable sysvol crawling (only dump ldap data)\r\n");
	printf("-l AD_PRIV_LOGIN\tdefine AD username for explicit authentification\r\n");
	printf("-p AD_PRIV_PASSWD\tdefine AD password for explicit authentification\r\n");
	printf("-o OUTPUT_FORMAT\tselect output format (CSV,XML,STDOUT)\r\n");
	printf("-e DEBUG_LEVEL\t\tdefine debug level (default: 5)\r\n");
	printf("-r LDAP_PORT\t\tdefine ldap port (default: 389)\r\n");
	printf("-s DNS_NAME\t\tdefine domain dns name (default: resolved dynamically)\r\n");
	printf("\r\n");
}

VOID DefineOutputFormat(_In_ PCHAR pSelectedOutputFormat)
{
	if (!pSelectedOutputFormat || !pSyscrwlrOptions)
	{
		printf("internal error.\r\n");
		DoExit(D_ERROR);
	}

	if (_strlwr_s(pSelectedOutputFormat, strlen(pSelectedOutputFormat) + 1))
	{
		printf("internal error.\r\n");
		DoExit(D_ERROR);
	}

	pSyscrwlrOptions->bShouldPrintXML = FALSE;
	pSyscrwlrOptions->bShouldPrintCSV = FALSE;
	pSyscrwlrOptions->bShouldPrintSTDOUT = FALSE;

	if (strstr(pSelectedOutputFormat, "xml"))
		pSyscrwlrOptions->bShouldPrintXML = TRUE;

	if (strstr(pSelectedOutputFormat, "csv"))
		pSyscrwlrOptions->bShouldPrintCSV = TRUE;

	if (strstr(pSelectedOutputFormat, "stdout"))
		pSyscrwlrOptions->bShouldPrintSTDOUT = TRUE;
}