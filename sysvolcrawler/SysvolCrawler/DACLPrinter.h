/**************************************************
* SysvolCrawler - DACLPrinter.h
* AUTHOR: Luc Delsalle
*
* Display or export DACL data
*
* ANSSI/COSSI/DTO/BAI - 2014
***************************************************/

#ifndef __DACLPRINTER_H__
#define __DACLPRINTER_H__

#include "DACLParser.h"
#include "PrinterCommon.h"

//************** <PRINTER DEFINITION> *********************
#define OUTPUT_NAME_DACL_FILE			TEXT("FilesDACL")
#define OUTPUT_DIRECTORY_DACL_FILE		TEXT("")
//************** </PRINTER DEFINITION> ********************

// Generic dispatcher for printers
BOOL PrintData(_In_ PDACL_FILE_DATA pDaclData);
BOOL PrintDaclDataHeader(_In_ PTCHAR tFilePath);
BOOL PrintDaclDataFooter(_In_ PTCHAR tFilePath);

// Printers for file format
BOOL PrintXMLData(_In_ PDACL_FILE_DATA pDaclData);
BOOL PrintCSVData(_In_ PDACL_FILE_DATA pDaclData);
BOOL PrintSTDOUTData(_In_ PDACL_FILE_DATA pDaclData);

#endif