/**************************************************
* SysvolCrawler - LDAPPrinter.h
* AUTHOR: Luc Delsalle	
*
* Display or export data extracted from LDAP 
* directory
*	
* ANSSI/COSSI/DTO/BAI - 2014
***************************************************/

#ifndef __LDAPPRINTER_H__
#define __LDAPPRINTER_H__

#include "Common.h"
#include "LDAPCrawler.h"
#include "PrinterCommon.h"

//************** <PRINTER DEFINITION> *********************
#define OUTPUT_NAME_USERS_FILE		TEXT("LDAPUsersFile")
#define OUTPUT_NAME_GROUPS_FILE		TEXT("LDAPGroupsFile")
#define OUTPUT_NAME_OUS_FILE			TEXT("LDAPOusFile")
#define OUTPUT_NAME_GPOS_FILE			TEXT("LDAPGPOsFile")
#define OUTPUT_DIRECTORY_LDAP_FILE	TEXT(".\\")
//************** </PRINTER DEFINITION> ********************

// Generic dispatcher for printers
BOOL PrintData(_In_ PLDAP_AD_INFOS pLdapADInfos);
BOOL PrintLdapDataHeader();
BOOL PrintLdapDataFooter();
BOOL PrintSpecifiedData(_In_ PLDAP_AD_INFOS pLdapADInfos, _In_ LDAP_REQUESTED_DATA_INFO dwRequestedInfo);

// Printers for file format
BOOL PrintXMLData(_In_ PLDAP_AD_INFOS pLdapADInfos);
BOOL PrintCSVData(_In_ PLDAP_AD_INFOS pLdapADInfos);
BOOL PrintSTDOUTData(_In_ PLDAP_AD_INFOS pLdapADInfos);

BOOL PrintXMLDataUsers(_In_ HANDLE hXMLUsersFile, _In_ PLDAP_AD_INFOS pLdapADInfos);
BOOL PrintXMLDataGroups(_In_ HANDLE hXMLGroupsFile, _In_ PLDAP_AD_INFOS pLdapADInfos);
BOOL PrintXMLDataOUs(_In_ HANDLE hXMLOusFile, _In_ PLDAP_AD_INFOS pLdapADInfos);
BOOL PrintXMLDataGPOs(_In_ HANDLE hXMLGPOsFile, _In_ PLDAP_AD_INFOS pLdapADInfos);

BOOL PrintCSVDataUsers(_In_ HANDLE hCSVUsersFile, _In_ PLDAP_AD_INFOS pLdapADInfos);
BOOL PrintCSVDataGroups(_In_ HANDLE hCSVGroupsFile, _In_ PLDAP_AD_INFOS pLdapADInfos);
BOOL PrintCSVDataOUs(_In_ HANDLE hCSVOusFile, _In_ PLDAP_AD_INFOS pLdapADInfos);
BOOL PrintCSVDataGPOs(_In_ HANDLE hCSVGPOsFile, _In_ PLDAP_AD_INFOS pLdapADInfos);

BOOL PrintSTDOUTDataUsers(_In_ PLDAP_AD_INFOS pLdapADInfos);
BOOL PrintSTDOUTDataGroups(_In_ PLDAP_AD_INFOS pLdapADInfos);
BOOL PrintSTDOUTDataOUs(_In_ PLDAP_AD_INFOS pLdapADInfos);
BOOL PrintSTDOUTDataGPOs(_In_ PLDAP_AD_INFOS pLdapADInfos);

#endif
