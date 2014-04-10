/**************************************************
* SysvolCrawler - PrinterCommon.h
* AUTHOR: Luc Delsalle	
*
* Common functions for printers
*	
* ANSSI/COSSI/DTO/BAI - 2014
***************************************************/

#ifndef __PRINTERCOMMON_H__
#define __PRINTERCOMMON_H__

#include "Common.h"
#include <Shlobj.h>

// Define the filename of printer output
typedef PTCHAR							OUTPUT_FILE_NAME;

// Define output folder
typedef PTCHAR							OUTPUT_DIRECTORY_NAME;

// Define printer file format
typedef DWORD							OUTPUT_FILE_TYPE;
#define OUTPUT_FILE_XML					0
#define OUTPUT_FILE_CSV					1
#define OUTPUT_FILE_STDOUT				2

// XML caracters to escape 
#define XML_TOESCAPE_CHAR_NUMB			5
#define XML_TOESCAPE_CHAR				{TEXT('"'),TEXT('\''), TEXT('<'), TEXT('>'), TEXT('&')}
#define XML_ESCAPED_CHAR				{TEXT("&quot;"),TEXT("&apos;"), TEXT("&lt;"), TEXT("&gt;"), TEXT("&amp;")}
#define CSV_TOESCAPE_CHAR				TEXT(';')
#define CSV_ESCAPED_CHAR				TEXT("\"\"")

// Open and retrieve handle for printer file
HANDLE GetFileHandle(_In_ OUTPUT_FILE_TYPE dwOutputFileType, _In_ OUTPUT_DIRECTORY_NAME tOutputDirectoryName, _In_ OUTPUT_FILE_NAME tOutputFileName);
PTCHAR GetBase64FromByte(_In_ PBYTE pbData, _In_ DWORD dwDataSize);
PTCHAR EscapeXMLString(_In_ PTCHAR tXmlStringToEscape);
PTCHAR EscapeCSVString(_In_ PTCHAR tCsvStringToEscape);
VOID CloseXMLRootElement(_In_ PTCHAR tPath);

#endif