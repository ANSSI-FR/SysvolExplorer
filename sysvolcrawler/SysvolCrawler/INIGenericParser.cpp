/**************************************************
* SysvolCrawler - INICommon.cpp
* AUTHOR: Luc Delsalle	
*
* Functions library for generic INI file
*	
* ANSSI/COSSI/DTO/BAI - 2014
***************************************************/

#include "INIGenericParser.h"

PINI_FILE_DATA ParseIniFile(_In_ PWCHAR pwFileRawData, _In_ DWORD dwDataSize, _In_ PTCHAR tFilePath)
{
	PINI_FILE_DATA pIniData = NULL;
	PINI_SECTION_DATA pCurrentSection = NULL;

	if (!pwFileRawData || (dwDataSize == 0) || (!tFilePath))
	{
		DEBUG_LOG(D_ERROR, "pwFileRawData or dwDataSize or tFilePath invalid.\r\nExiting now...");
		DoExit(D_ERROR);
	}

	pIniData = (PINI_FILE_DATA) HeapAlloc(hCrawlerHeap, HEAP_ZERO_MEMORY, sizeof (INI_FILE_DATA));
	if (!pIniData)
	{
		DEBUG_LOG(D_ERROR, "Unable to allocate INF_FILE_DATA structure.\r\nExiting now...");
		DoExit(D_ERROR);
	}
	pIniData->iNumberOfSection = 0;
	pIniData->tFilePath = tFilePath;

	for (DWORD i = 0; i < dwDataSize; i += 2)
	{
		PWCHAR tCurrentLine = NULL;
		
		if (GetLine(&i, dwDataSize, (PBYTE *)&pwFileRawData, &tCurrentLine) == FALSE)
			break;

		if (IsLineEmpty(tCurrentLine) == TRUE)
			goto freeAndContinue;

		if (IsLineCommentInINI(tCurrentLine) == TRUE)
			goto freeAndContinue;

		// Guess if line contains a new section or property
		PTCHAR tNewSectionName = NULL;
		if (IsIniLineContainASection(tCurrentLine, &tNewSectionName) == TRUE)
		{
			PINI_SECTION_DATA pNewSection = NULL;

			DEBUG_LOG(D_MISC, "[INI] New section found [%ws]\r\n", tNewSectionName);
			
			if (AddNewSection(pIniData, tNewSectionName, &pNewSection) == FALSE)
			{
				DEBUG_LOG(D_ERROR, "Unable to add new section.\r\nExiting now...");
				DoExit(D_ERROR);
			}
			else
			{
				pCurrentSection = pNewSection;
			}
		}
		else
		{
			if (!pCurrentSection)
			{
				DEBUG_LOG(D_ERROR, "[INI] File invalid, property (%ws) found before any section.\r\n", tCurrentLine);
				return FALSE;
			}
			else
				DEBUG_LOG(D_MISC, "[INI] New property found [%ws] for section %ws\r\n", tCurrentLine, pCurrentSection->tSectionName);
			
			AddNewProperty(pCurrentSection, tCurrentLine);
		}

	freeAndContinue:
		if (tCurrentLine) HeapFree(hCrawlerHeap, NULL, tCurrentLine);
	}

	return pIniData;
}

