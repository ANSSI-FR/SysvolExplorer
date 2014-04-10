/**************************************************
* SysvolCrawler - Common.cpp
* AUTHOR: Luc Delsalle	
*
* Common file for projet
*	
* ANSSI/COSSI/DTO/BAI - 2014
***************************************************/

#include "Common.h"

VOID DebugLog(_In_ CHAR CONST *function, _In_ CHAR CONST *file, _In_ INT line, _In_ DWORD dwDebugLevel, _In_ CONST CHAR *format, ...)
{
	HANDLE hLogFile = INVALID_HANDLE_VALUE;
	CHAR szLine[MAX_LINE];
	SYSTEMTIME sSystemTime;
	LARGE_INTEGER liDistanceToMove;
	CHAR *pszDebugLevel = NULL;
	va_list arg;
	int i;

	if ((!pSyscrwlrOptions) || (dwDebugLevel > pSyscrwlrOptions->dwDebugLevel))
		return;

	GetSystemTime(&sSystemTime);

	va_start(arg, format);

	switch(dwDebugLevel)
	{
	case D_NOLOG:
		pszDebugLevel = "NOLOG";
		break;
	case D_ERROR:
		pszDebugLevel = "ERROR";
		break;
	case D_SECURITY_WARNING:
		pszDebugLevel = "SECURITY WARNING";
		break;
	case D_WARNING:
		pszDebugLevel = "WARNING";
		break;
	case D_INFO:
		pszDebugLevel = "INFO";
		break;
	case D_MISC:
		pszDebugLevel = "MISC";
		break;
	default:
		pszDebugLevel = "UNKNOWN";
		break;
	}

	if (DEBUG_TO_STDOUT == TRUE)
		i = sprintf_s(szLine, MAX_LINE, "[%02d:%02d:%02d][%s] ", sSystemTime.wHour, sSystemTime.wMinute, sSystemTime.wSecond, pszDebugLevel);
	else
		i = sprintf_s(szLine, MAX_LINE, "[%s:%d][%02d:%02d:%02d][%s] ", function, line, sSystemTime.wHour, sSystemTime.wMinute, sSystemTime.wSecond, pszDebugLevel);
	if (i == -1)
	{
		fprintf(stderr, "[DebugLog]\tError pendant l'appel à sprintf_s\r\n");
		DoExit(D_ERROR);
	}
	vsprintf_s(szLine + i, MAX_LINE - i, format, arg);

	if (DEBUG_TO_STDOUT == TRUE)
	{
		fprintf(stderr, "%s", szLine);
	}
	 else
	{
	DWORD dwNumberOfBytesWritten;
		hLogFile = CreateFile_s(pSyscrwlrOptions->tLogFilePath, GENERIC_WRITE, NULL, NULL, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL , NULL);
	if (hLogFile == INVALID_HANDLE_VALUE)
		return;

	ZeroMemory(&liDistanceToMove, sizeof(LARGE_INTEGER));
	if (SetFilePointerEx(hLogFile, liDistanceToMove, NULL, FILE_END) == INVALID_SET_FILE_POINTER)
	{
		return;
	}
	WriteFile(hLogFile, szLine, (DWORD)strlen(szLine), &dwNumberOfBytesWritten, NULL);
	CloseHandle(hLogFile);
   }
	va_end(arg);
}

VOID DoExit(_In_ DWORD statuscode)
{
	if (hCrawlerHeap)
	{
		HeapDestroy(hCrawlerHeap);
	}

	ExitProcess(statuscode);
}

