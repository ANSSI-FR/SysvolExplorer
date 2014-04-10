/**************************************************
* SysvolCrawler - FDEPLOYiniParser.h
* AUTHOR: Luc Delsalle	
*
* Parsing engine for folder deployment file
*	
* ANSSI/COSSI/DTO/BAI - 2014
***************************************************/

#ifndef __FDEPLOYINIPARSER_H__
#define __FDEPLOYINIPARSER_H__

#include "Common.h"
#include "INIGenericParser.h"

//************** <PARSER DEFINITION> *********************
#define FDEPLOYINI_PARSER_NAME				TEXT("FDEPLOY.ini parser")
#define FDEPLOYINI_MATCHING_FILE_REGEXP		TEXT("fdeploy.ini")
#define FDEPLOYINI_MATCHING_FOLDER_REGEXP	TEXT("[NON SUPPORTED]")
//************** </PARSER DEFINITION> ********************

#define FDEPLOY_MAX_REDIRECTION_VALUES		1024
#define FDEPLOY_MAX_STATUS_VALUES			1024
#define FDEPLOY_STATUS_SECTION				TEXT("FolderStatus")
#define FDEPLOY_MYDOCUMENTS_SECTION			TEXT("My Documents")
#define FDEPLOY_MYPICTURES_SECTION			TEXT("My Pictures")
#define FDEPLOY_APPDATA_SECTION				TEXT("Application Data")
#define FDEPLOY_DESKTOP_SECTION				TEXT("Desktop")
#define FDEPLOY_STARTMENU_SECTION			TEXT("Start Menu")
#define FDEPLOY_PROGRAMS_SECTION			TEXT("Programs")
#define FDEPLOY_STARTUP_SECTION				TEXT("Startup")

typedef DWORD								FDEPLOY_REDIRECTION_ID;
#define FDEPLOY_MYDOCUMENTS_REDIRECTION_ID	0x1
#define FDEPLOY_MYPICTURES_REDIRECTION_ID	0x2
#define FDEPLOY_APPDATA_REDIRECTION_ID		0x3
#define FDEPLOY_DESKTOP_REDIRECTION_ID		0x4
#define FDEPLOY_STARTMENU_REDIRECTION_ID	0x5
#define FDEPLOY_PROGRAMS_REDIRECTION_ID		0x6
#define FDEPLOY_STARTUP_REDIRECTION_ID		0x7

//******* <STORE FDEPLOY.INI DATA> ******
typedef struct _FDEPLOYINI_FOLDER_REDIRECTION
{
	PTCHAR									tTargetedSID;
	PTCHAR									tRedirectionPath;
} FDEPLOYINI_FOLDER_REDIRECTION, *PFDEPLOYINI_FOLDER_REDIRECTION;

typedef struct _FDEPLOYINI_FOLDER_STATUS
{
	PTCHAR									tTargetedFolder;
	DWORD									dwStatus;
} FDEPLOYINI_FOLDER_STATUS, *PFDEPLOYINI_FOLDER_STATUS;

typedef struct _FDEPLOYINI_FILE_DATA
{
	PWCHAR									tFilePath;

	DWORD									dwFolderStatusNum;
	PFDEPLOYINI_FOLDER_STATUS				pFolderStatus[FDEPLOY_MAX_STATUS_VALUES];

	DWORD									dwMyDocumentsRedirectionNum;
	PFDEPLOYINI_FOLDER_REDIRECTION			pMyDocumentsRedirection[FDEPLOY_MAX_REDIRECTION_VALUES];

	DWORD									dwMyPicturesRedirectionNum;
	PFDEPLOYINI_FOLDER_REDIRECTION			pMyPicturesRedirection[FDEPLOY_MAX_REDIRECTION_VALUES];

	DWORD									dwAppDataRedirectionNum;
	PFDEPLOYINI_FOLDER_REDIRECTION			pAppdataRedirection[FDEPLOY_MAX_REDIRECTION_VALUES];

	DWORD									dwDesktopRedirectionNum;
	PFDEPLOYINI_FOLDER_REDIRECTION			pDesktopRedirection[FDEPLOY_MAX_REDIRECTION_VALUES];

	DWORD									dwStartMenuRedirectionNum;
	PFDEPLOYINI_FOLDER_REDIRECTION			pStartMenuRedirection[FDEPLOY_MAX_REDIRECTION_VALUES];

	DWORD									dwProgramsRedirectionNum;
	PFDEPLOYINI_FOLDER_REDIRECTION			pProgramsRedirection[FDEPLOY_MAX_REDIRECTION_VALUES];

	DWORD									dwStartupRedirectionNum;
	PFDEPLOYINI_FOLDER_REDIRECTION			pStartupRedirection[FDEPLOY_MAX_REDIRECTION_VALUES];

	DWORD									dwNumberOfUnReferrencedSections;
	PINI_SECTION_DATA						pUnReferrencedSections[FDEPLOY_MAX_REDIRECTION_VALUES];
} FDEPLOYINI_FILE_DATA, *PFDEPLOYINI_FILE_DATA;
//****** </STORE FDEPLOY.INI DATA> ******

// Forward declaration for printers
extern BOOL PrintData(_In_ PFDEPLOYINI_FILE_DATA pFdeployIniData);
extern BOOL PrintFdeployIniDataHeader(_In_ PTCHAR tFilePath);
extern BOOL PrintFdeployIniDataFooter(_In_ PTCHAR tFilePath);

// Parser registration
VOID RegisterFdeployIniParser(_Inout_ PPARSER_IDENTIFIER *pParserID);
// Entry point for GPE.ini
BOOL ParseFdeployIniFile(_In_ PTCHAR tFilePath);
BOOL FreeFdeployIniFileData(_Inout_ PFDEPLOYINI_FILE_DATA pFdeployIniData);

BOOL FillFdeployIniMethods(_Inout_ PFDEPLOYINI_FILE_DATA pFdeployIniFileData, _In_ PINI_FILE_DATA pGenericIniFileData);
BOOL FillFolderStatusSection(_Inout_ PFDEPLOYINI_FILE_DATA pFdeployIniFileData, _In_ PINI_SECTION_DATA pGenericIniSection, _In_ DWORD dwSectionNumb);
BOOL FillFolderRedirectionSection(_Inout_ PFDEPLOYINI_FILE_DATA pFdeployIniFileData, _In_ PINI_SECTION_DATA pGenericIniSection, _In_ DWORD dwSectionNumb, _In_ FDEPLOY_REDIRECTION_ID dwRedirectionID);
#endif