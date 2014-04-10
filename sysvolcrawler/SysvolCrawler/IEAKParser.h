/**************************************************
* SysvolCrawler - IEAKParser.h
* AUTHOR: Luc Delsalle	
* 
* Parsing engine for Internet Explorer file
* (store in IEAK folder)
*	
* ANSSI/COSSI/DTO/BAI - 2014
***************************************************/

#ifndef __IEAK_PARSER_H__
#define __IEAK_PARSER_H__

#include "Common.h"
#include "INIGenericParser.h"

//************** <PARSER DEFINITION> *********************
#define IEAK_PARSER_NAME			TEXT("IEAK files handler")
#define IEAK_MATCHING_FILE_REGEXP	TEXT("[NON SUPPORTED]")
#define IEAK_MATCHING_FOLDER_REGEXP	TEXT("*\\IEAK\\*")
//************** </PARSER DEFINITION> ********************

typedef DWORD								IEAK_FILE_EXTENSION;
#define IEAK_UNHANDLE_FILE					0
#define IEAK_INI_FILE						1
#define IEAK_INF_FILE						2

#define IEAK_INI_FILE_EXTENSION				TEXT("ini")
#define IEAK_INF_FILE_EXTENSION				TEXT("inf")

//******* <STORE DATA FOR FILE DANS IEAK> ******
// Gather information for IEAK files
typedef struct _IEAK_FILE_DATA
{
	PWCHAR						tFilePath;
	IEAK_FILE_EXTENSION			dwFileType;

	DWORD						dwDataSize;
	PVOID						pvData;
} IEAK_FILE_DATA, *PIEAK_FILE_DATA;
//****** </STORE DATA FOR FILE DANS IEAK> ******

// Forward declaration for printers
extern BOOL PrintData(_In_ PIEAK_FILE_DATA pMiscData);
extern BOOL PrintIeakDataHeader(_In_ PTCHAR tFilePath);
extern BOOL PrintIeakDataFooter(_In_ PTCHAR tFilePath);

// Parser registration
VOID RegisterIeakParser(_Inout_ PPARSER_IDENTIFIER *pParserID);
BOOL ParseIeakFile(_In_ PTCHAR tFilePath);
BOOL FreeIeakFileData(_Inout_ PIEAK_FILE_DATA pIeakFileData);

// Guess what kind of file we are gonna parse
IEAK_FILE_EXTENSION GetIEAKFileExtensionID(_In_ PTCHAR tFilePath);
// Extract IEAK file data and size
BOOL FillIeakDataContent(_Inout_ PIEAK_FILE_DATA pIeakFileData, _In_ PBYTE pbIeakFileRawDATA, _In_ DWORD dwIeakFileRawDATALen);
// Parse IEAK file as ini file
BOOL FillIniDataContent(_Inout_ PIEAK_FILE_DATA pIeakFileData, _In_ PBYTE pbIeakFileRawDATA, _In_ DWORD dwIeakFileRawDATALen);
// Parse IEAK file as raw data (need parser implementation for that type of file)
BOOL FillDefaultDataContent(_Inout_ PIEAK_FILE_DATA pIeakFileData, _In_ PBYTE pbIeakFileRawDATA, _In_ DWORD dwIeakFileRawDATALen);

/*****************************************************************
 * HOW TO add new IEAK file parser
 * 1 - Specify new extension id and file extension in header file:
 *     IEAK_FILE_EXTENSION & IEAK_XXX_FILE_EXTENSION
 *
 * 2 - Add switch case in GetFileExtensionID function and implement
 *     dedicated allocation function (eg. FillXXXDataContent)
 *
 * 3 - Fill FillIeakDataContent function for the new type of file
 *
 * 4 - Add memory release code in FreeIeakFileData function
 *****************************************************************/
#endif