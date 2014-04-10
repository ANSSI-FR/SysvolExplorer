/**************************************************
* SysvolCrawler - IEAKPrinter.c
* AUTHOR: Luc Delsalle	
*
* Display or export content of Internet Explorer file
* (store in IEAK folder)
*	
* ANSSI/COSSI/DTO/BAI - 2014
***************************************************/

#ifndef __IEAKPRINTER_H__
#define __IEAKPRINTER_H__

#include "Common.h"
#include "PrinterCommon.h"
#include "IEAKParser.h"
#include "INIGenericPrinter.h"

//************** <PRINTER DEFINITION> *********************
#define OUTPUT_NAME_IEAK_FOLDER			TEXT("IEAKFileFolder")
#define OUTPUT_DIRECTORY_IEAK_FOLDER	TEXT("[Machine||User]")
//************** </PRINTER DEFINITION> ********************

// Generic dispatcher for printers
BOOL PrintData(_In_ PIEAK_FILE_DATA pIeakFileData);
BOOL PrintIeakDataHeader(_In_ PTCHAR tFilePath);
BOOL PrintIeakDataFooter(_In_ PTCHAR tFilePath);

// Printers for file format
BOOL PrintXMLData(_In_ PIEAK_FILE_DATA pIeakFileData);
BOOL PrintCSVData(_In_ PIEAK_FILE_DATA pIeakFileData);
BOOL PrintSTDOUTData(_In_ PIEAK_FILE_DATA pIeakFileData);

BOOL PrintXMLRawData(_In_ PIEAK_FILE_DATA pIeakFileData, _In_ HANDLE hXMLFile);
BOOL PrintXMLIniData(_In_ PIEAK_FILE_DATA pIeakFileData, _In_ HANDLE hXMLFile);
BOOL PrintCSVRawData(_In_ PIEAK_FILE_DATA pIeakFileData, _In_ HANDLE hCSVFile);
BOOL PrintCSVIniData(_In_ PIEAK_FILE_DATA pIeakFileData, _In_ HANDLE hCSVFile);
BOOL PrintSTDOUTRawData(_In_ PIEAK_FILE_DATA pIeakFileData);
BOOL PrintSTDOUTIniData(_In_ PIEAK_FILE_DATA pIeakFileData);

/*****************************************************************
 * HOW TO add new IEAK file printer
 * 1 - Add switch case in  PrintXMLData, PrintCSVData and
 *      PrintSTDOUTData method
 *
 * 2 - Implement new printing functions: PrintXMLXXXData,
 *     PrintCSVXXXData and PrintSTDOUTXXXData
 *****************************************************************/
#endif