HANDLE CreateFile_s(_In_ LPCTSTR lpFileName, _In_ DWORD dwDesiredAccess, _In_ DWORD dwShareMode, _In_opt_ LPSECURITY_ATTRIBUTES lpSecurityAttributes, _In_ DWORD dwCreationDisposition, _In_ DWORD dwFlagsAndAttributes, _In_opt_ HANDLE hTemplateFile)
{
	HANDLE hResFile = INVALID_HANDLE_VALUE;

	hResFile = CreateFile(lpFileName, dwDesiredAccess, dwShareMode, lpSecurityAttributes, dwCreationDisposition, dwFlagsAndAttributes | FILE_ATTRIBUTE_NORMAL, hTemplateFile);
	if (hResFile == INVALID_HANDLE_VALUE)
	{
		if (SetBackupPrivilege() == FALSE)
		{
			SetLastError(ERROR_ACCESS_DENIED);
			DEBUG_LOG(D_WARNING, "Unable to open file %ws. ErrorCode: %d.\r\n", lpFileName, GetLastError());
			return INVALID_HANDLE_VALUE;
		}

		hResFile = CreateFile(lpFileName, dwDesiredAccess, dwShareMode, lpSecurityAttributes, dwCreationDisposition, FILE_FLAG_BACKUP_SEMANTICS, hTemplateFile);
		if (hResFile == INVALID_HANDLE_VALUE)
		{
			DEBUG_LOG(D_WARNING, "Unable to open file %ws. ErrorCode: %d.\r\n", lpFileName, GetLastError());
			SetLastError(ERROR_ACCESS_DENIED);
			return INVALID_HANDLE_VALUE;
		}
	}
	return hResFile;
}

BOOL SetBackupPrivilege()
{
	HRESULT hr = S_OK;
	TOKEN_PRIVILEGES NewState;
	LUID luid;
	HANDLE hToken = NULL;
	BOOL bRes = TRUE;

	if (!OpenProcessToken(GetCurrentProcess(), TOKEN_ADJUST_PRIVILEGES | TOKEN_QUERY, &hToken))
		return FALSE;

	if (!LookupPrivilegeValue(NULL, SE_BACKUP_NAME, &luid))
	{
		CloseHandle(hToken);
		return FALSE;
	}

	NewState.PrivilegeCount = 1;
	NewState.Privileges[0].Luid = luid;
	NewState.Privileges[0].Attributes = SE_PRIVILEGE_ENABLED;

	if (!AdjustTokenPrivileges(hToken, FALSE, &NewState, 0, NULL, NULL))
		bRes = FALSE;

	CloseHandle(hToken);
	return bRes;
}

PTCHAR CStrToPtchar(_In_ CONST PBYTE cstr, _In_ CONST DWORD dwLength)
{
	PTCHAR tNewStr = NULL;
 
	if ((cstr == NULL) || (dwLength == 0))
	{
		return NULL;
	}

	tNewStr = (PTCHAR) HeapAlloc(hCrawlerHeap, HEAP_ZERO_MEMORY, sizeof (TCHAR) * (dwLength + 1));
	if (!tNewStr)
	{
		DEBUG_LOG(D_ERROR, "Error during string allocation.\r\nExiting now...");
		DoExit(D_ERROR);
	}
 
	if (!MultiByteToWideChar(CP_ACP, MB_PRECOMPOSED, (LPCSTR) cstr, dwLength, tNewStr, dwLength))
	{
		DEBUG_LOG(D_ERROR, "Error during string conversion.\r\nExiting now...");
		DoExit(D_ERROR);
	}
	tNewStr[dwLength] = TEXT('\0');
 
	return (tNewStr);
}

PCHAR PtcharToCStr(_In_ const PTCHAR tstr)
{
	PCHAR pNewStr = NULL;
	DWORD dwBuffLen = 0;

	if (!tstr)
		return NULL;

	dwBuffLen = (DWORD) _tcslen(tstr) + 1;

	pNewStr = (PCHAR) HeapAlloc(hCrawlerHeap, HEAP_ZERO_MEMORY, sizeof (CHAR) * dwBuffLen);	
	if (!pNewStr)
	{
		DEBUG_LOG(D_ERROR, "Error during string allocation\r\n");
		return NULL;
	}

	if (!WideCharToMultiByte(CP_ACP, 0, tstr, dwBuffLen, pNewStr, sizeof (CHAR) * dwBuffLen, NULL, NULL))
	{
		DEBUG_LOG(D_ERROR, "Error during string conversion\r\n");
		return NULL;
	}
	return pNewStr;
}

