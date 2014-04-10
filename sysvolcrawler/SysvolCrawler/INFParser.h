/**************************************************
* SysvolCrawler - INFParser.h
* AUTHOR: Luc Delsalle	
*
* Parsing engine for .inf file like GptTmpl.inf
*	
* ANSSI/COSSI/DTO/BAI - 2014
***************************************************/

#ifndef __INF_PARSER_H__
#define __INF_PARSER_H__

#include "Common.h"

//************** <PARSER DEFINITION> *********************
#define INF_PARSER_NAME				TEXT("INF parser")
#define INF_MATCHING_FILE_REGEXP	TEXT("GptTmpl.inf")
#define INF_MATCHING_FOLDER_REGEXP	TEXT("[NON SUPPORTED]")
//************** </PARSER DEFINITION> ********************

#define INF_COMMENT_SYMBOL ';'
#define INF_ESCAPE_SYMBOL '\\'
#define INF_PROPERTY_SEPARATOR_SYMBOL '='
#define MAX_INF_SECTIONS 1024
#define MAX_INF_PROPERTIES 4096

//******* <STORE DATA FOR FILE INF> ******
// Generic structure for '[sample]' section
typedef struct _INF_PROPERTY_DATA
{
	PWCHAR						tName;
	PWCHAR						tValue;
} INF_PROPERTY_DATA, *PINF_PROPERTY_DATA;

// Generic structure for '[sample]' section
typedef struct _INF_SECTION_DATA
{
	PWCHAR						tSectionName;

	DWORD						iNumberOfProperty;
	PINF_PROPERTY_DATA			pProperties[MAX_INF_PROPERTIES];
} INF_SECTION_DATA, *PINF_SECTION_DATA;

// Gather INF data
typedef struct _INF_FILE_DATA
{
	PWCHAR						tFilePath;

	DWORD						iNumberOfSection;
	PINF_SECTION_DATA			pSections[MAX_INF_SECTIONS];
} INF_FILE_DATA, *PINF_FILE_DATA;
//****** </STORE DATA FOR FILE INF> ******

// Forward declaration for printers
extern BOOL PrintData(_In_ PINF_FILE_DATA pInfData);
extern BOOL PrintInfDataHeader(_In_ PTCHAR tFilePath);
extern BOOL PrintInfDataFooter(_In_ PTCHAR tFilePath);

// Parser registration
VOID RegisterInfParser(_Inout_ PPARSER_IDENTIFIER *pParserID);
// Entry point for INF
BOOL ParseInfFile(_In_ PTCHAR tFilePath);

BOOL IsLineContainASection(_In_ PWCHAR tLine, _In_ PWCHAR *pSectionName);
BOOL IsLineComment(_In_ PWCHAR tLine);
BOOL AddNewSection(_Inout_ PINF_FILE_DATA pInfData, _In_ PWCHAR pSectionName, _In_ PINF_SECTION_DATA *pOutNewSection);
BOOL AddNewProperty(_Inout_ PINF_SECTION_DATA pSectionData, _In_ PWCHAR tRawValue);
BOOL FreeInfFileData(_Inout_ PINF_FILE_DATA pInfData);

#endif