BOOL FreeIniFileData(_Inout_ PINI_FILE_DATA pIniData)
{
	PINI_SECTION_DATA pCurrentSection = NULL;
	PINI_PROPERTY_DATA pCurrentProperty = NULL;

	if (pIniData == NULL)
	{
		DEBUG_LOG(D_ERROR, "INI_FILE_DATA pointer invalid.\r\nExiting now...");
		DoExit(D_ERROR);
	}

	for (DWORD i = 0; i < pIniData->iNumberOfSection; ++i)
	{
		pCurrentSection = pIniData->pSections[i];
		if (!pCurrentSection)
			continue;

		for (DWORD j = 0; j < pCurrentSection->iNumberOfProperty; ++j)
		{
			pCurrentProperty = pCurrentSection->pProperties[j];
			if (!pCurrentProperty)
				continue;

			if (pCurrentProperty->tName)
			{
				HeapFree(hCrawlerHeap, NULL, pCurrentProperty->tName);
				pCurrentProperty->tName = NULL;
			}
			if (pCurrentProperty->tValue)
			{
				HeapFree(hCrawlerHeap, NULL, pCurrentProperty->tValue);
				pCurrentProperty->tValue = NULL;
			}
			HeapFree(hCrawlerHeap, NULL, pCurrentProperty);
			pCurrentProperty = NULL;
		}

		if (pCurrentSection->tSectionName)
		{
			HeapFree(hCrawlerHeap, NULL, pCurrentSection->tSectionName);
			pCurrentSection->tSectionName = NULL;
		}
		HeapFree(hCrawlerHeap, NULL, pCurrentSection);
		pCurrentSection = NULL;
	}

	if (pIniData)
		HeapFree(hCrawlerHeap, NULL, pIniData);
	pIniData = NULL;
	return TRUE;
}

BOOL AddNewSection(_In_ PINI_FILE_DATA pInfData, _In_ PWCHAR pSectionName, _In_ PINI_SECTION_DATA *pOutNewSection)
{
	*pOutNewSection = NULL;

	// Create and fill section structure
	*pOutNewSection = (PINI_SECTION_DATA) HeapAlloc(hCrawlerHeap, HEAP_ZERO_MEMORY, sizeof (INI_SECTION_DATA));
	if ((*pOutNewSection) == NULL)
	{
		DEBUG_LOG(D_ERROR, "pOutNewSection pointer invalid.\r\nExiting now...");
		DoExit(D_ERROR);
	}
	(*pOutNewSection)->iNumberOfProperty = 0;
	(*pOutNewSection)->tSectionName = pSectionName;

	if ((pInfData->iNumberOfSection + 1) == MAX_INI_SECTIONS)
	{
		DEBUG_LOG(D_ERROR, "[INI] Unable to add new section, reached the maximum number of section for one Ini file\r\n");
		return FALSE;
	}

	pInfData->pSections[pInfData->iNumberOfSection] = (*pOutNewSection);
	pInfData->iNumberOfSection += 1;

	return TRUE;
}