BOOL GetLine(_In_ PDWORD pdwIndex, _In_ DWORD dwRawDataMaxSize, _In_ PBYTE *pbRawDATA, _Out_ PWCHAR *tResultLine)
{
	DWORD i = 0, dwLength = 0;
	WCHAR cPreviousChar = TEXT('\x00');
	WCHAR cPreviousPreviousChar = TEXT('\x00');

	*tResultLine = NULL;

	for (i = *pdwIndex; i < dwRawDataMaxSize; i += 2)
	{
		WCHAR cCurrentChar = (WCHAR) *((*pbRawDATA) + i);

		//EoL (in case of '\', line should continue)
		if (cCurrentChar == TEXT('\n'))
		{
			if (!((cPreviousChar == TEXT('\r') && (cPreviousPreviousChar == TEXT('\\'))) || (cPreviousChar == TEXT('\\'))))
				break;
			else
			{
				if (cPreviousChar == TEXT('\\'))
				{
					*((*pbRawDATA) + i) = TEXT(' ');
					*((*pbRawDATA) + i - 2) = TEXT(' ');
				}
				else if ((cPreviousChar == TEXT('\r')) && (cPreviousPreviousChar == TEXT('\\'))) 
				{
					*((*pbRawDATA) + i) = TEXT(' ');
					*((*pbRawDATA) + i - 2) = TEXT(' ');
					*((*pbRawDATA) + i - 4) = TEXT(' ');
				}

			}
		}
			
		cPreviousPreviousChar = cPreviousChar;
		cPreviousChar = cCurrentChar;		
	}

	dwLength = i - (*pdwIndex) + 2;
			
	// Remove \n
	if (dwLength >= 2) 
		dwLength -= 2;

	// Remove \r if needed
	if (dwLength >= 2)
	{
		WCHAR cPrevChar = (WCHAR) *((*pbRawDATA) + (i - 2));
		if (cPrevChar == TEXT('\r'))
			dwLength -= 2;
	}

	if (dwLength > 0)
	{
		*tResultLine = (PWCHAR) HeapAlloc(hCrawlerHeap, HEAP_ZERO_MEMORY, (dwLength + sizeof (WCHAR)));
		if (memcpy_s((*tResultLine), dwLength + sizeof (WCHAR), ((*pbRawDATA) + (*pdwIndex)), dwLength))
		{
			DEBUG_LOG(D_ERROR, "Unable to extract line.\r\nExiting now...");
			DoExit(D_ERROR);
		}
	}
	*pdwIndex = i;
	return TRUE;
}

BOOL IsLineEmpty(_In_ PWCHAR tLine)
{
	DWORD i = 0;
	PWCHAR wCurrentChar;
	
	if (!tLine)
		return TRUE;
	wCurrentChar =  tLine;


	while ((wCurrentChar) 
			&& (*wCurrentChar != '\0')
			&& ((*wCurrentChar == ' ') || (*wCurrentChar == '\r') || (*wCurrentChar == '\n')))
		wCurrentChar++;

	if ((wCurrentChar) && (*wCurrentChar != '\0'))
		return FALSE;
	else
		return TRUE;
}

