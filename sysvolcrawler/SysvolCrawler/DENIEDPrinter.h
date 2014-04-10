/**************************************************
* SysvolCrawler - INFPrinter.h
* AUTHOR: Luc Delsalle	
*
* Display data for file which couldn't be opened
* during CreateFile attempt
*	
* ANSSI/COSSI/DTO/BAI - 2014
***************************************************/

#ifndef __DENIEDPRINTER_H__
#define __DENIEDPRINTER_H__

#include "DENIEDParser.h"
#include "PrinterCommon.h"

//************** <PRINTER DEFINITION> *********************
#define OUTPUT_NAME_DENIED_FILE			TEXT("DENIEDFiles")
#define OUTPUT_DIRECTORY_DENIED_FILE	TEXT("Misc")
//************** </PRINTER DEFINITION> ********************

// Generic dispatcher for printers
BOOL PrintData(_In_ PDENIED_FILE_DATA pDeniedData);
BOOL PrintDeniedDataHeader(_In_ PTCHAR tFilePath);
BOOL PrintDeniedDataFooter(_In_ PTCHAR tFilePath);

// Printers for file format
BOOL PrintXMLData(_In_ PDENIED_FILE_DATA pDeniedData);
BOOL PrintCSVData(_In_ PDENIED_FILE_DATA pDeniedData);
BOOL PrintSTDOUTData(_In_ PDENIED_FILE_DATA pDeniedData);

#endif