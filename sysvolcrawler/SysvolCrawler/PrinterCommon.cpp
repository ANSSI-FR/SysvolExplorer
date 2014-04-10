/**************************************************
* SysvolCrawler - PrinterCommon.cpp
* AUTHOR: Luc Delsalle	
*
* Common functions for printers
*	
* ANSSI/COSSI/DTO/BAI - 2014
***************************************************/
#include "PrinterCommon.h"

HANDLE GetFileHandle(_In_ OUTPUT_FILE_TYPE dwOutputFileType, _In_ OUTPUT_DIRECTORY_NAME tOutputDirectoryName, _In_ OUTPUT_FILE_NAME tOutputFileName)
{
	PTCHAR tCurrentDirectory = NULL;
	HANDLE hOutputFile = INVALID_HANDLE_VALUE;
	PTCHAR tOutputFolder = pSyscrwlrOptions->tOutputFolderPath;
	LARGE_INTEGER liDistanceToMove;
	DWORD dwError = 0, dwFileSize = 0, dwRes = 0;

	tCurrentDirectory = (PTCHAR) HeapAlloc(hCrawlerHeap, HEAP_ZERO_MEMORY, sizeof (TCHAR) * MAX_PATH);	
	if (!tCurrentDirectory)
	{
		DEBUG_LOG(D_ERROR, "Error during module current dir allocation\r\nExiting now...");
		DoExit(D_ERROR);
	}

	if ((StringCchCat(tCurrentDirectory, MAX_PATH, tOutputFolder) != S_OK) 
		|| (StringCchCat(tCurrentDirectory, MAX_PATH, TEXT("\\")) != S_OK)
		|| (StringCchCat(tCurrentDirectory, MAX_PATH, OUTPUT_FOLDER_NAME) != S_OK)
		|| (StringCchCat(tCurrentDirectory, MAX_PATH, TEXT("\\")) != S_OK)
		|| (StringCchCat(tCurrentDirectory, MAX_PATH, tOutputDirectoryName) != S_OK)
		|| (StringCchCat(tCurrentDirectory, MAX_PATH, TEXT("\\")) != S_OK))
	{
		DEBUG_LOG(D_WARNING, "Unable to create output path folder.\r\nExiting now...");
		DoExit(D_ERROR);
	}

	dwRes = SHCreateDirectory(NULL, tCurrentDirectory);
	dwError = GetLastError();
	
	// User put relative path, we cannot use SHCreateDirectory
	if ((dwRes != ERROR_SUCCESS) && (dwError == ERROR_BAD_PATHNAME))
	{
		if (CreateFolderRecursively(tCurrentDirectory) != TRUE)
		{
			DEBUG_LOG(D_ERROR, "Unable to manually create directory architecture.\r\nExiting now...");
			DoExit(D_ERROR);
		}
	}
	else if ((dwRes != ERROR_SUCCESS) && (dwError != ERROR_ALREADY_EXISTS))
	{
		DEBUG_LOG(D_ERROR, "Unable to create output directory. Last error: %d.\r\nExiting now...", dwError);
		DoExit(D_ERROR);
	}

	if (StringCchCat(tCurrentDirectory, MAX_PATH, tOutputFileName)  != S_OK)
		goto unableToBuildOutputFilename;

	switch (dwOutputFileType)
	{
	case OUTPUT_FILE_XML:
		if (StringCchCat(tCurrentDirectory, MAX_PATH, TEXT(".xml")) != S_OK)
			goto unableToBuildOutputFilename;
		break;
	case OUTPUT_FILE_CSV:
		if (StringCchCat(tCurrentDirectory, MAX_PATH, TEXT(".csv")) != S_OK)
			goto unableToBuildOutputFilename;
		break;
	default:
		DEBUG_LOG(D_ERROR, "File type unknown.\r\nExiting now...");
		DoExit(D_ERROR);
		break;
	}

	hOutputFile = CreateFile_s(tCurrentDirectory, GENERIC_WRITE, FILE_SHARE_READ, NULL, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
	if (hOutputFile == INVALID_HANDLE_VALUE)
	{
		DEBUG_LOG(D_WARNING, "Unable to create output file for format: %ws.\r\nExiting now...", tOutputFileName);
		DoExit(D_ERROR);
	}

	dwFileSize = GetFileSize(hOutputFile, NULL);
	if (dwFileSize == INVALID_FILE_SIZE)
	{
		DEBUG_LOG(D_ERROR, "Error during reading FileSize for %ws.\r\nExiting now...", tCurrentDirectory);
		DoExit(D_ERROR);
	}
	else if (dwFileSize == 0)
	{
		goto ReturnHandle;
	}

	ZeroMemory(&liDistanceToMove, sizeof(LARGE_INTEGER));
	if (SetFilePointerEx(hOutputFile, liDistanceToMove, NULL, FILE_END) == INVALID_SET_FILE_POINTER)
	{
		dwError = GetLastError();
		DEBUG_LOG(D_WARNING, "Unable to move file pointer to the end of the file for: %ws (err: %d).\r\nExiting now...", tOutputFileName, dwError);
		DoExit(D_ERROR);
	}

ReturnHandle:
	HeapFree(hCrawlerHeap, NULL, tCurrentDirectory);
	return hOutputFile;

unableToBuildOutputFilename:
	DEBUG_LOG(D_ERROR, "Unable to create out filename.\r\nExiting now...");
	DoExit(D_ERROR);
	return NULL;
}

PTCHAR GetBase64FromByte(_In_ PBYTE pbData, _In_ DWORD dwDataSize)
{
	BOOL bRes = FALSE;
	DWORD pcchString = 0;
	PTCHAR tBase64Str = NULL,tBase64StrBkp = NULL;

	if (!pbData)
		return NULL;

	if (dwDataSize == 0)
	{
		tBase64Str = (PTCHAR) HeapAlloc(hCrawlerHeap, HEAP_ZERO_MEMORY, sizeof(TCHAR)); 
		tBase64Str[0] = '\0';
		return tBase64Str;
	}

	CryptBinaryToString(pbData, dwDataSize, CRYPT_STRING_BASE64 | CRYPT_STRING_NOCRLF , NULL, &pcchString);
	if (!pcchString)
	{
		DEBUG_LOG(D_ERROR, "Unable to determine base64String length (ErrCode=%d).\r\n.", GetLastError());
		DoExit(D_ERROR);
	}
	else
	{
		tBase64Str = (PTCHAR) HeapAlloc(hCrawlerHeap, HEAP_ZERO_MEMORY, sizeof(TCHAR) * pcchString); 
		if (!tBase64Str)
		{
			DEBUG_LOG(D_ERROR, "Unable to allocate memory (ErrCode=%d).\r\n.", GetLastError());
			DoExit(D_ERROR);
		}
		tBase64StrBkp = tBase64Str;
	}

	if (CryptBinaryToString(pbData, dwDataSize, CRYPT_STRING_BASE64 | CRYPT_STRING_NOCRLF , tBase64Str, &pcchString) != TRUE)
	{
		DEBUG_LOG(D_ERROR, "Unable to convert into base64 (ErrCode=%d).\r\n.", GetLastError());
		DoExit(D_ERROR);
	}

	return tBase64Str;
}

PTCHAR EscapeXMLString(_In_ PTCHAR tXmlStringToEscape)
{
	DWORD i = 0, dwNumberOfCharToEscape = 0, dwNewStringIndex = 0, dwXmlStringToEscapeLen = 0;
	PTCHAR tNewXmlStringEscaped = NULL;
	PTCHAR tEscapedCharList[XML_TOESCAPE_CHAR_NUMB] = XML_ESCAPED_CHAR;

	if (!tXmlStringToEscape)
	{
		DEBUG_LOG(D_MISC, "Unable to escape tXmlStringToEscape: string is null.\r\n");
		return NULL;
	}

	dwXmlStringToEscapeLen = (DWORD) _tcslen(tXmlStringToEscape);

	for (i = 0; i < dwXmlStringToEscapeLen; ++i)
	{
		switch (tXmlStringToEscape[i])
		{
		case TEXT('"'):		// '"'
			dwNumberOfCharToEscape += (DWORD) _tcslen(tEscapedCharList[0]);
			break;
		case TEXT('\''):	// '''
			dwNumberOfCharToEscape += (DWORD) _tcslen(tEscapedCharList[1]);
			break;
		case TEXT('<'):		// '<'
			dwNumberOfCharToEscape += (DWORD) _tcslen(tEscapedCharList[2]);
			break;
		case TEXT('>'):		// '>'
			dwNumberOfCharToEscape += (DWORD) _tcslen(tEscapedCharList[3]);
			break;
		case TEXT('&'):		// '&'
			dwNumberOfCharToEscape += (DWORD) _tcslen(tEscapedCharList[4]);
			break;
		}
	}

	// We dont need to escate any caracter
	if (dwNumberOfCharToEscape == 0)
	{
		tNewXmlStringEscaped = (PTCHAR) HeapAlloc(hCrawlerHeap, NULL, sizeof (TCHAR) * (dwXmlStringToEscapeLen + 1));
		if (!tNewXmlStringEscaped)
		{
			DEBUG_LOG(D_ERROR, "Unable to allocate tNewXmlStringEscaped.\r\nExiting now...");
			DoExit(D_ERROR);
		}

		if (memcpy_s(tNewXmlStringEscaped, sizeof (TCHAR) * (dwXmlStringToEscapeLen + 1), tXmlStringToEscape, sizeof (TCHAR) * dwXmlStringToEscapeLen))
		{
			DEBUG_LOG(D_ERROR, "Unable to extract ID.\r\nExiting now...");
			DoExit(D_ERROR);
		}
		tNewXmlStringEscaped[dwXmlStringToEscapeLen] = TEXT('\0');
		return tNewXmlStringEscaped;
	}

	tNewXmlStringEscaped = (PTCHAR) HeapAlloc(hCrawlerHeap, NULL, sizeof (TCHAR) * (dwXmlStringToEscapeLen + dwNumberOfCharToEscape + 1));
	if (!tNewXmlStringEscaped)
	{
		DEBUG_LOG(D_ERROR, "Unable to allocate tNewXmlStringEscaped.\r\nExiting now...");
		DoExit(D_ERROR);
	}

	// we need to build new string with escaped caracters
	for (i = 0; i < dwXmlStringToEscapeLen; ++i)
	{

		switch (tXmlStringToEscape[i])
		{
		case TEXT('"'):		// '"'
			for (DWORD j = 0; j < _tcslen(tEscapedCharList[0]); ++j)
			{
				tNewXmlStringEscaped[dwNewStringIndex] = tEscapedCharList[0][j];
				++dwNewStringIndex;
			}
			break;
		case TEXT('\''):		// '''
			for (DWORD j = 0; j < _tcslen(tEscapedCharList[1]); ++j)
			{
				tNewXmlStringEscaped[dwNewStringIndex] = tEscapedCharList[1][j];
				++dwNewStringIndex;
			}			
			break;
		case TEXT('<'):		// '<'
			for (DWORD j = 0; j < _tcslen(tEscapedCharList[2]); ++j)
			{
				tNewXmlStringEscaped[dwNewStringIndex] = tEscapedCharList[2][j];
				++dwNewStringIndex;
			}			
			break;
		case TEXT('>'):		// '>'
			for (DWORD j = 0; j < _tcslen(tEscapedCharList[3]); ++j)
			{
				tNewXmlStringEscaped[dwNewStringIndex] = tEscapedCharList[3][j];
				++dwNewStringIndex;
			}			
			break;
		case TEXT('&'):		// '&'
			for (DWORD j = 0; j < _tcslen(tEscapedCharList[4]); ++j)
			{
				tNewXmlStringEscaped[dwNewStringIndex] = tEscapedCharList[4][j];
				++dwNewStringIndex;
			}			
			break;
		default:
			tNewXmlStringEscaped[dwNewStringIndex] = tXmlStringToEscape[i];
			++dwNewStringIndex;
			break;
		}
	}
	tNewXmlStringEscaped[dwNewStringIndex] = TEXT('\0');

	return tNewXmlStringEscaped;
}

PTCHAR EscapeCSVString(_In_ PTCHAR tCsvStringToEscape)
{
	DWORD i = 0, dwNumberOfCharToEscape = 0, dwNewStringIndex = 0, dwCsvStringToEscapeLen = 0;
	PTCHAR tNewCSVStringEscaped = NULL;

	if (!tCsvStringToEscape)
	{
		DEBUG_LOG(D_MISC, "Unable to escape tCsvStringToEscape: string is null.\r\n");
		return NULL;
	}

	dwCsvStringToEscapeLen = (DWORD) _tcslen(tCsvStringToEscape);

	// We dont need to espace any caracters
	if (!_tcsrchr(tCsvStringToEscape, CSV_TOESCAPE_CHAR))
	{
		tNewCSVStringEscaped = (PTCHAR)HeapAlloc(hCrawlerHeap, NULL, sizeof (TCHAR)* (dwCsvStringToEscapeLen + 1));
		if (!tNewCSVStringEscaped)
		{
			DEBUG_LOG(D_ERROR, "Unable to allocate tNewCsvStringEscaped.\r\nExiting now...");
			DoExit(D_ERROR);
		}

		if (memcpy_s(tNewCSVStringEscaped, sizeof (TCHAR)* (dwCsvStringToEscapeLen + 1), tCsvStringToEscape, sizeof (TCHAR)* dwCsvStringToEscapeLen))
		{
			DEBUG_LOG(D_ERROR, "Unable to extract ID.\r\nExiting now...");
			DoExit(D_ERROR);
		}
		tNewCSVStringEscaped[dwCsvStringToEscapeLen] = TEXT('\0');
		return tNewCSVStringEscaped;
	}
	else
	{
		tNewCSVStringEscaped = (PTCHAR)HeapAlloc(hCrawlerHeap, NULL, sizeof (TCHAR)* (dwCsvStringToEscapeLen + _tcslen(CSV_ESCAPED_CHAR) + 1));
		if (!tNewCSVStringEscaped)
		{
			DEBUG_LOG(D_ERROR, "Unable to allocate tNewCsvStringEscaped.\r\nExiting now...");
			DoExit(D_ERROR);
		}
		tNewCSVStringEscaped[0] = TEXT('"');
		for (i = 0; i < dwCsvStringToEscapeLen; ++i)
		{
			tNewCSVStringEscaped[i + 1] = tCsvStringToEscape[i];
		}
		tNewCSVStringEscaped[dwCsvStringToEscapeLen + 1] = TEXT('"');
		tNewCSVStringEscaped[dwCsvStringToEscapeLen + 2] = TEXT('\0');
		return tNewCSVStringEscaped;
	}
}

VOID CloseXMLRootElement(_In_ PTCHAR tPath)
{
	HANDLE hXMLFile = INVALID_HANDLE_VALUE;
	HANDLE hNode = INVALID_HANDLE_VALUE;
	WIN32_FIND_DATA sFindDataMask;
	TCHAR tFindPath[MAX_PATH];
	TCHAR tFullNamePath[MAX_PATH];
	DWORD dwDataRead = 0;
	LARGE_INTEGER liDistanceToMove;

	// Fixing path by adding /* at the end for matching with regexp engine
	if ((StringCchCopy(tFindPath, MAX_PATH, tPath) != S_OK)
		|| (StringCchCat(tFindPath, MAX_PATH, TEXT("\\*")) != S_OK))
	{
		DEBUG_LOG(D_WARNING, "Unable to compute output path folder.\r\nExiting now...");
		DoExit(D_ERROR);
	}

	hNode = FindFirstFile(tFindPath, &sFindDataMask);
	if (hNode == INVALID_HANDLE_VALUE)
	{
		DEBUG_LOG(D_ERROR, "Unable to close XML file %ws.\r\nExiting now...", tFindPath);
			DoExit(1);
	}

	// Following file type we dispacth it
	do
	{
		if (sFindDataMask.cFileName == NULL)
		{
			DEBUG_LOG(D_WARNING, "Folder node with no name found !\r\n");
			continue;
		}
		else if (!_tcscmp(sFindDataMask.cFileName, TEXT(".")) 
			|| !_tcscmp(sFindDataMask.cFileName, TEXT("..")))
			continue;

		// build targeted folder full path
		StringCchCopy(tFullNamePath, MAX_PATH, tPath);
		StringCchCat(tFullNamePath, MAX_PATH, TEXT("\\"));
		StringCchCat(tFullNamePath, MAX_PATH, sFindDataMask.cFileName);

		if (sFindDataMask.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
			CloseXMLRootElement(tFullNamePath);

		// Close XML file
		if (_tcsstr(sFindDataMask.cFileName, TEXT(".xml")))
		{
			hXMLFile = CreateFile_s(tFullNamePath, GENERIC_WRITE, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
			if (hXMLFile == INVALID_HANDLE_VALUE)
			{
				DEBUG_LOG(D_WARNING, "Unable to close xml file for format: %ws (err=%d).\r\nExiting now...", tFullNamePath, GetLastError());
				DoExit(D_ERROR);
			}

			ZeroMemory(&liDistanceToMove, sizeof(LARGE_INTEGER));
			if (SetFilePointerEx(hXMLFile, liDistanceToMove, NULL, FILE_END) == INVALID_SET_FILE_POINTER)
			{
				DEBUG_LOG(D_WARNING, "Unable to move file pointer to the end of the file for: %ws.\r\nExiting now...", tFullNamePath);
				DoExit(D_ERROR);
			}

			if ((WriteFile(hXMLFile, TEXT("</"), (DWORD)(_tcslen(TEXT("</")) * sizeof (TCHAR)), &dwDataRead, NULL) == FALSE)
			|| (WriteFile(hXMLFile, sFindDataMask.cFileName, (DWORD)(_tcslen(sFindDataMask.cFileName) * sizeof (TCHAR)), &dwDataRead, NULL) == FALSE)
			|| (WriteFile(hXMLFile, TEXT(">\r\n"), (DWORD)(_tcslen(TEXT(">\r\n")) * sizeof (TCHAR)), &dwDataRead, NULL) == FALSE))
			{
				DEBUG_LOG(D_WARNING, "Unable add XML root element for file %ws.\r\nExiting now...", tFullNamePath);
				DoExit(D_ERROR);
			}

			CloseHandle(hXMLFile);
		}

	}
	while (FindNextFile(hNode, &sFindDataMask) != 0);
	FindClose(hNode);
}

