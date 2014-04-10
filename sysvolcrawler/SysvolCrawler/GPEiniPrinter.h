/**************************************************
* SysvolCrawler - GPEiniPrinter.h
* AUTHOR: Luc Delsalle	
*
* Display or export GPE.ini data
*	
* ANSSI/COSSI/DTO/BAI - 2014
***************************************************/

#ifndef __GPEINIPPRINTER_H__
#define __GPEINIPPRINTER_H__

#include "Common.h"
#include "PrinterCommon.h"
#include "GPEiniParser.h"

//************** <PRINTER DEFINITION> *********************
#define OUTPUT_NAME_GPE_INI				TEXT("GPEiniFiles")
#define OUTPUT_DIRECTORY_GPE_INI		TEXT("Group Policy")
//************** </PRINTER DEFINITION> ********************

// Generic dispatcher for printers
BOOL PrintData(_In_ PGPEINI_FILE_DATA pGpeIniData);
BOOL PrintGpeIniDataHeader(_In_ PTCHAR tFilePath);
BOOL PrintGpeIniDataFooter(_In_ PTCHAR tFilePath);

// Printers for file format
BOOL PrintXMLData(_In_ PGPEINI_FILE_DATA pGpeIniData);
BOOL PrintCSVData(_In_ PGPEINI_FILE_DATA pGpeIniData);
BOOL PrintSTDOUTData(_In_ PGPEINI_FILE_DATA pGpeIniData);

// Handle unreferrenced section
BOOL PrintXMLUnreferencedSectionDataInGPE(_In_ PINI_SECTION_DATA pSectionData, _In_ HANDLE hXMLFile);

#endif