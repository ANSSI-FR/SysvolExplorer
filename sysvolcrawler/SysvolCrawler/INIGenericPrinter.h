/**************************************************
* SysvolCrawler - INIGenericPrinter.h
* AUTHOR: Luc Delsalle	
*
* Generic INI file printer
*	
* ANSSI/COSSI/DTO/BAI - 2014
***************************************************/

#ifndef __INIGENERICPPRINTER_H__
#define __INIGENERICPPRINTER_H__

#include "Common.h"
#include "PrinterCommon.h"
#include "INIGenericParser.h"

// Generic dispatcher for printers
BOOL PrintData(_In_ PINI_FILE_DATA pIniFileData, _In_ HANDLE hXMLFile, _In_ HANDLE hCSVFile);
BOOL PrintIniDataHeader(_In_ PTCHAR tFilePath, _In_ HANDLE hXMLFile, _In_ HANDLE hCSVFile);
BOOL PrintIniDataFooter(_In_ PTCHAR tFilePath);

// Printers for file format
BOOL PrintXMLData(_In_ PINI_FILE_DATA pIniFileData, _In_ HANDLE hXMLFile);
BOOL PrintCSVData(_In_ PINI_FILE_DATA pIniFileData, _In_ HANDLE hCSVFile);
BOOL PrintSTDOUTData(_In_ PINI_FILE_DATA pIniFileData);

BOOL PrintXMLSectionData(_In_ PINI_SECTION_DATA pSectionData, _In_ HANDLE hXMLFile);
BOOL PrintCSVSectionData(_In_ PINI_FILE_DATA pIniData, _In_ PINI_SECTION_DATA pSectionData, _In_ HANDLE hCSVFile);
BOOL PrintSTDOUTSectionData(_In_ PINI_FILE_DATA pInfData, _In_ PINI_SECTION_DATA pSectionData);

#endif