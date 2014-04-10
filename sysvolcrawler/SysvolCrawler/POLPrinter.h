/**************************************************
* SysvolCrawler - POLPrinter.h
* AUTHOR: Luc Delsalle	
*
* Display or export POL file content
*
* ANSSI/COSSI/DTO/BAI - 2014
***************************************************/

#ifndef __POLPRINTER_H__
#define __POLPRINTER_H__

#include "PrinterCommon.h"
#include "POLParser.h"

//************** <PRINTER DEFINITION> *********************
#define OUTPUT_NAME_POL_FILE			TEXT("RegistryPolicyFile")
#define OUTPUT_DIRECTORY_POL_FILE		TEXT("[Machine||User]")
//************** </PRINTER DEFINITION> ********************

// Generic dispatcher for printers
BOOL PrintData(_In_ PPOL_DATA pPolData);
BOOL PrintPolDataHeader(_In_ PTCHAR tFilePath);
BOOL PrintPolDataFooter(_In_ PTCHAR tFilePath);

// Printers for file format
BOOL PrintXMLData(_In_ PPOL_DATA pPolData);
BOOL PrintCSVData(_In_ PPOL_DATA pPolData);
BOOL PrintSTDOUTData(_In_ PPOL_DATA pPolData);

PTCHAR GetTypeFromID(_In_ DWORD dwPolType);
BOOL RemoveEndline(_In_ PTCHAR tString);
#endif
