/**************************************************
* SysvolCrawler - SCRIPTSiniParser.h
* AUTHOR: Luc Delsalle	
* 
* Parsing engine for scripts.ini file
*	
* ANSSI/COSSI/DTO/BAI - 2014
***************************************************/

#ifndef __SCRIPTSINIPARSER_H__
#define __SCRIPTSINIPARSER_H__

#include "Common.h"
#include "INIGenericParser.h"

//************** <PARSER DEFINITION> *********************
#define SCRIPTSINI_PARSER_NAME				TEXT("scripts.ini parser")
#define SCRIPTSINI_MATCHING_FILE_REGEXP		TEXT("scripts.ini")
#define SCRIPTSINI_MATCHING_FOLDER_REGEXP	TEXT("[NON SUPPORTED]")
//************** </PARSER DEFINITION> ********************

//******* <STORE SCRIPTS.INI FILE DATA> ******
typedef struct _SCRIPTSINI_ACTION_DATA
{
	PTCHAR						tCmdLine;
	PTCHAR						tParameters;
} SCRIPTSINI_ACTION_DATA, *PSCRIPTSINI_ACTION_DATA;

typedef struct _SCRIPTSINI_FILE_DATA
{
	PWCHAR						tFilePath;

	DWORD						dwLogonScriptNum;
	PSCRIPTSINI_ACTION_DATA		pLogonScripts[MAX_INI_PROPERTIES];

	DWORD						dwLogoffScriptNum;
	PSCRIPTSINI_ACTION_DATA		pLogoffScripts[MAX_INI_PROPERTIES];

	DWORD						dwStartupScriptNum;
	PSCRIPTSINI_ACTION_DATA		pStartupScripts[MAX_INI_PROPERTIES];

	DWORD						dwShutdownScriptNum;
	PSCRIPTSINI_ACTION_DATA		pShutdownScripts[MAX_INI_PROPERTIES];

	DWORD						dwNumberOfUnReferrencedSections;
	PINI_SECTION_DATA			pUnReferrencedSections[MAX_INI_SECTIONS];
} SCRIPTSINI_FILE_DATA, *PSCRIPTSINI_FILE_DATA;
//****** <STORE SCRIPTS.INI FILE DATA> ******

#define SCRIPTS_LOGON_SECTION			TEXT("Logon")
#define SCRIPTS_LOGOFF_SECTION			TEXT("Logoff")
#define SCRIPTS_STARTUP_SECTION			TEXT("Startup")
#define SCRIPTS_SHUTDOWN_SECTION		TEXT("Shutdown")

#define SCRIPTS_CMDLINE_PROPERTY_NAME	TEXT("CmdLine")
#define SCRIPTS_PARAM_PROPERTY_NAME		TEXT("Parameters")

typedef DWORD							SCRIPTS_SECTION_ID;
#define SCRIPTS_LOGON_SECTION_ID		0x1
#define SCRIPTS_LOGOFF_SECTION_ID		0x2
#define SCRIPTS_STARTUP_SECTION_ID		0x3
#define SCRIPTS_SHUTDOWN_SECTION_ID		0x4

// Forward declaration for printers
extern BOOL PrintData(_In_ PSCRIPTSINI_FILE_DATA pScriptsIniData);
extern BOOL PrintScriptsIniDataHeader(_In_ PTCHAR tFilePath);
extern BOOL PrintScriptsIniDataFooter(_In_ PTCHAR tFilePath);

// Parser registration
VOID RegisterScriptsIniParser(_Inout_ PPARSER_IDENTIFIER *pParserID);
// Entry point for scripts.ini file
BOOL ParseScriptsIniFile(_In_ PTCHAR tFilePath);
BOOL FreeScriptsIniFileData(_Inout_ PSCRIPTSINI_FILE_DATA pScriptsIniFileData);

// internal functions
BOOL FillScriptsIniMethods(_Inout_ PSCRIPTSINI_FILE_DATA pScriptsIniFileData, _In_ PINI_FILE_DATA pGenericIniFileData);
BOOL FillScriptsIniMethodsActions(_Inout_ PSCRIPTSINI_FILE_DATA pScriptsIniFileData, _In_ PINI_SECTION_DATA pGenericIniSection, _In_ DWORD dwSectionNumb, _In_ SCRIPTS_SECTION_ID dwSectionID);

#endif