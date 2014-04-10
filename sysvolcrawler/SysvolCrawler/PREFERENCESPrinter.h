/**************************************************
* SysvolCrawler - PREFERENCESPrinter.h
* AUTHOR: Luc Delsalle	
* 
* Display or export preferences GPO file data
*
* ANSSI/COSSI/DTO/BAI - 2014
***************************************************/

#ifndef __PREFERENCESPRINTER_H__
#define __PREFERENCESPRINTER_H__

#include "Common.h"
#include "PrinterCommon.h"
#include "PREFERENCESParser.h"
#include "INIGenericPrinter.h"

//************** <PRINTER DEFINITION> *********************
#define OUTPUT_NAME_PREFERENCES_FOLDER		TEXT("PREFERENCESFileFolder")
#define OUTPUT_DIRECTORY_PREFERENCES_FOLDER	TEXT("[Machine||User]")
//************** </PRINTER DEFINITION> ********************

// Generic dispatcher for printers
BOOL PrintData(_In_ PPREFERENCES_FILE_DATA pPreferencesFileData);
BOOL PrintPreferencesDataHeader(_In_ PTCHAR tFilePath);
BOOL PrintPreferencesDataFooter(_In_ PTCHAR tFilePath);

// Printers for file format
BOOL PrintXMLData(_In_ PPREFERENCES_FILE_DATA pPreferencesFileData);
BOOL PrintCSVData(_In_ PPREFERENCES_FILE_DATA pPreferencesFileData);
BOOL PrintSTDOUTData(_In_ PPREFERENCES_FILE_DATA pPreferencesFileData);

BOOL PrintXMLRawData(_In_ PPREFERENCES_FILE_DATA pPreferencesFileData, _In_ HANDLE hXMLFile);
BOOL PrintXMLIniData(_In_ PPREFERENCES_FILE_DATA pPreferencesFileData, _In_ HANDLE hXMLFile);
BOOL PrintCSVRawData(_In_ PPREFERENCES_FILE_DATA pPreferencesFileData, _In_ HANDLE hCSVFile);
BOOL PrintCSVIniData(_In_ PPREFERENCES_FILE_DATA pPreferencesFileData, _In_ HANDLE hCSVFile);
BOOL PrintSTDOUTRawData(_In_ PPREFERENCES_FILE_DATA pPreferencesFileData);
BOOL PrintSTDOUTIniData(_In_ PPREFERENCES_FILE_DATA pPreferencesFileData);

/*****************************************************************
 * HOW TO add new PREFERENCES file printer
 * 1 - Add switch case in  PrintXMLData, PrintCSVData and
 *      PrintSTDOUTData method
 *
 * 2 - Implement new printing functions: PrintXMLXXXData,
 *     PrintCSVXXXData and PrintSTDOUTXXXData
 *****************************************************************/
#endif