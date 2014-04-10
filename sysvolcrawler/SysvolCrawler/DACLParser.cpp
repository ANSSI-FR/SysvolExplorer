/**************************************************
* SysvolCrawler - DACLParser.c
* AUTHOR: Luc Delsalle
*
* Extract DACL from GPO files
*
* ANSSI/COSSI/DTO/BAI - 2014
***************************************************/

#include "DACLParser.h"

VOID RegisterDaclParser(_Inout_ PPARSER_IDENTIFIER *pParserID)
{
	*pParserID = (PPARSER_IDENTIFIER)HeapAlloc(hCrawlerHeap, HEAP_ZERO_MEMORY, sizeof (PARSER_IDENTIFIER));
	if (!pParserID)
	{
		DEBUG_LOG(D_ERROR, "Unable to allocate PARSER_IDENTIFIER structure.\r\nExiting now...");
		DoExit(D_ERROR);
	}

	(*pParserID)->tParserName = DACL_PARSER_NAME;
	(*pParserID)->tFileMatchingRegExp = NULL;
	(*pParserID)->tFolderMatchingRegExp = NULL;
	(*pParserID)->pParserEntryPoint = ParseFileDacl;
}

BOOL ParseFileDacl(_In_ PTCHAR tFilePath)
{
	HANDLE hDaclFile = INVALID_HANDLE_VALUE;
	PDACL_FILE_DATA pFileDaclData = NULL;
	DWORD dwRes = 0;
	PACL *ppDacl = NULL;
	PSID psidOwner = NULL;
	PSECURITY_DESCRIPTOR pSecurityDescriptor = NULL;
	PTCHAR tOwnerSidStr = NULL;

	if (tFilePath == NULL)
	{
		DEBUG_LOG(D_ERROR, "FILEPATH pointer invalid.\r\nExiting now...");
		DoExit(D_ERROR);
	}
	DEBUG_LOG(D_MISC, "[DACL] Now parsing %ws\r\n", tFilePath);

	hDaclFile = CreateFile_s(tFilePath, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);	
	if (hDaclFile == INVALID_HANDLE_VALUE)
	{
		return FALSE;
	}

	pFileDaclData = (PDACL_FILE_DATA)HeapAlloc(hCrawlerHeap, HEAP_ZERO_MEMORY, sizeof (DACL_FILE_DATA));
	if (pFileDaclData == NULL)
	{
		DEBUG_LOG(D_ERROR, "pFileDaclData pointer invalid.\r\nExiting now...");
		DoExit(D_ERROR);
	}
	pFileDaclData->tFilePath = tFilePath;

	dwRes = GetSecurityInfo(hDaclFile, SE_FILE_OBJECT, OWNER_SECURITY_INFORMATION | DACL_SECURITY_INFORMATION, &psidOwner, NULL, ppDacl, NULL, &pSecurityDescriptor);
	if (dwRes != ERROR_SUCCESS)
	{
		DEBUG_LOG(D_ERROR, "Unable to retrieve DACL data for file %ws. ErrorCode: %d.\r\n", tFilePath, GetLastError());
		SetLastError(ERROR_ACCESS_DENIED);
		return FALSE;
	}

	if (!ConvertSidToStringSid(psidOwner, &pFileDaclData->tOwnerSid))
	{
		DEBUG_LOG(D_ERROR, "Unable to convert Owner SID for file %ws. ErrorCode: %d.\r\n", tFilePath, GetLastError());
		return FALSE;
	}

	if (!ConvertSecurityDescriptorToStringSecurityDescriptor(pSecurityDescriptor, SDDL_REVISION_1, DACL_SECURITY_INFORMATION, &pFileDaclData->tSDDL, NULL))
	{
		DEBUG_LOG(D_ERROR, "Unable to convert DACL for file %ws. ErrorCode: %d.\r\n", tFilePath, GetLastError());
		return FALSE;
	}

	PrintDaclDataHeader(pFileDaclData->tFilePath);
	PrintData(pFileDaclData);
	PrintDaclDataFooter(pFileDaclData->tFilePath);

	FreeDaclFileData(pFileDaclData);
	return TRUE;
}

BOOL FreeDaclFileData(_Inout_ PDACL_FILE_DATA pDaclData)
{
	if (!pDaclData)
		return TRUE;

	if (pDaclData->tOwnerSid)
		LocalFree(pDaclData->tOwnerSid);

	if (pDaclData->tSDDL)
		LocalFree(pDaclData->tSDDL);

	HeapFree(hCrawlerHeap, NULL, pDaclData);
	return TRUE;
}