BOOL AddNewProperty(_Inout_ PINI_SECTION_DATA pSectionData, _In_ PWCHAR tRawValue)
{
	DWORD i = 0;
	BOOL bValueFound = FALSE;
	DWORD dwNameBegin = 0, dwNameLen = 0, dwValueBegin = 0, dwValueLen = 0;
	PINI_PROPERTY_DATA pIniPropertyData = NULL;

	if (!pSectionData || ! tRawValue)
	{
		DEBUG_LOG(D_ERROR, "INI_SECTION_DATA or CHAR pointer invalid.\r\nExiting now...");
		DoExit(D_ERROR);
	}

	pIniPropertyData = (PINI_PROPERTY_DATA) HeapAlloc(hCrawlerHeap, HEAP_ZERO_MEMORY, sizeof (INI_PROPERTY_DATA));
	if (!pIniPropertyData)
	{
		DEBUG_LOG(D_ERROR, "Unable to alloc pInfPropertyData.\r\nExiting now...");
		DoExit(D_ERROR);
	}
	pIniPropertyData->tName = NULL;
	pIniPropertyData->tValue = NULL;

	while (tRawValue[i] != TEXT('\0'))
	{
		if ((tRawValue[i] == TEXT(INI_PROPERTY_SEPARATOR_SYMBOL))
			&& (i > 0)
			&& (tRawValue[i - 1] != TEXT(INI_ESCAPE_SYMBOL)))
		{
			bValueFound = TRUE;
			dwNameLen = i;
			dwValueBegin = i + 1;
		}
		++i;
	}

	if (bValueFound == FALSE)
		dwNameLen = i;
	else
		dwValueLen = i - dwValueBegin;

	// Extract property name
	pIniPropertyData->tName = (PTCHAR) HeapAlloc(hCrawlerHeap, HEAP_ZERO_MEMORY, sizeof (WCHAR) * (dwNameLen + 1));
	if (!(pIniPropertyData->tName))
	{
		DEBUG_LOG(D_ERROR, "Unable to alloc tName.\r\nExiting now...");
		DoExit(D_ERROR);
	}
	if (memcpy_s((pIniPropertyData->tName), sizeof (WCHAR) * dwNameLen, (tRawValue + dwNameBegin), sizeof (WCHAR) * dwNameLen))
	{
		DEBUG_LOG(D_ERROR, "Unable to extract property name.\r\nExiting now...");
		DoExit(D_ERROR);
	}
	pIniPropertyData->tName[dwNameLen] = '\0';
	TrimWhiteSpace(&(pIniPropertyData->tName));


	// Extract value if exists
	if (bValueFound == FALSE)
	{
		pIniPropertyData->tValue = (PTCHAR) HeapAlloc(hCrawlerHeap, HEAP_ZERO_MEMORY, sizeof (WCHAR) * 1);
		pIniPropertyData->tValue[0] = TEXT('\0');
		goto AddPropertyTosection;
	}

	pIniPropertyData->tValue = (PTCHAR) HeapAlloc(hCrawlerHeap, HEAP_ZERO_MEMORY, sizeof (WCHAR) * (dwValueLen + 1));
	if (!(pIniPropertyData->tValue))
	{
		DEBUG_LOG(D_ERROR, "Unable to alloc tValue.\r\nExiting now...");
		DoExit(D_ERROR);
	}

	if (dwValueLen > 0)
	{
		if (memcpy_s((pIniPropertyData->tValue), sizeof (WCHAR) * dwValueLen, (tRawValue + dwValueBegin), sizeof (WCHAR) * dwValueLen))
		{
			DEBUG_LOG(D_ERROR, "Unable to extract property value.\r\nExiting now...");
			DoExit(D_ERROR);
		}
	}
	pIniPropertyData->tValue[dwValueLen] = '\0';
	TrimWhiteSpace(&(pIniPropertyData->tValue));

	// Add new property to current section
AddPropertyTosection:
	if (pSectionData->iNumberOfProperty < MAX_INI_PROPERTIES)
	{
		pSectionData->pProperties[pSectionData->iNumberOfProperty] = pIniPropertyData;
		pSectionData->iNumberOfProperty += 1;
	}
	else
	{
		DEBUG_LOG(D_ERROR, "[INI] Unable to add new property, reached the maximum number of section for one Ini file\r\n");
		return FALSE;
	}
	return TRUE;
}

PINI_SECTION_DATA GetSectionByName(_In_ PINI_FILE_DATA pIniData, _In_ PTCHAR tSectionName)
{
	if (!pIniData || !tSectionName)
	{
		DEBUG_LOG(D_ERROR, "INI_FILE_DATA or SectionName pointer invalid.\r\nExiting now...");
		DoExit(D_ERROR);
	}

	for (DWORD i = 0; i < pIniData->iNumberOfSection; ++i)
	{
		PINI_SECTION_DATA pCurrentSection = pIniData->pSections[i];

		if ((pCurrentSection) && (pCurrentSection->tSectionName)
			 && (!wcscmp(pCurrentSection->tSectionName, tSectionName)))
			 return pCurrentSection;
	}
	return NULL;
}

PINI_PROPERTY_DATA GetPropertyByName(_In_ PINI_SECTION_DATA pSectionData, _In_ PTCHAR tPropertyName)
{
	if (!pSectionData || !tPropertyName)
	{
		DEBUG_LOG(D_ERROR, "INI_SECTION_DATA or PropertyName pointer invalid.\r\nExiting now...");
		DoExit(D_ERROR);
	}

	for (DWORD i = 0; i < pSectionData->iNumberOfProperty; ++i)
	{
		PINI_PROPERTY_DATA pCurrentProperty = pSectionData->pProperties[i];

		if ((pCurrentProperty) && (pCurrentProperty->tName)
			 && (!wcscmp(pCurrentProperty->tName, tPropertyName)))
			 return pCurrentProperty;
	}
	return NULL;
}

