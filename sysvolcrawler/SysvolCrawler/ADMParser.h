/**************************************************
* SysvolCrawler - MISCParser.h
* AUTHOR: Luc Delsalle
*
* Parsing engine for ADM file
*
* ANSSI/COSSI/DTO/BAI - 2014
***************************************************/

#ifndef __ADM_PARSER_H__
#define __ADM_PARSER_H__

#include "Common.h"

//************** <PARSER DEFINITION> *********************
#define ADM_PARSER_NAME				TEXT("ADM file parser")
#define ADM_MATCHING_FILE_REGEXP		TEXT("*.adm")
#define ADM_MATCHING_FOLDER_REGEXP		TEXT("[NON SUPPORTED]")
//************** </PARSER DEFINITION> ********************

//******* <STORE DATA FOR MISC FILE> ******
// Gather generic data for misc file
typedef struct _ADM_FILE_DATA
{
	PWCHAR						tFilePath;

	DWORD						dwDataSize;
	PBYTE						pbData;
} ADM_FILE_DATA, *PADM_FILE_DATA;
//****** </STORE DATA FOR MISC FILE> ******

// Forward declaration for printers
extern BOOL PrintData(_In_ PADM_FILE_DATA pAdmData);
extern BOOL PrintAdmDataHeader(_In_ PTCHAR tFilePath);
extern BOOL PrintAdmDataFooter(_In_ PTCHAR tFilePath);

// Parser registration
VOID RegisterAdmParser(_Inout_ PPARSER_IDENTIFIER *pParserID);
// Entry point for misc file
BOOL ParseAdmFile(_In_ PTCHAR tFilePath);

#endif