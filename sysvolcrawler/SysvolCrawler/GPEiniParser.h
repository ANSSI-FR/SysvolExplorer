/**************************************************
* SysvolCrawler - GPEiniParser.h
* AUTHOR: Luc Delsalle	
*
* Parsing engine for GPE.ini file
*
* ANSSI/COSSI/DTO/BAI - 2014
***************************************************/

#ifndef __GPEINIPARSER_H__
#define __GPEINIPARSER_H__

#include "Common.h"
#include "INIGenericParser.h"

//************** <PARSER DEFINITION> *********************
#define GPEINI_PARSER_NAME				TEXT("GPE.ini parser")
#define GPEINI_MATCHING_FILE_REGEXP		TEXT("gpe.ini")
#define GPEINI_MATCHING_FOLDER_REGEXP	TEXT("[NON SUPPORTED]")
//************** </PARSER DEFINITION> ********************

#define GPE_MAX_CSE						1024
#define GPE_MAX_CSE_VALUES				1024
#define GPE_GENERAL_SECTION				TEXT("General")
#define GPE_MACHINE_EXTENSION_VERSION	TEXT("MachineExtensionVersions")
#define GPE_USER_EXTENSION_VERSION		TEXT("UserExtensionVersions")

//******* <STORE GPE.INI DATA> ******
typedef struct _GPEINI_CSE_DATA
{
	DWORD						dwCSEValuesNum;
	PTCHAR						pCSEValues[GPE_MAX_CSE_VALUES];
	DWORD						dwCSEID;
} GPEINI_CSE_DATA, *PGPEINI_CSE_DATA;

typedef struct _GPEINI_FILE_DATA
{
	PWCHAR						tFilePath;

	DWORD						dwMachineExtensionVersionsNum;
	PGPEINI_CSE_DATA			pMachineExtensionVersions[GPE_MAX_CSE];

	DWORD						dwUserExtensionVersionsNum;
	PGPEINI_CSE_DATA			pUserExtensionVersions[GPE_MAX_CSE];

	DWORD						dwNumberOfUnReferrencedSections;
	PINI_SECTION_DATA			pUnReferrencedSections[GPE_MAX_CSE];
} GPEINI_FILE_DATA, *PGPEINI_FILE_DATA;
//****** </STORE GPE.INI DATA> ******

// Forward declaration for printers
extern BOOL PrintData(_In_ PGPEINI_FILE_DATA pGpeIniData);
extern BOOL PrintGpeIniDataHeader(_In_ PTCHAR tFilePath);
extern BOOL PrintGpeIniDataFooter(_In_ PTCHAR tFilePath);

// Parser registration
VOID RegisterGpeIniParser(_Inout_ PPARSER_IDENTIFIER *pParserID);
BOOL ParseGpeIniFile(_In_ PTCHAR tFilePath);
BOOL FreeGpeIniFileData(_Inout_ PGPEINI_FILE_DATA pGpeIniFileData);

// Fill GPEINI_FILE_DATA structure
BOOL FillGpeIniAttributes(_Inout_ PGPEINI_FILE_DATA pGpeIniFileData, _In_ PINI_FILE_DATA pGenericIniFileData);
BOOL FillExtensionAttributes(_Inout_ PGPEINI_FILE_DATA pGpeIniFileData, _In_ PINI_SECTION_DATA pGenericIniSection, _In_ DWORD dwSectionNumb);
BOOL FillCSEAttributes(_Inout_ PGPEINI_CSE_DATA pCseData, _In_ PTCHAR tRawCSEAttributes);
PTCHAR ExtractCSEFromProperty(_In_ PTCHAR tProperty, _In_ DWORD dwPropertyLen, _In_ PDWORD pdwIndex);
PTCHAR ExtractCSEValuesFromProperty(_In_ PTCHAR tProperty, _In_ DWORD dwPropertyLen, _In_ PDWORD pdwIndex);
PTCHAR ExtractCSEIdFromProperty(_In_ PTCHAR tProperty);

#endif