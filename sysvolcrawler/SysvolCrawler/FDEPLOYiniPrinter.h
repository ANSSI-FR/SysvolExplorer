/**************************************************
* SysvolCrawler - FDEPLOYiniPrinter.h
* AUTHOR: Luc Delsalle	
* 
* Display or store data for folder deployment file
*	
* ANSSI/COSSI/DTO/BAI - 2014
***************************************************/

#ifndef __FDEPLOYINIPPRINTER_H__
#define __FDEPLOYINIPPRINTER_H__

#include "Common.h"
#include "PrinterCommon.h"
#include "FDEPLOYiniParser.h"

//************** <PRINTER DEFINITION> *********************
#define OUTPUT_NAME_FDEPLOY_INI				TEXT("FDEPLOYiniFiles")
#define OUTPUT_DIRECTORY_FDEPLOY_INI		TEXT("User\\Documents & Settings")
//************** </PRINTER DEFINITION> ********************

// Generic dispatcher for printers
BOOL PrintData(_In_ PFDEPLOYINI_FILE_DATA pFdeployIniFileData);
BOOL PrintFdeployIniDataHeader(_In_ PTCHAR tFilePath);
BOOL PrintFdeployIniDataFooter(_In_ PTCHAR tFilePath);

// Printers for file format
BOOL PrintXMLData(_In_ PFDEPLOYINI_FILE_DATA pFdeployIniFileData);
BOOL PrintCSVData(_In_ PFDEPLOYINI_FILE_DATA pFdeployIniFileData);
BOOL PrintSTDOUTData(_In_ PFDEPLOYINI_FILE_DATA pFdeployIniFileData);

BOOL PrintXMLStatusData(_In_ PFDEPLOYINI_FILE_DATA pFdeployIniFileData, _In_ HANDLE hXMLFile);
BOOL PrintXMLRedirectionData(_In_ PFDEPLOYINI_FILE_DATA pFdeployIniFileData, _In_ HANDLE hXMLFile, _In_ FDEPLOY_REDIRECTION_ID dwRedirectionID);
BOOL PrintXMLFdeployUnreferencedSectionData(_In_ PINI_SECTION_DATA pSectionData, _In_ HANDLE hXMLFile);
BOOL PrintCSVStatusData(_In_ PFDEPLOYINI_FILE_DATA pFdeployIniFileData, _In_ HANDLE hCSVFile);
BOOL PrintCSVRedirectionData(_In_ PFDEPLOYINI_FILE_DATA pFdeployIniFileData, _In_ HANDLE hCSVFile, _In_ FDEPLOY_REDIRECTION_ID dwRedirectionID);
BOOL PrintSTDOUTStatusData(_In_ PFDEPLOYINI_FILE_DATA pFdeployIniFileData);
BOOL PrintSTDOUTRedirectionData(_In_ PFDEPLOYINI_FILE_DATA pFdeployIniFileData, _In_ FDEPLOY_REDIRECTION_ID dwRedirectionID);

#endif