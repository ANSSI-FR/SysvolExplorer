/**************************************************
* SysvolCrawler - INFPrinter.h
* AUTHOR: Luc Delsalle	
*
* Display or export INF file content
*	
* ANSSI/COSSI/DTO/BAI - 2014
***************************************************/

#ifndef __INFPRINTER_H__
#define __INFPRINTER_H__

#include "INFParser.h"
#include "PrinterCommon.h"

//************** <PRINTER DEFINITION> *********************
#define OUTPUT_NAME_INF_FILE		TEXT("INFormationFile")
#define OUTPUT_DIRECTORY_INF_FILE	TEXT("Machine\\Microsoft\\Windows NT\\SecEdit")
//************** </PRINTER DEFINITION> ********************

// Generic dispatcher for printers
BOOL PrintData(_In_ PINF_FILE_DATA pInfData);
BOOL PrintInfDataHeader(_In_ PTCHAR tFilePath);
BOOL PrintInfDataFooter(_In_ PTCHAR tFilePath);

// Printers for file format
BOOL PrintXMLData(_In_ PINF_FILE_DATA pInfData);
BOOL PrintXMLSectionData(_In_ PINF_SECTION_DATA pSectionData, _In_ HANDLE hXMLFile);
BOOL PrintCSVData(_In_ PINF_FILE_DATA pInfData);
BOOL PrintCSVSectionData(_In_ PINF_FILE_DATA pInfData, _In_ PINF_SECTION_DATA pSectionData, _In_ HANDLE hCSVFile);
BOOL PrintSTDOUTData(_In_ PINF_FILE_DATA pInfData);
BOOL PrintSTDOUTSectionData(_In_ PINF_FILE_DATA pInfData, _In_ PINF_SECTION_DATA pSectionData);

#endif
