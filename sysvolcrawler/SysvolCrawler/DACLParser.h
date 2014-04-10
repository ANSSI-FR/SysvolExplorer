/**************************************************
* SysvolCrawler - DACLParser.c
* AUTHOR: Luc Delsalle
*
* Extract DACL from GPO files
*
* ANSSI/COSSI/DTO/BAI - 2014
***************************************************/

#ifndef __DACL_PARSER_H__
#define __DACL_PARSER_H__

#include "Common.h"
#include "Sddl.h"
#include "accctrl.h"
#include "aclapi.h"

//************** <PARSER DEFINITION> *********************
#define DACL_PARSER_NAME					TEXT("DACL parser")
#define DACL_MATCHING_FILE_REGEXP			TEXT("[NON SUPPORTED]")
#define DACL_MATCHING_FOLDER_REGEXP			TEXT("[NON SUPPORTED]")
//************** </PARSER DEFINITION> ********************

// Gather DACL information
typedef struct _DACL_FILE_DATA
{
	PWCHAR									tFilePath;
	PTCHAR									tOwnerSid;
	PTCHAR									tSDDL;
} DACL_FILE_DATA, *PDACL_FILE_DATA;

// Forward declaration for printers
extern BOOL PrintData(_In_ PDACL_FILE_DATA pDaclData);
extern BOOL PrintDaclDataHeader(_In_ PTCHAR tFilePath);
extern BOOL PrintDaclDataFooter(_In_ PTCHAR tFilePath);

// Parser registration
VOID RegisterDaclParser(_Inout_ PPARSER_IDENTIFIER *pParserID);
// Entry point for DACL parsing
BOOL ParseFileDacl(_In_ PTCHAR tFilePath);
BOOL FreeDaclFileData(_Inout_ PDACL_FILE_DATA pDaclData);
#endif