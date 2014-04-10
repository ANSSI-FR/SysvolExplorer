/**************************************************
* SysvolCrawler - INICommon.h
* AUTHOR: Luc Delsalle	
*
* Functions library for generic INI file
*	
* ANSSI/COSSI/DTO/BAI - 2014
***************************************************/

#ifndef __INI_GENERIC_PARSER_H__
#define __INI_GENERIC_PARSER_H__

#include "Common.h"

#define INI_COMMENT_SYMBOL ';'
#define INI_ESCAPE_SYMBOL '\\'
#define INI_PROPERTY_SEPARATOR_SYMBOL '='
#define MAX_INI_SECTIONS 1024
#define MAX_INI_PROPERTIES 4096

//******* <STORE DATA FOR INI FILE> ******
// Gather data for section properties 
typedef struct _INI_PROPERTY_DATA
{
	PWCHAR						tName;
	PWCHAR						tValue;
} INI_PROPERTY_DATA, *PINI_PROPERTY_DATA;

// Gather data for a section '[sample]' 
typedef struct _INI_SECTION_DATA
{
	PWCHAR						tSectionName;

	DWORD						iNumberOfProperty;
	PINI_PROPERTY_DATA			pProperties[MAX_INI_PROPERTIES];
} INI_SECTION_DATA, *PINI_SECTION_DATA;

// Store information for a generic ini file
typedef struct _INI_FILE_DATA
{
	PWCHAR						tFilePath;

	DWORD						iNumberOfSection;
	PINI_SECTION_DATA			pSections[MAX_INI_SECTIONS];
} INI_FILE_DATA, *PINI_FILE_DATA;
//****** </STORE DATA FOR INI FILE> ******

// Entry point for generic INI parsing
PINI_FILE_DATA ParseIniFile(_In_ PWCHAR pwFileRawData, _In_ DWORD dwDataSize, _In_ PTCHAR tFilePath);
BOOL FreeIniFileData(_Inout_ PINI_FILE_DATA pIniData);

// Get section from it name
PINI_SECTION_DATA GetSectionByName(_In_ PINI_FILE_DATA pIniData, _In_ PTCHAR tSectionName);
// Get property from it name
PINI_PROPERTY_DATA GetPropertyByName(_In_ PINI_SECTION_DATA pSectionData, _In_ PTCHAR tPropertyName);
// Delete section from an INI file
BOOL RemoveSectionInIniData(_Inout_ PINI_FILE_DATA pIniData, _In_ PINI_SECTION_DATA pSectionToDelete);
// Delete property from an INI file
BOOL RemovePropertyInSection(_Inout_ PINI_SECTION_DATA pSectionData, _In_ PINI_PROPERTY_DATA pPropertyToDelete);
// Release section data
BOOL FreeSectionData(_Inout_ PINI_SECTION_DATA pSectionData);
// Release property data
BOOL FreePropertyData(_Inout_ PINI_PROPERTY_DATA pPropertyData);
// Determine if a section is empty or not
BOOL IsSectionEmpty(_In_ PINI_SECTION_DATA pSectionData);

// Add section structure to generic INI structure
BOOL AddNewSection(_In_ PINI_FILE_DATA pInfData, _In_ PWCHAR pSectionName, _In_ PINI_SECTION_DATA *pOutNewSection);
// Add a new property to a section
BOOL AddNewProperty(_Inout_ PINI_SECTION_DATA pSectionData, _In_ PWCHAR tRawValue);

// Oracle guessing if a file is encoded in WCHAR OR ANSI
BOOL IsIniFileWcharEncoded(_In_ PBYTE pbINIRawDATA, _In_ DWORD dwINIRawDATALen);

// Guess if a line contain a new section to parse
BOOL IsIniLineContainASection(_In_ PWCHAR tLine, _In_ PWCHAR *pSectionName);

//Check if line is a comment
BOOL IsLineCommentInINI(_In_ PWCHAR tLine);
#endif