/**************************************************
* SysvolCrawler - DENIEDParser.h
* AUTHOR: Luc Delsalle	
* 
* Parsing engine for file which couldn't be
* opened during CreateFile attempt
*	
* ANSSI/COSSI/DTO/BAI - 2014
***************************************************/

#ifndef __DENIED_PARSER_H__
#define __DENIED_PARSER_H__

#include "Common.h"

//************** <PARSER DEFINITION> *********************
#define DENIED_PARSER_NAME				TEXT("DENIED file parser")
#define DENIED_MATCHING_FILE_REGEXP		TEXT("*.*")
#define DENIED_MATCHING_FOLDER_REGEXP	TEXT("[NON SUPPORTED]")
//************** </PARSER DEFINITION> ********************

//******* <STORE DATA FOR FILE DENIED> ******
// Gather information for ACCESS_DENIED file
typedef struct _DENIED_FILE_DATA
{
	PWCHAR						tFilePath;
	BOOL						bIsADirectory;
} DENIED_FILE_DATA, *PDENIED_FILE_DATA;
//****** </STORE DATA FOR FILE DENIED> ******

// Forward declaration for printers
extern BOOL PrintData(_In_ PDENIED_FILE_DATA pDeniedData);
extern BOOL PrintDeniedDataHeader(_In_ PTCHAR tFilePath);
extern BOOL PrintDeniedDataFooter(_In_ PTCHAR tFilePath);

// Parser registration
VOID RegisterDeniedParser(_Inout_ PPARSER_IDENTIFIER *pParserID);
// Entry point for ACCESS_DENIED file
BOOL ParseDeniedFile(_In_ PTCHAR tFilePath);

#endif