/**************************************************
* SysvolCrawler - GPTiniParser.h
* AUTHOR: Luc Delsalle	
*
* Parsing engine for GPT.ini file
*	
* ANSSI/COSSI/DTO/BAI - 2014
***************************************************/

#ifndef __GPTINIPARSER_H__
#define __GPTINIPARSER_H__

#include "Common.h"
#include "INIGenericParser.h"

//************** <PARSER DEFINITION> *********************
#define GPTINI_PARSER_NAME				TEXT("GPT.ini parser")
#define GPTINI_MATCHING_FILE_REGEXP		TEXT("gpt.ini")
#define GPTINI_MATCHING_FOLDER_REGEXP	TEXT("[NON SUPPORTED]")
//************** </PARSER DEFINITION> ********************

#define GPT_GENERAL_SECTION "General"
#define GPT_GENERAL_VERSION "Version"
#define GPT_GENERAL_DISPLAYNAME "displayName"

//******* <STORE GPT.INI FILE DATA> ******
typedef struct _GPTINI_FILE_DATA
{
	PWCHAR						tFilePath;

	PTCHAR						tVersion;
	PTCHAR						tDisplayName;

	DWORD						iNumberOfUnReferrencedSections;
	PINI_SECTION_DATA			pUnReferrencedSections[MAX_INI_SECTIONS];
} GPTINI_FILE_DATA, *PGPTINI_FILE_DATA;
//****** <STORE GPT.INI FILE DATA> ******

// Forward declaration for printers
extern BOOL PrintData(_In_ PGPTINI_FILE_DATA pGptIniData);
extern BOOL PrintGptIniDataHeader(_In_ PTCHAR tFilePath);
extern BOOL PrintGptIniDataFooter(_In_ PTCHAR tFilePath);

// Parser registration
VOID RegisterGptIniParser(_Inout_ PPARSER_IDENTIFIER *pParserID);
BOOL ParseGptIniFile(_In_ PTCHAR tFilePath);
BOOL FreeGptIniFileData(_Inout_ PGPTINI_FILE_DATA pGptIniFileData);

#endif