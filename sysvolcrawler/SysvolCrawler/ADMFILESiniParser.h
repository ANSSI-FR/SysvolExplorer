/**************************************************
* SysvolCrawler - ADMFILESiniParser.h
* AUTHOR: Luc Delsalle	
*
* Parsing engine for administrative template file
*	
* ANSSI/COSSI/DTO/BAI - 2014
***************************************************/

#ifndef __ADMFILESINIPARSER_H__
#define __ADMFILESINIPARSER_H__

#include "Common.h"
#include "INIGenericParser.h"

//************** <PARSER DEFINITION> *********************
#define ADMFILESINI_PARSER_NAME				TEXT("ADMFILES.ini parser")
#define ADMFILESINI_MATCHING_FILE_REGEXP	TEXT("admfiles.ini")
#define ADMFILESINI_MATCHING_FOLDER_REGEXP	TEXT("[NON SUPPORTED]")
//************** </PARSER DEFINITION> ********************

#define ADMFILES_MAX_FILES						1024
#define ADMFILES_FILELIST_SECTION				TEXT("FileList")

//******* <STORE ADMFILES.INI INFORMATION> ******
typedef struct _ADMFILESINI_ADM_DATA
{
	PWCHAR						tAdmName;
	DWORD						dwAdmVersion;
} ADMFILESINI_ADM_DATA, *PADMFILESINI_ADM_DATA;

typedef struct _ADMFILESINI_FILE_DATA
{
	PWCHAR						tFilePath;

	DWORD						dwAdmFileListNum;
	PADMFILESINI_ADM_DATA		pAdmFileList[ADMFILES_MAX_FILES];

	DWORD						dwNumberOfUnReferrencedSections;
	PINI_SECTION_DATA			pUnReferrencedSections[ADMFILES_MAX_FILES];
} ADMFILESINI_FILE_DATA, *PADMFILESINI_FILE_DATA;
//****** </STORE ADMFILES.INI INFORMATION> ******

// Forward declaration for printers
extern BOOL PrintData(_In_ PADMFILESINI_FILE_DATA pAdmFilesIniData);
extern BOOL PrintAdmFilesIniDataHeader(_In_ PTCHAR tFilePath);
extern BOOL PrintAdmFilesIniDataFooter(_In_ PTCHAR tFilePath);

// Parser registration
VOID RegisterAdmFilesIniParser(_Inout_ PPARSER_IDENTIFIER *pParserID);
// Entry point for GPE.ini
BOOL ParseAdmFilesIniFile(_In_ PTCHAR tFilePath);
BOOL FreeAdmFilesIniFileData(_In_ PADMFILESINI_FILE_DATA pAdmFilesIniData);

BOOL FillAdmFilesIniMethods(_Inout_ PADMFILESINI_FILE_DATA pAdmFilesIniData, _In_ PINI_FILE_DATA pGenericIniFileData);
BOOL FillAdmFilesIniMethodsActions(_Inout_ PADMFILESINI_FILE_DATA pAdmFilesIniData, _In_ PINI_SECTION_DATA pGenericIniSection, _In_ DWORD dwSectionNumb);

#endif