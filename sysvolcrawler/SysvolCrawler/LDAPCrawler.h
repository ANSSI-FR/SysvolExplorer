/**************************************************
* SysvolCrawler - LDAPCrawler.h
* AUTHOR: Luc Delsalle	
*
* Extract GPO metadata from LDAP Directory
*	
* ANSSI/COSSI/DTO/BAI - 2014
***************************************************/

#ifndef __LDAPCRAWLER_H__
#define __LDAPCRAWLER_H__

#include "Common.h"
#include <Winldap.h>
#include <Winber.h>
#include <sddl.h>

#define AD_LDAP_SEARCH_LIMIT				1
#define LDAP_SEARCH_USERS_FILTER			TEXT("(&(objectCategory=person)(objectClass=user))")
#define LDAP_SEARCH_GROUPS_FILTER			TEXT("(&(&(objectCategory=group)(objectClass=group))(name=*))")
#define LDAP_SEARCH_OUS_FILTER				TEXT("(|(&(objectCategory=organizationalUnit)(objectClass=organizationalUnit))(objectClass=domainDNS))")
#define LDAP_SEARCH_GPO_FILTER				TEXT("(&(objectCategory=groupPolicyContainer)(objectClass=groupPolicyContainer))")

#define LDAP_USER_TARGETED					1
#define LDAP_USER_TARGETED_INFO_CN			TEXT("cn")
#define LDAP_USER_TARGETED_INFO_UPN			TEXT("userPrincipalName")
#define LDAP_USER_TARGETED_INFO_LLOGON		TEXT("lastLogon")
#define LDAP_USER_TARGETED_INFO_PWD_LS		TEXT("pwdLastSet")
#define LDAP_USER_TARGETED_INFO_MOF			TEXT("memberOf")
#define LDAP_USER_TARGETED_INFO_NAME		TEXT("name")
#define LDAP_USER_TARGETED_INFO_SD			TEXT("nTSecurityDescriptor")
#define LDAP_USER_TARGETED_INFO_SID			TEXT("objectSid")

#define LDAP_GROUP_TARGETED					2
#define LDAP_GROUP_TARGETED_INFO_CN			TEXT("cn")
#define LDAP_GROUP_TARGETED_INFO_DESC		TEXT("description")
#define LDAP_GROUP_TARGETED_INFO_GTYPE		TEXT("groupType")
#define LDAP_GROUP_TARGETED_INFO_MEMBER		TEXT("member")
#define LDAP_GROUP_TARGETED_INFO_NAME		TEXT("name")
#define LDAP_GROUP_TARGETED_INFO_SD			TEXT("nTSecurityDescriptor")
#define LDAP_GROUP_TARGETED_INFO_SID		TEXT("objectSid")

#define LDAP_OU_TARGETED					3
#define LDAP_OU_TARGETED_INFO_OU			TEXT("ou")
#define LDAP_OU_TARGETED_INFO_DESC			TEXT("description")
#define LDAP_OU_TARGETED_INFO_NAME			TEXT("name")
#define LDAP_OU_TARGETED_INFO_GPLINK		TEXT("gPLink")
#define LDAP_OU_TARGETED_INFO_GPOPTIONS		TEXT("gPOptions")
#define LDAP_OU_TARGETED_INFO_SD			TEXT("nTSecurityDescriptor")

#define LDAP_GPO_TARGETED					4
#define LDAP_GPO_TARGETED_INFO_CN			TEXT("cn")
#define LDAP_GPO_TARGETED_INFO_CREATED		TEXT("whenCreated")
#define LDAP_GPO_TARGETED_INFO_MODIFIED		TEXT("whenChanged")
#define LDAP_GPO_TARGETED_INFO_DISPLAY		TEXT("displayName")
#define LDAP_GPO_TARGETED_INFO_FLAGS		TEXT("flags")
#define LDAP_GPO_TARGETED_INFO_VERSION		TEXT("versionNumber")
#define LDAP_GPO_TARGETED_INFO_FUNCVER		TEXT("gPCFunctionalityVersion")
#define LDAP_GPO_TARGETED_INFO_FILEPATH		TEXT("gPCFileSysPath")
#define LDAP_GPO_TARGETED_INFO_EXT_COMP		TEXT("gPCMachineExtensionNames")
#define LDAP_GPO_TARGETED_INFO_EXT_USR		TEXT("gPCUserExtensionNames")
#define LDAP_GPO_TARGETED_INFO_PROPAG		TEXT("dSCorePropagationData")
#define LDAP_GPO_TARGETED_INFO_WQLFILTER	TEXT("gPCWQLFilter")
#define LDAP_GPO_TARGETED_INFO_SD			TEXT("nTSecurityDescriptor")

