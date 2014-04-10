/**************************************************
* SysvolCrawler - POLParser.h
* AUTHOR: Luc Delsalle	
*
* Parsing engine for .pol file (eg. Registry.pol)
*	
* ANSSI/COSSI/DTO/BAI - 2014
***************************************************/

#ifndef __POL_PARSER_H__
#define __POL_PARSER_H__

#include "Common.h"

//************** <PARSER DEFINITION> *********************
#define POL_PARSER_NAME				TEXT("POL parser")
#define POL_MATCHING_FILE_REGEXP	TEXT("Registry.pol")
#define POL_MATCHING_FOLDER_REGEXP	TEXT("[NON SUPPORTED]")
//************** </PARSER DEFINITION> ********************

// Gather information from POL file
typedef struct _POL_DATA
{
	PTCHAR	tFilePath;
	PWCHAR	pwKey;
	PBYTE	pbValue;
	DWORD	dwValueSize;
	PDWORD	pwdType;
	PDWORD	pwdSize;
	PBYTE	pbData;
} POL_DATA, *PPOL_DATA;

// Forward declaration for printers
extern BOOL PrintData(_In_ PPOL_DATA pPolData);
extern BOOL PrintPolDataHeader(_In_ PTCHAR tFilePath);
extern BOOL PrintPolDataFooter(_In_ PTCHAR tFilePath);

// Parser registration
VOID RegisterPOLParser(_Inout_ PPARSER_IDENTIFIER *pParserID);
// Entry point for POL
BOOL ParsePolFile(_In_ PTCHAR tFilePath);
// extract token ([key;value;type;size;data]) from pol file
BOOL ParseBodyRegisteryValues(_In_ PHANDLE hPOLFile, _In_ PTCHAR tFilePath);
// extract subtoken ([key;value;type;size;data]) from every token
BOOL ExtractSubToken(_Inout_ PPOL_DATA pPolDATA, _In_ PBYTE pToken, _In_ INT iSubTokenStartPos, _In_ INT iSubTokenLen, _In_ DWORD dwSubTokenIndex);
#endif