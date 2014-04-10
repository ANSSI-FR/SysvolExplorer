/**************************************************
*				SysvolCrawler 
*
* 
* Description:
* This projet is a fast, complete and reliable Active 
* Directory SYSVOL crawler.
* It allows ITOPs or Security Auditors to inspect
* GPO parameters at a domain scale in order to 
* evaluate GPO accurency.
*
*
* ANSSI/COSSI/DTO/BAI - 2014
***************************************************/

#ifndef __SYSVOLCRAWLER_H__
#define __SYSVOLCRAWLER_H__

#include "Common.h"
#include "Dispatcher.h"
#include "PrinterCommon.h"
#include "LDAPPrinter.h"

INT main(_In_ INT argc, _In_ PCHAR argv[]);
VOID ParseCmdLineOption(_In_ INT argc, _In_ PCHAR *argv);
BOOL LaunchSysvolCrawling(_In_ PCHAR pSysvolPath);
BOOL LaunchLDAPCrawling();

INT GetOpt(_In_ INT argc, _In_ PCHAR *argv, _In_ PCHAR optstring, _Out_ PCHAR *outOptArg, _Out_ PINT pOptInd);
VOID SysCrwlrUsage(_In_ PCHAR pSyscrwlrName, _In_ BOOL bSouldPrintInfo);
VOID DefineOutputFormat(_In_ PCHAR pSelectedOutputFormat);

#endif