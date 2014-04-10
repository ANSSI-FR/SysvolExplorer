/**************************************************
* SysvolCrawler - MISCParser.h
* AUTHOR: Luc Delsalle	
* 
* Parsing engine for odd file which should not be
* on a Sysvol folder
*	
* ANSSI/COSSI/DTO/BAI - 2014
***************************************************/

#ifndef __MISC_PARSER_H__
#define __MISC_PARSER_H__

#include "Common.h"

//************** <PARSER DEFINITION> *********************
#define MISC_PARSER_NAME				TEXT("MISCellaneous file parser")
#define MISC_MATCHING_FILE_REGEXP		TEXT("*.*")
#define MISC_MATCHING_FOLDER_REGEXP		TEXT("[NON SUPPORTED]")
//************** </PARSER DEFINITION> ********************

//******* <STORE DATA FOR FILE MISC> ******
// Gather data for misc file
typedef struct _MISC_FILE_DATA
{
	PWCHAR						tFilePath;

	DWORD						dwDataSize;
	PBYTE						pbData;
} MISC_FILE_DATA, *PMISC_FILE_DATA;
//****** </STORE DATA FOR FILE MISC> ******

// Forward declaration for printers
extern BOOL PrintData(_In_ PMISC_FILE_DATA pMiscData);
extern BOOL PrintMiscDataHeader(_In_ PTCHAR tFilePath);
extern BOOL PrintMiscDataFooter(_In_ PTCHAR tFilePath);

// Parser registration
VOID RegisterMiscParser(_Inout_ PPARSER_IDENTIFIER *pParserID);
// Entry point for MISC file
BOOL ParseMiscFile(_In_ PTCHAR tFilePath);

#endif