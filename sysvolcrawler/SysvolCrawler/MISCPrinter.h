/**************************************************
* SysvolCrawler - INFPrinter.h
* AUTHOR: Luc Delsalle	
*
* Display or export data for odd file which should
* not be on a Sysvol folder
*	
* ANSSI/COSSI/DTO/BAI - 2014
***************************************************/

#ifndef __MISCPRINTER_H__
#define __MISCPRINTER_H__

#include "MISCParser.h"
#include "PrinterCommon.h"

//************** <PRINTER DEFINITION> *********************
#define OUTPUT_NAME_MISC_FILE			TEXT("MISCellaneousFiles")
#define OUTPUT_DIRECTORY_MISC_FILE		TEXT("Misc")
//************** </PRINTER DEFINITION> ********************

// Generic dispatcher for printers
BOOL PrintData(_In_ PMISC_FILE_DATA pMiscData);
BOOL PrintMiscDataHeader(_In_ PTCHAR tFilePath);
BOOL PrintMiscDataFooter(_In_ PTCHAR tFilePath);

// Printers for file format
BOOL PrintXMLData(_In_ PMISC_FILE_DATA pMiscData);
BOOL PrintCSVData(_In_ PMISC_FILE_DATA pMiscData);
BOOL PrintSTDOUTData(_In_ PMISC_FILE_DATA pMiscData);

#endif