/**************************************************
* SysvolCrawler - INFPrinter.h
* AUTHOR: Luc Delsalle
*
* Display or export ADM data
*
* ANSSI/COSSI/DTO/BAI - 2014
***************************************************/

#ifndef __ADM_PRINTER_H__
#define __ADM_PRINTER_H__

#include "ADMParser.h"
#include "PrinterCommon.h"

//************** <PRINTER DEFINITION> *********************
#define OUTPUT_NAME_ADM_FILE			TEXT("AdmFiles")
#define OUTPUT_DIRECTORY_ADM_FILE		TEXT("Adm")
//************** </PRINTER DEFINITION> ********************

// Generic dispatcher for printers
BOOL PrintData(_In_ PADM_FILE_DATA pAdmData);
BOOL PrintMiscDataHeader(_In_ PTCHAR tFilePath);
BOOL PrintMiscDataFooter(_In_ PTCHAR tFilePath);

// Printers for file format
BOOL PrintXMLData(_In_ PADM_FILE_DATA pAdmData);
BOOL PrintCSVData(_In_ PADM_FILE_DATA pAdmData);
BOOL PrintSTDOUTData(_In_ PADM_FILE_DATA pAdmData);

#endif