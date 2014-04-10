/**************************************************
* SysvolCrawler - GPTiniPrinter.h
* AUTHOR: Luc Delsalle	
*
* Display or export GPT.ini file data
*	
* ANSSI/COSSI/DTO/BAI - 2014
***************************************************/

#ifndef __GPTINIPRINTER_H__
#define __GPTINIPRINTER_H__

#include "Common.h"
#include "PrinterCommon.h"
#include "GPTiniParser.h"

//************** <PRINTER DEFINITION> *********************
#define OUTPUT_NAME_GPT_INI			TEXT("GPTiniFiles")
#define OUTPUT_DIRECTORY_GPT_INI		TEXT("")
//************** </PRINTER DEFINITION> ********************

// Generic dispatcher for printers
BOOL PrintData(_In_ PGPTINI_FILE_DATA pGptIniData);
BOOL PrintGptIniDataHeader(_In_ PTCHAR tFilePath);
BOOL PrintGptIniDataFooter(_In_ PTCHAR tFilePath);

// Printers for file format
BOOL PrintXMLData(_In_ PGPTINI_FILE_DATA pGptIniData);
BOOL PrintXMLUnreferrencedSectionDataInGPT(_In_ PINI_SECTION_DATA pSectionData, _In_ HANDLE hXMLFile);
BOOL PrintCSVData(_In_ PGPTINI_FILE_DATA pGptIniData);
BOOL PrintCSVUnreferrencedSectionDataInGPT(_In_ PINI_SECTION_DATA pSectionData, _In_ HANDLE hCSVFile);
BOOL PrintSTDOUTData(_In_ PGPTINI_FILE_DATA pGptIniData);
BOOL PrintSTDOUTSectionData(_In_ PINI_SECTION_DATA pSectionData);

#endif