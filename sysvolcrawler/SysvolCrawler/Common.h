/**************************************************
* SysvolCrawler - Common.h
* AUTHOR: Luc Delsalle	
*
* Common file for projet
*	
* ANSSI/COSSI/DTO/BAI - 2014
***************************************************/

#ifndef __COMMON_H__
#define __COMMON_H__

#include <Windows.h>
#include <tchar.h>
#include <Strsafe.h>

// Define crawler version
#define CRAWLER_VERSION							TEXT("0.5e")

// DEBUG Mode
#define DEBUG_TO_STDOUT							true
#define DEFAULT_DEBUG_LEVEL						5
#define DEFAULT_LOGFILE							TEXT("SysvolCrawler.log")

// Define output folername
#define OUTPUT_FOLDER_NAME						TEXT("SysCrwlrResults")

// LDAP default port
#define DEFAULT_LDAP_PORT						389

// Define logs options
#define D_NOLOG									0
#define D_ERROR									1
#define D_SECURITY_WARNING						2
#define D_WARNING								4
#define D_INFO									5
#define D_MISC									6
#define MAX_LINE								8192

// Allow parser to know if it is currently processing computer or user settings
typedef DWORD									GPO_FILTER_TARGET;
#define GPO_FILTER_UNKNOWN						0
#define GPO_FILTER_TARGET_MACHINE				1
#define GPO_FILTER_TARGET_USER					2

// Define maximum file size to be store by MISC parser
#define MISC_MAX_FILE_SIZE						0x02000000
#define MISC_MAX_FILE_ERR_MSG					TEXT("File too big to be collected.")

// Store launch parameters
typedef struct _SYSCRWLR_OPTIONS
{
	DWORD		dwDebugLevel;
	BOOL		bShouldDumpLDAP;
	BOOL		bShouldDumpSYSVOL;
	BOOL		bShouldPrintCSV;
	BOOL		bShouldPrintXML;
	BOOL		bShouldPrintSTDOUT;
	PTCHAR		tADLogin;
	PTCHAR		tADPassword;
	PTCHAR		tSysvolFolderPath;
	PTCHAR		tOutputFolderPath;
	PTCHAR		tLogFilePath;
	PTCHAR		tLDAPServer;
	DWORD		dwLDAPPort;
	PTCHAR		tDNSName;
} SYSCRWLR_OPTIONS, *PSYSCRWLR_OPTIONS;

// Store parser metadata
typedef struct _PARSER_IDENTIFIER
{
	BOOL		(*pParserEntryPoint) (PTCHAR);
	PTCHAR		tFileMatchingRegExp;
	PTCHAR		tFolderMatchingRegExp;
	PTCHAR		tParserName;
} PARSER_IDENTIFIER, *PPARSER_IDENTIFIER;

// Crawler heap
extern HANDLE									hCrawlerHeap;

// Forward declaration for launch options
extern PSYSCRWLR_OPTIONS						pSyscrwlrOptions;

// Debug macro
VOID DebugLog(_In_ CHAR CONST *function, _In_ CHAR CONST *file, _In_ INT line, _In_ DWORD dwDebugLevel, _In_ CONST CHAR *format, ...);
#define DEBUG_LOG(...) DebugLog(__FUNCTION__, __FILE__, __LINE__, __VA_ARGS__)

// Standard function library for SysvolCrawler projet
VOID DoExit(_In_ DWORD statuscode);
HANDLE CreateFile_s(_In_ LPCTSTR lpFileName, _In_ DWORD dwDesiredAccess, _In_ DWORD dwShareMode, _In_opt_ LPSECURITY_ATTRIBUTES lpSecurityAttributes, _In_ DWORD dwCreationDisposition, _In_ DWORD dwFlagsAndAttributes, _In_opt_ HANDLE hTemplateFile);
BOOL SetBackupPrivilege();
PTCHAR CStrToPtchar(_In_ CONST PBYTE cstr, _In_ CONST DWORD dwLength);
PCHAR PtcharToCStr(_In_ const PTCHAR tstr);
BOOL GetLine(_In_ PDWORD pdwIndex, _In_ DWORD dwRawDataMaxSize, _In_ PBYTE *pbRawDATA, _Out_ PWCHAR *tResultLine);
BOOL IsLineEmpty(_In_ PWCHAR tLine);
BOOL TrimWhiteSpace(_In_ PWCHAR *pwStr);
GPO_FILTER_TARGET GetTargetGPO(_In_ PTCHAR tFilePath);
PTCHAR rstrstr(_In_ PTCHAR str, _In_ PTCHAR pattern);
BOOL CreateFolderRecursively(_In_ PTCHAR tFolderToCreateOnFS);

#endif