/**************************************************
* SysvolCrawler - PREFERENCESParser.h
* AUTHOR: Luc Delsalle	
* 
* Parsing engine for preferences GPO file store
* in PREFERENCES folder
* 
* ANSSI/COSSI/DTO/BAI - 2014
***************************************************/

#ifndef __PREFERENCES_PARSER_H__
#define __PREFERENCES_PARSER_H__

#include "Common.h"
#include "INIGenericParser.h"

//************** <PARSER DEFINITION> *********************
#define PREFERENCES_PARSER_NAME				TEXT("IEAK files handler")
#define PREFERENCES_MATCHING_FILE_REGEXP	TEXT("[NON SUPPORTED]")
#define PREFERENCES_MATCHING_FOLDER_REGEXP	TEXT("*\\Preferences\\*")
//************** </PARSER DEFINITION> ********************

typedef DWORD								PREFERENCES_FILE_EXTENSION;
#define PREFERENCES_UNHANDLE_FILE			0
#define PREFERENCES_INI_FILE				1
#define PREFERENCES_INF_FILE				2

#define PREFERENCES_INI_FILE_EXTENSION		TEXT("ini")
#define PREFERENCES_INF_FILE_EXTENSION		TEXT("inf")

//******* <STORE DATA FOR PREFERENCES FILE> ******
// Gather generic information from PREFERENCES files
typedef struct _PREFERENCES_FILE_DATA
{
	PWCHAR									tFilePath;
	PREFERENCES_FILE_EXTENSION				dwFileType;

	DWORD									dwDataSize;
	PVOID									pvData;
} PREFERENCES_FILE_DATA, *PPREFERENCES_FILE_DATA;
//****** </STORE DATA FOR PREFERENCES FILE> ******

// Forward declaration for printers
extern BOOL PrintData(_In_ PPREFERENCES_FILE_DATA pMiscData);
extern BOOL PrintPreferencesDataHeader(_In_ PTCHAR tFilePath);
extern BOOL PrintPreferencesDataFooter(_In_ PTCHAR tFilePath);

// Parser registration
VOID RegisterPreferencesParser(_Inout_ PPARSER_IDENTIFIER *pParserID);
BOOL ParsePreferencesFile(_In_ PTCHAR tFilePath);
BOOL FreePreferencesFileData(_Inout_ PPREFERENCES_FILE_DATA pPreferencesFileData);

// Determine whuich type of file we wanna parse
PREFERENCES_FILE_EXTENSION GetPreferenceFileExtensionID(_In_ PTCHAR tFilePath);
// Extract PREFERENCES file data and size
BOOL FillPreferencesDataContent(_Inout_ PPREFERENCES_FILE_DATA pPreferencesFileData, _In_ PBYTE pbPreferencesFileRawDATA, _In_ DWORD dwPreferencesFileRawDATALen);
// Parse IEAK file as ini file
BOOL FillIniDataContent(_Inout_ PPREFERENCES_FILE_DATA pPreferencesFileData, _In_ PBYTE pbPreferencesFileRawDATA, _In_ DWORD dwPreferencesFileRawDATALen);
// Parse IEAK file as raw data (need parser implementation for that type of file)
BOOL FillDefaultDataContent(_Inout_ PPREFERENCES_FILE_DATA pPreferencesFileData, _In_ PBYTE pbPreferencesFileRawDATA, _In_ DWORD dwPreferencesFileRawDATALen);

/*****************************************************************
 * HOW TO add new PREFERENCES file parser
 * 1 - Specify new extension id and file extension in header file:
 *     PREFERENCES_INI_FILE_EXTENSION & PREFERENCES_XXX_FILE_EXTENSION
 *
 * 2 - Add switch case in GetFileExtensionID function and implement
 *     dedicated allocation function (eg. FillXXXDataContent)
 *
 * 3 - Fill FillPreferencesDataContent function for the new type of
 *     file
 *
 * 4 - Add memory release code in FreePreferencesFileData  function
 *****************************************************************/
#endif