//******* <LDAP AUTHENTIFICATION CONTEXT> ******
typedef struct _LDAP_CONNECT_INFOS
{
	PLDAP						hLDAPConnection;
	PTCHAR						ptLDAPDomainDN;
	PTCHAR						ptLDAPDomainName;
} LDAP_CONNECT_INFOS, *PLDAP_CONNECT_INFO;
//******* </LDAP AUTHENTIFICATION CONTEXT> ******

//******* <LDAP DATA STRUCTURES> ******
typedef DWORD LDAP_REQUESTED_DATA_INFO;

// Store temporary data
typedef struct _LDAP_RETRIEVED_DATA
{
	PTCHAR						tDN;
	PBYTE						*ppbData;
	PDWORD						pdwDataSize;
	DWORD						dwElementCount;
} LDAP_RETRIEVED_DATA, *PLDAP_RETRIEVED_DATA;

// Store generic user data
typedef struct _LDAP_AD_USER
{
	PTCHAR						tCN;
	PTCHAR						tName;
	PTCHAR						tDistinguishedName;
	PTCHAR						tUserPrincipalName;
	PTCHAR						tLastLogon;
	PTCHAR						tPwdLastSet;
	PTCHAR						tMemberOf;
	PTCHAR						tSecurityDescriptor;
	PTCHAR						tSid;
} LDAP_AD_USER, *PLDAP_AD_USER;

// Store generic user group data
typedef struct _LDAP_AD_GROUP
{
	PTCHAR						tCN;
	PTCHAR						tName;
	PTCHAR						tDistinguishedName;
	PTCHAR						tDescription;
	PTCHAR						tGroupType;
	PTCHAR						tMember;
	PTCHAR						tSecurityDescriptor;
	PTCHAR						tSid;
} LDAP_AD_GROUP, *PLDAP_AD_GROUP;

// Store generic user data
typedef struct _LDAP_AD_OU
{
	PTCHAR						tOu;
	PTCHAR						tDistinguishedName;
	PTCHAR						tDescription;
	PTCHAR						tName;
	PTCHAR						tGpLink;
	PTCHAR						tGpOptions;
	PTCHAR						tSecurityDescriptor;
} LDAP_AD_OU, *PLDAP_AD_OU;

// Store generic GPO data
typedef struct _LDAP_AD_GPO
{
	PTCHAR						tCN;
	PTCHAR						tDistinguishedName;
	PTCHAR						tWhenCreated;
	PTCHAR						tWhenChanged;
	PTCHAR						tDisplayName;
	PTCHAR						tFlags;
	PTCHAR						tVersionNumber;
	PTCHAR						tFunctionalityVersion;
	PTCHAR						tFileSysPath;
	PTCHAR						tMachineExtensionsNames;
	PTCHAR						tUserExtensionsNames;
	PTCHAR						tCorePropagationData;
	PTCHAR						tWQLFilter;
	PTCHAR						tSecurityDescriptor;
} LDAP_AD_GPO, *PLDAP_AD_GPO;

// Store ldap crawling results
typedef struct _LDAP_AD_INFOS
{
	DWORD						dwNumberOfUser;
	PLDAP_AD_USER				*pUsers;

	DWORD						dwNumberOfGroup;
	PLDAP_AD_GROUP				*pGroups;

	DWORD						dwNumberOfOU;
	PLDAP_AD_OU					*pOUs;

	DWORD						dwNumberOfGPO;
	PLDAP_AD_GPO				*pGPOs;
} LDAP_AD_INFOS, *PLDAP_AD_INFOS;

//******* </LDAP DATA STRUCTURES> ******

// Forward declaration defining ldap authentification context
extern PLDAP_CONNECT_INFO pLDAPConnectInfo;