BOOL RemoveSectionInIniData(_Inout_ PINI_FILE_DATA pIniData, _In_ PINI_SECTION_DATA pSectionToDelete)
{
	DWORD dwIndexOfSectionToDel = 0xdeadb33f;

	if (!pIniData || !pSectionToDelete)
	{
		DEBUG_LOG(D_ERROR, "INI_FILE_DATA or INI_SECTION_DATA pointer invalid.\r\nExiting now...");
		DoExit(D_ERROR);
	}

	for (DWORD i = 0; i < pIniData->iNumberOfSection; ++i)
	{
		PINI_SECTION_DATA pCurrentSection = pIniData->pSections[i];

		if ((pCurrentSection) && (pCurrentSection == pSectionToDelete))
			 dwIndexOfSectionToDel = i;
	}

	if (dwIndexOfSectionToDel == 0xdeadb33f)
		return FALSE;

	for (DWORD i = (dwIndexOfSectionToDel); i < (pIniData->iNumberOfSection - 1); ++i)
	{
		if (!pIniData->pSections[i] || !pIniData->pSections[i + 1])
			return FALSE;
		pIniData->pSections[i] = pIniData->pSections[i + 1];
	}
	if (pIniData->pSections[pIniData->iNumberOfSection - 1])
		pIniData->pSections[pIniData->iNumberOfSection - 1] = NULL;

	pIniData->iNumberOfSection--;
	pSectionToDelete->iNumberOfProperty = 0;

	if (pSectionToDelete->tSectionName != NULL)
		HeapFree(hCrawlerHeap, NULL, pSectionToDelete->tSectionName);
	pSectionToDelete->tSectionName = NULL;
	FreeSectionData(pSectionToDelete);
	return TRUE;
}

BOOL RemovePropertyInSection(_Inout_ PINI_SECTION_DATA pSectionData, _In_ PINI_PROPERTY_DATA pPropertyToDelete)
{
	DWORD dwIndexOfPropertyToDel = 255;

	if (!pSectionData || !pPropertyToDelete)
	{
		DEBUG_LOG(D_ERROR, "INI_SECTION_DATA or INI_PROPERTY_DATA pointer invalid.\r\nExiting now...");
		DoExit(D_ERROR);
	}

	for (DWORD i = 0; i < pSectionData->iNumberOfProperty; ++i)
	{
		PINI_PROPERTY_DATA pCurrentProperty = pSectionData->pProperties[i];

		if ((pCurrentProperty) && (pCurrentProperty == pPropertyToDelete))
			 dwIndexOfPropertyToDel = i;
	}

	if (dwIndexOfPropertyToDel == 255)
		return FALSE;

	for (DWORD i = (dwIndexOfPropertyToDel); i < (pSectionData->iNumberOfProperty - 1); ++i)
	{
		if (!pSectionData->pProperties[i] || !pSectionData->pProperties[i + 1])
			return FALSE;
		pSectionData->pProperties[i] = pSectionData->pProperties[i + 1];
	}

	if (pSectionData->pProperties[pSectionData->iNumberOfProperty - 1])
		pSectionData->pProperties[pSectionData->iNumberOfProperty - 1] = NULL;

	pSectionData->iNumberOfProperty--;
	
	FreePropertyData(pPropertyToDelete);
	return TRUE;
}

BOOL FreeSectionData(_Inout_ PINI_SECTION_DATA pSectionData)
{
	if (!pSectionData)
	{
		DEBUG_LOG(D_ERROR, "INI_SECTION_DATA pointer invalid.\r\nExiting now...");
		DoExit(D_ERROR);
	}

	for (DWORD i = 0; i < pSectionData->iNumberOfProperty; ++i)
	{
		PINI_PROPERTY_DATA pCurrentProperty = pSectionData->pProperties[i];

		if (pCurrentProperty)
		{
			if (FreePropertyData(pCurrentProperty) != TRUE)
			{
				DEBUG_LOG(D_ERROR, "Unable to free property data.\r\nExiting now...");
				DoExit(D_ERROR);
			}
		}
	}

	if (pSectionData->tSectionName)
		HeapFree(hCrawlerHeap, NULL, pSectionData->tSectionName);
	if (pSectionData)
		HeapFree(hCrawlerHeap, NULL, pSectionData);
	pSectionData = NULL;
	return TRUE;
}

