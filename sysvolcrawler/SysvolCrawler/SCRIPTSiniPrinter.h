/**************************************************
* SysvolCrawler - SCRIPTSiniPrinter.h
* AUTHOR: Luc Delsalle	
*
* Display or export scripts.ini file content
*	
* ANSSI/COSSI/DTO/BAI - 2014
***************************************************/

#ifndef __SCRIPTSINIPPRINTER_H__
#define __SCRIPTSINIPPRINTER_H__

#include "Common.h"
#include "PrinterCommon.h"
#include "SCRIPTSiniParser.h"

//************** <PRINTER DEFINITION> *********************
#define OUTPUT_NAME_SCRIPTS_INI				TEXT("SCRIPTSiniFiles")
#define OUTPUT_DIRECTORY_SCRIPTS_INI		TEXT("[Machine||User]")
//************** </PRINTER DEFINITION> ********************

// Generic dispatcher for printers
BOOL PrintData(_In_ PSCRIPTSINI_FILE_DATA pScriptsIniData);
BOOL PrintScriptsIniDataHeader(_In_ PTCHAR tFilePath);
BOOL PrintScriptsIniDataFooter(_In_ PTCHAR tFilePath);

// Printers for file format
BOOL PrintXMLData(_In_ PSCRIPTSINI_FILE_DATA pScriptsIniData);
BOOL PrintXMLUnreferencedSectionData(_In_ PINI_SECTION_DATA pSectionData, _In_ HANDLE hXMLFile);
BOOL PrintCSVData(_In_ PSCRIPTSINI_FILE_DATA pScriptsIniData);
BOOL PrintSTDOUTData(_In_ PSCRIPTSINI_FILE_DATA pScriptsIniData);

#endif