BOOL TrimWhiteSpace(_In_ PWCHAR *pwStr)
{
	PWCHAR pwNewStr = NULL;
	DWORD dwStrBegin = 0, dwStrEnd = 0, dwStrLen  = 0;

	if (!(*pwStr))
		return NULL;

	for (DWORD i = 0; _istspace((*pwStr)[i]); ++i)
		dwStrBegin++;

	if ((*pwStr)[dwStrBegin] == TEXT('\0'))
	{
		return TRUE;
	}

	dwStrEnd = (DWORD)_tcslen((*pwStr));
	for (DWORD i = ((DWORD)_tcslen((*pwStr)) - 1); _istspace((*pwStr)[i]); --i)
		dwStrEnd--;

	dwStrLen = dwStrEnd - dwStrBegin;

	// Return orginal string if no whitespace is found
	if (dwStrLen == _tcslen((*pwStr)))
		return TRUE;

	pwNewStr = (PWCHAR) HeapAlloc(hCrawlerHeap, HEAP_ZERO_MEMORY, (dwStrLen + 1) * sizeof (WCHAR));
	if (!pwNewStr)
	{
		DEBUG_LOG(D_ERROR, "Unable to alloc pwNewStr.\r\nExiting now...");
		DoExit(D_ERROR);
	}

	PWCHAR tst = ((*pwStr) + dwStrBegin);

	if (memcpy_s(pwNewStr, dwStrLen * sizeof (WCHAR), ((*pwStr) + dwStrBegin), sizeof (WCHAR) * dwStrLen))
	{
		DEBUG_LOG(D_ERROR, "Unable to extract property value.\r\nExiting now...");
		DoExit(D_ERROR);
	}

	HeapFree(hCrawlerHeap, NULL, (*pwStr));
	*pwStr = pwNewStr;
	return TRUE;
}

GPO_FILTER_TARGET GetTargetGPO(_In_ PTCHAR tFilePath)
{
	PTCHAR tTmpFilePath = NULL;
	GPO_FILTER_TARGET dwRes = GPO_FILTER_UNKNOWN;
	DWORD i = 0;

	if (!tFilePath)
	{
		DEBUG_LOG(D_ERROR, "Inavlid tFilePath.\r\nExiting now...");
		DoExit(D_ERROR);
	}

	tTmpFilePath = (PTCHAR) HeapAlloc(hCrawlerHeap, NULL, sizeof(TCHAR) * (_tcslen(tFilePath) + 1));
	for (i = 0; i < _tcslen(tFilePath); ++i)
		tTmpFilePath[i] = towlower(tFilePath[i]);
	tTmpFilePath[i] = TEXT('\0');

	if (_tcsstr(tTmpFilePath, TEXT("machine")))
		dwRes = GPO_FILTER_TARGET_MACHINE;
	else if (_tcsstr(tTmpFilePath, TEXT("user")))
		dwRes = GPO_FILTER_TARGET_USER;

	HeapFree(hCrawlerHeap, NULL, tTmpFilePath);
	return dwRes;
}

PTCHAR rstrstr(_In_ PTCHAR str, _In_ PTCHAR pattern)
{
	DWORD dwS1Len = 0, dwpatternLen = 0;
	PTCHAR s = NULL;

  if (!str || !pattern)
	  return NULL;

  dwS1Len = (DWORD) _tcslen(str);
  dwpatternLen = (DWORD) _tcslen(pattern);

  if (dwpatternLen > dwS1Len)
    return NULL;

  for (s = str + dwS1Len - dwpatternLen; s >= str; --s)
    if (_tcsncmp(s, pattern, dwpatternLen) == 0)
      return s;
  
  return NULL;
}

BOOL CreateFolderRecursively(_In_ PTCHAR tFolderToCreateOnFS)
{
	TCHAR tFolder[MAX_PATH];
	PTCHAR ptEnd;
	DWORD dwErr = 0, dwLen = 0;

	ZeroMemory(tFolder, MAX_PATH * sizeof(TCHAR));
	ptEnd = wcschr(tFolderToCreateOnFS, L'\\');

	while(ptEnd != NULL)
	{
		dwLen = (DWORD) (ptEnd - tFolderToCreateOnFS + 1);
		_tcsncpy_s(tFolder, MAX_PATH, tFolderToCreateOnFS, dwLen);

		if ((_tcsclen(tFolder) == 2) && (_tcsncmp(tFolder, TEXT(".\\"), 2) == 0))
			goto continue_loop;

		if(!CreateDirectory(tFolder, NULL))
		{
			dwErr = GetLastError();
			if(dwErr != ERROR_ALREADY_EXISTS)
			{
				DEBUG_LOG(D_ERROR, "Unable to create folder recursively.\r\nExiting now...");
				DoExit(D_ERROR);
			}
		}
continue_loop:
		ptEnd = _tcschr(++ptEnd, L'\\');
	}
	return TRUE;
}