BOOL FreePropertyData(_Inout_ PINI_PROPERTY_DATA pPropertyData)
{
	if (!pPropertyData)
	{
		DEBUG_LOG(D_ERROR, "INI_PROPERTY_DATA pointer invalid.\r\nExiting now...");
		DoExit(D_ERROR);
	}

	if (pPropertyData->tName)
	{
		HeapFree(hCrawlerHeap, NULL, pPropertyData->tName);
		pPropertyData->tName = NULL;
	}
	if (pPropertyData->tValue)
	{
		HeapFree(hCrawlerHeap, NULL, pPropertyData->tValue);
		pPropertyData->tValue = NULL;
	}
	
	HeapFree(hCrawlerHeap, NULL, pPropertyData);
	pPropertyData = NULL;
	return TRUE;
}

BOOL IsSectionEmpty(_In_ PINI_SECTION_DATA pSectionData)
{
	if (!pSectionData)
		return FALSE;

	if (pSectionData->iNumberOfProperty == 0)
		return TRUE;
	return FALSE;
}

BOOL IsIniFileWcharEncoded(_In_ PBYTE pbINIRawDATA, _In_ DWORD dwINIRawDATALen)
{
	if (!pbINIRawDATA)
		return FALSE;

	if ((dwINIRawDATALen >= 2) &&(pbINIRawDATA[0] == 0xff) && (pbINIRawDATA[1] == 0xfe))
		return TRUE;

	return FALSE;
}

BOOL IsIniLineContainASection(_In_ PWCHAR tLine, _In_ PWCHAR *pSectionName)
{
	DWORD dwSectionNameStart = 0, dwSectionNameLen = 0;
	BOOL isCharFound = FALSE;

	*pSectionName = NULL;

	for (DWORD i = 0; tLine != '\0'; ++i)
	{
		WCHAR cCurrChar = (WCHAR)*(tLine + i);
		WCHAR cPrevChar = (WCHAR)*(tLine + i - 2);

		if ((isCharFound == FALSE) && (cCurrChar == TEXT(' ')))
			continue;
		else if ((isCharFound == FALSE) && (cCurrChar == TEXT('[')))
		{
			isCharFound = TRUE;
			dwSectionNameStart = i + 1;
			continue;
		}
		else if (isCharFound == FALSE)
			break;

		if ((isCharFound == TRUE) && (cCurrChar == TEXT(']')) && (cPrevChar != TEXT(INI_ESCAPE_SYMBOL)))
		{
			dwSectionNameLen = i - dwSectionNameStart;
			*pSectionName = (PTCHAR)HeapAlloc(hCrawlerHeap, HEAP_ZERO_MEMORY, (dwSectionNameLen + 1) * sizeof(WCHAR));

			if (memcpy_s((*pSectionName), dwSectionNameLen * sizeof(WCHAR), (tLine + dwSectionNameStart), dwSectionNameLen * sizeof(WCHAR)))
			{
				DEBUG_LOG(D_ERROR, "Unable to extract section name.\r\nExiting now...");
				DoExit(D_ERROR);
			}
			(*pSectionName)[dwSectionNameLen] = TEXT('\0');
			return TRUE;
		}
	}
	return FALSE;
}

BOOL IsLineCommentInINI(_In_ PWCHAR tLine)
{
	DWORD i = 0;
	PWCHAR wCurrentChar;

	if (!tLine)
		return FALSE;
	wCurrentChar = tLine;

	while ((wCurrentChar)
		&& (*wCurrentChar != '\0')
		&& (*wCurrentChar != ';')
		&& ((*wCurrentChar == ' ') || (*wCurrentChar == '\r') || (*wCurrentChar == '\n')))
		wCurrentChar++;

	if ((wCurrentChar) && (*wCurrentChar == ';'))
		return TRUE;
	else
		return FALSE;
}