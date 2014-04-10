/**************************************************
* SysvolCrawler - ADMFILESiniPrinter.h
* AUTHOR: Luc Delsalle	
*
* Display or export administrative template data
*	
* ANSSI/COSSI/DTO/BAI - 2014
***************************************************/

#ifndef __ADMFILESINIPPRINTER_H__
#define __ADMFILESINIPPRINTER_H__

#include "Common.h"
#include "PrinterCommon.h"
#include "ADMFILESiniParser.h"

//************** <PRINTER DEFINITION> *********************
#define OUTPUT_NAME_ADMFILES_INI			TEXT("ADMFILESiniFiles")
#define OUTPUT_DIRECTORY_ADMFILES_INI		TEXT("Adm")
//************** </PRINTER DEFINITION> ********************

// Generic dispatcher for printers
BOOL PrintData(_In_ PADMFILESINI_FILE_DATA pAdmFilesIniData);
BOOL PrintAdmFilesIniDataHeader(_In_ PTCHAR tFilePath);
BOOL PrintAdmFilesIniDataFooter(_In_ PTCHAR tFilePath);

// Printers for file format
BOOL PrintXMLData(_In_ PADMFILESINI_FILE_DATA pAdmFilesIniData);
BOOL PrintXMLUnreferencedSectionDataInAdmFiles(_In_ PINI_SECTION_DATA pSectionData, _In_ HANDLE hXMLFile);
BOOL PrintCSVData(_In_ PADMFILESINI_FILE_DATA pAdmFilesIniData);
BOOL PrintSTDOUTData(_In_ PADMFILESINI_FILE_DATA pAdmFilesIniData);

#endif