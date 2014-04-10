/**************************************************
* SysvolCrawler - AASPrinter.h
* AUTHOR: Luc Delsalle	
*
* Display or export AAS data
*	
* ANSSI/COSSI/DTO/BAI - 2014
***************************************************/

#ifndef __AASPRINTER_H__
#define __AASPRINTER_H__

#include "AASParser.h"
#include "PrinterCommon.h"

//************** <PRINTER DEFINITION> *********************
#define OUTPUT_NAME_AAS_FILE			TEXT("AdvertisementApplicationFile")
#define OUTPUT_DIRECTORY_AAS_FILE		TEXT("[Machine||User]")
//************** </PRINTER DEFINITION> ********************

// Generic dispatcher for printers
BOOL PrintData(_In_ PAAS_FILE_DATA pAasData);
BOOL PrintAasDataHeader(_In_ PTCHAR tFilePath);
BOOL PrintAasDataFooter(_In_ PTCHAR tFilePath);

// Printers for file format
BOOL PrintXMLData(_In_ PAAS_FILE_DATA pAasData);
BOOL PrintCSVData(_In_ PAAS_FILE_DATA pAasData);
BOOL PrintSTDOUTData(_In_ PAAS_FILE_DATA pAasData);

#endif