// Connect/Disconnect from LDAP
BOOL InitToLDAP(_In_ PTCHAR ptHostName, _In_ ULONG dwPortNumber);
BOOL BindToLDAP(_In_ PTCHAR ptUserName, _In_ PTCHAR ptPassword);
BOOL ExtractDomainNamingContext();
BOOL DisconnectFromLDAP();
BOOL FreeLDAPInfo(_Inout_ PLDAP_AD_INFOS pLdapADInfos);
BOOL FreeLDAPUsersInfo(_Inout_ PLDAP_AD_INFOS pLdapADInfos);
BOOL FreeLDAPGroupsInfo(_Inout_ PLDAP_AD_INFOS pLdapADInfos);
BOOL FreeLDAPOUsInfo(_Inout_ PLDAP_AD_INFOS pLdapADInfos);
BOOL FreeLDAPGPOsInfo(_Inout_ PLDAP_AD_INFOS pLdapADInfos);

BOOL LDAPExtractDomainUsers(_Inout_ PLDAP_AD_INFOS pLdapADInfos);
BOOL LDAPExtractDomainGroups(_Inout_ PLDAP_AD_INFOS pLdapADInfos);
BOOL LDAPExtractOrganizationalUnits(_Inout_ PLDAP_AD_INFOS pLdapADInfos);
BOOL LDAPExtractGPOs(_Inout_ PLDAP_AD_INFOS pLdapADInfos);

BOOL FillDomainUserStruct(_Inout_ PLDAP_AD_USER pLdapADUser, _In_ PTCHAR ptAttribute, _In_ PTCHAR ptValue);
BOOL FillDomainGroupStruct(_Inout_ PLDAP_AD_GROUP pLdapADGroup, _In_ PTCHAR ptAttribute, _In_ PTCHAR ptValue);
BOOL FillDomainOUStruct(_Inout_ PLDAP_AD_OU pLdapADOU, _In_ PTCHAR ptAttribute, _In_ PTCHAR ptValue);
BOOL FillDomainGPOStruct(_Inout_ PLDAP_AD_GPO pLdapADGPO, _In_ PTCHAR ptAttribute, _In_ PTCHAR ptValue);

PLDAP_AD_USER GetLDAPADUser(PLDAP_AD_USER *pADUsers, DWORD dwADUsersCount, PTCHAR tDN);
PLDAP_AD_GROUP GetLDAPADGroup(PLDAP_AD_GROUP *pADGroups, DWORD dwADGroupsCount, PTCHAR tDN);
PLDAP_AD_OU GetLDAPADOu(PLDAP_AD_OU *pADOus, DWORD dwADOusCount, PTCHAR tDN);
PLDAP_AD_GPO GetLDAPADGpo(PLDAP_AD_GPO *pADGpos, DWORD dwADGposCount, PTCHAR tDN);

BOOL LDAPDoPageSearch(_In_ PTCHAR tLdapFilter, _In_ PTCHAR tOrigAttribute, _Inout_ PLDAP_RETRIEVED_DATA **ppRetrievedResults, _Inout_ PDWORD dwResultsCount);
BOOL LDAPExtractAttributes(_In_ PLDAPMessage pCurrentEntry, _In_ PTCHAR tAttribute, _Inout_ PLDAP_RETRIEVED_DATA *ppRetrievedData);
BOOL LDAPExtractRangedAttributes(_In_ PLDAPMessage pCurrentEntry, _In_ PTCHAR tOrigAttribute, _In_ PTCHAR tAttribute, _Inout_ PLDAP_RETRIEVED_DATA *ppRetrievedData);

BOOL GetRangeValues(_Inout_ PLDAPMessage pEntry, _In_ PTCHAR tOriginalAttribute, _Inout_ PDWORD pdwAttributeCount, _Inout_ PBYTE **pppbData, _Inout_ PDWORD *ppdwDataSize);
BOOL ParseRange(_In_ PTCHAR tAtttype, _In_ PTCHAR tAttdescr, _Inout_ PDWORD pdwStart, _Inout_ PDWORD pdwEnd);
BOOL ConstructRangeAtt(_In_ PTCHAR tAtttype, _In_ DWORD dwStart, _In_ INT iEnd, _Inout_ PTCHAR* tOutputRangeAttr);

#endif