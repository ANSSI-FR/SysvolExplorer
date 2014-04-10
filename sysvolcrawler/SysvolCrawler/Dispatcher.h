/**************************************************
* SysvolCrawler - Dispatcher.h
* AUTHOR: Luc Delsalle	
*
* Crawl the SYSVOL and dispatch content to the correct
* parser
*	
* ANSSI/COSSI/DTO/BAI - 2014
***************************************************/
#ifndef __DISPATCHER_H__
#define __DISPATCHER_H__

#include "Common.h"
#include <strsafe.h>

//Parser definition
#include "POLParser.h"
#include "INFParser.h"
#include "GPTiniParser.h"
#include "AASParser.h"
#include "SCRIPTSiniParser.h"
#include "GPEiniParser.h"
#include "IEAKParser.h"
#include "PREFERENCESParser.h"
#include "ADMFILESiniParser.h"
#include "FDEPLOYiniParser.h"
#include "DACLParser.h"
#include "DENIEDParser.h"
#include "MISCParser.h"
#include "ADMParser.h"

#define MAX_PARSER			128
#define DACL_PARSER_ID		125
#define DENIED_PARSER_ID	126
#define MISC_PARSER_ID		127

// Loader SysvolCrawler parsers
BOOL InitDispatcher();
BOOL FreeDispatcher();
// Browse SYSVOL and send file to dispatcher
BOOL BrowseAndDispatch(_In_ TCHAR *tCurrentPath, _In_ DWORD depth);
// Dispatch file to the right parser
BOOL DispatchFile(_In_ PTCHAR tFileName, _In_ PTCHAR tFilePath);

// Simple regexp engine for file name
BOOL wildcmp(_In_ TCHAR* wild, _In_ TCHAR* string);

#endif