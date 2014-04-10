/**************************************************
* SysvolCrawler - AASParser.c
* AUTHOR: Luc Delsalle	
*
* Parsing engine for .aas file
* (Application Advertise Script)
*	
* ANSSI/COSSI/DTO/BAI - 2014
***************************************************/

#include "AASParser.h"

VOID RegisterAasParser(_Inout_ PPARSER_IDENTIFIER *pParserID)
{
	*pParserID = (PPARSER_IDENTIFIER) HeapAlloc(hCrawlerHeap, HEAP_ZERO_MEMORY, sizeof (PARSER_IDENTIFIER));
	if (!pParserID)
	{
		DEBUG_LOG(D_ERROR, "Unable to allocate PARSER_IDENTIFIER structure.\r\nExiting now...");
		DoExit(D_ERROR);
	}

	(*pParserID)->tParserName = AAS_PARSER_NAME;
	(*pParserID)->tFileMatchingRegExp = AAS_MATCHING_FILE_REGEXP;
	(*pParserID)->tFolderMatchingRegExp = NULL;
	(*pParserID)->pParserEntryPoint = ParseAasFile;
}

BOOL ParseAasFile(_In_ PTCHAR tFilePath)
{
	PAAS_FILE_DATA pAasFileData = NULL;
	HANDLE hAASFile = INVALID_HANDLE_VALUE;
	DWORD dwFileSize = 0, dwNumberOfBytesRead = 0, pdwIndex = 0;
	PBYTE pbAASRawDATA = NULL;

	if (tFilePath == NULL)
	{
		DEBUG_LOG(D_ERROR, "FILEPATH pointer invalid.\r\nExiting now...");
		DoExit(D_ERROR);
	}
	DEBUG_LOG(D_MISC, "[AAS] Now parsing %ws\r\n", tFilePath);

	pAasFileData = (PAAS_FILE_DATA) HeapAlloc(hCrawlerHeap, HEAP_ZERO_MEMORY, sizeof (AAS_FILE_DATA));
	if (!pAasFileData)
	{
		DEBUG_LOG(D_ERROR, "Unable to allocate AAS_FILE_DATA structure.\r\nExiting now...");
		DoExit(D_ERROR);
	}
	pAasFileData->tFilePath = tFilePath;
	pAasFileData->dwNumberOfUnknwownBlock = 0;

	hAASFile = CreateFile_s(tFilePath, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
	if (hAASFile == INVALID_HANDLE_VALUE)
	{
		DEBUG_LOG(D_ERROR, "Unable to open file %ws. ErrorCode: %d.\r\n", tFilePath, GetLastError());
		SetLastError(ERROR_ACCESS_DENIED);
		return FALSE;
	}

	dwFileSize = GetFileSize(hAASFile, NULL);
	if (dwFileSize == INVALID_FILE_SIZE)
	{
		DEBUG_LOG(D_ERROR, "Error during reading FileSize for %ws.\r\nExiting now...", tFilePath);
		DoExit(D_ERROR);
	}

	pbAASRawDATA = (PBYTE) HeapAlloc(hCrawlerHeap, HEAP_ZERO_MEMORY, sizeof (DWORD) * dwFileSize);
	if (pbAASRawDATA == NULL)
	{
		DEBUG_LOG(D_ERROR, "pbAASRawDATA pointer invalid.\r\nExiting now...");
		DoExit(D_ERROR);
	}

	if (!ReadFile(hAASFile, pbAASRawDATA, dwFileSize, &dwNumberOfBytesRead, NULL))
	{
		DEBUG_LOG(D_ERROR, "Unable to read file %ws. ErrorCode: %d.\r\n", tFilePath, GetLastError());
		return FALSE;
	}
	CloseHandle(hAASFile);

	// Fill AAS structure
	if (dwFileSize > 0)
		DispatchAASFile(pAasFileData, pbAASRawDATA, &pdwIndex, dwFileSize);

	PrintAasDataHeader(pAasFileData->tFilePath);
	PrintData(pAasFileData);
	PrintAasDataFooter(pAasFileData->tFilePath);

	// Release data
	FreeAasFileData(pAasFileData);
	HeapFree(hCrawlerHeap, NULL, pbAASRawDATA);
	return TRUE;
}

BOOL FreeAasFileData(_Inout_ PAAS_FILE_DATA pAasData)
{
	BOOL bRes = FALSE;

	if (pAasData == NULL)
	{
		DEBUG_LOG(D_ERROR, "AAS_FILE_DATA pointer invalid.\r\nExiting now...");
		DoExit(D_ERROR);
	}

	if (pAasData->pAasHeader)
		bRes = FreeAasHeaderBlock(pAasData->pAasHeader);
	if ((bRes) && (pAasData->pAasProductInfo))
		bRes = FreeAasProductInfoBlock(pAasData->pAasProductInfo);
	if ((bRes) && (pAasData->pAasSourceListPublish))
		bRes = FreeAasSourceListPublishBlock(pAasData->pAasSourceListPublish);
	if ((bRes) && (pAasData->pAasProductPublish))
		bRes = FreeAasProductPublishBlock(pAasData->pAasProductPublish);
	if ((bRes) && (pAasData->pAasEnd))
		bRes = FreeAasEndBlock(pAasData->pAasEnd);

	for (DWORD i = 0; i < pAasData->dwNumberOfUnknwownBlock; ++i)
	{
		if ((bRes) && (pAasData->sBlockUnkwnown[i]))
			bRes = FreeAasUnknownBlock(pAasData->sBlockUnkwnown[i]);
	}

	HeapFree(hCrawlerHeap, NULL, pAasData);
	pAasData = NULL;
	return bRes;
}

BOOL DispatchAASFile(_Inout_ PAAS_FILE_DATA pAasFileData, _In_ PBYTE pbRawData, _In_ PDWORD pdwIndex, _In_ DWORD dwRawDataSize)
{
	if (!pbRawData || (*pdwIndex >= dwRawDataSize))
	{
		DEBUG_LOG(D_ERROR, "pbAASRawDATA pointer, pdwIndex or dwRawDataSize invalid.\r\nExiting now...");
		DoExit(D_ERROR);
	}

	while ((*pdwIndex) < dwRawDataSize)
	{
		WORD wCurrOpCode = 0;
		WORD wArgNumber = 0;

		wCurrOpCode = (WORD)* (pbRawData + (*pdwIndex));
		wArgNumber = (WORD)* (pbRawData + (*pdwIndex) + 1);

		if ((wCurrOpCode == AAS_OPCODE_HEADER) && (wArgNumber == 9))
			pAasFileData->pAasHeader = FillAasHeader(pbRawData, pdwIndex, dwRawDataSize);
		else if ((wCurrOpCode == AAS_OPCODE_PRODUCTINFO) && (wArgNumber == 16))
			pAasFileData->pAasProductInfo = FillAasProductInfo(pbRawData, pdwIndex, dwRawDataSize);
		else if ((wCurrOpCode == AAS_OPCODE_SRCLISTPUB) && (wArgNumber >= 6) && ((wArgNumber % 3) == 0))
			pAasFileData->pAasSourceListPublish = FillAasSourceListPublish(pbRawData, pdwIndex, dwRawDataSize);
		else if ((wCurrOpCode == AAS_OPCODE_PRODUCTPUB) && (wArgNumber == 1))
			pAasFileData->pAasProductPublish = FillAasProductPublish(pbRawData, pdwIndex, dwRawDataSize);
		else if ((wCurrOpCode == AAS_OPCODE_END) && (wArgNumber == 3))
			pAasFileData->pAasEnd = FillAasEnd(pbRawData, pdwIndex, dwRawDataSize);
		else
		{
			if ((pAasFileData->dwNumberOfUnknwownBlock + 1) == AAS_ARG_NUMBER_MAX)
			{
				DEBUG_LOG(D_ERROR, "SysCrwler reach the maximum unknwown block number.\r\nExiting now...");
				DoExit(D_ERROR);
			}

			DEBUG_LOG(D_MISC, "[AAS] Found an unknwown block with opcode: %d and %d arguments.\r\n", (DWORD) wCurrOpCode, (DWORD) wArgNumber);
			pAasFileData->sBlockUnkwnown[pAasFileData->dwNumberOfUnknwownBlock] = FillAasUnknownBlock(pbRawData, pdwIndex, dwRawDataSize);
			pAasFileData->dwNumberOfUnknwownBlock +=1;
		}
	}
	return TRUE;
}

PAAS_BLOCK_HEADER FillAasHeader(_In_ PBYTE pbRawData, _In_ PDWORD pdwIndex, _In_ DWORD dwRawDataSize)
{
	PAAS_BLOCK_HEADER pAasBlockHeader = NULL;
	DWORD dwNewIndex = 0;
	DWORD dwDatalen = 0;

	if (!pbRawData || (*pdwIndex >= dwRawDataSize))
	{
		DEBUG_LOG(D_ERROR, "pbAASRawDATA pointer, pdwIndex or dwRawDataSize invalid.\r\nExiting now...");
		DoExit(D_ERROR);
	}
	dwNewIndex = *pdwIndex;

	if ((dwRawDataSize - dwNewIndex) < AAS_BLOCK_HEADER_SIZE)
	{
		DEBUG_LOG(D_ERROR, "Size of pbRawData is invalid: less than the minimum length required for AAS Header.\r\nExiting now...");
		DoExit(D_ERROR);
	}

	pAasBlockHeader = (PAAS_BLOCK_HEADER) HeapAlloc(hCrawlerHeap, HEAP_ZERO_MEMORY, sizeof (AAS_BLOCK_HEADER));
	if (!pAasBlockHeader)
	{
		DEBUG_LOG(D_ERROR, "Unable to allocate AAS_FILE_DATA structure.\r\nExiting now...");
		DoExit(D_ERROR);
	}

	// Ensure that current block is a HEADER block
	if (((BYTE)*(pbRawData + dwNewIndex) != AAS_OPCODE_HEADER)
		|| ((BYTE)*(pbRawData + dwNewIndex + 1) != 0x09))
	{
		DEBUG_LOG(D_ERROR, "The current block doesn't seems to be a HEADER block.\r\nExiting now...");
		DoExit(D_ERROR);
	}
	else
		dwNewIndex += sizeof(BYTE) * 2;

	// Skip DATA_TYPE block
	dwDatalen = VerifyDataTypeAndGetDataSize(pbRawData, &dwNewIndex, dwRawDataSize, AAS_DATATYPE_32BITSINT);

	// Fill Signature field
	SetDataAttributes((PVOID*)&(pAasBlockHeader->pdwSignature), AAS_DATATYPE_32BITSINT, pbRawData, &dwNewIndex, dwDatalen, AAS_DATATYPE_32BITSINT);
	if (*(pAasBlockHeader->pdwSignature) != AAS_FILE_SIGNATURE)
	{
		DEBUG_LOG(D_ERROR, "Unable to verify AAS file signature.\r\nExiting now...");
		DoExit(D_ERROR);
	}

	// Fill Version field
	dwDatalen = VerifyDataTypeAndGetDataSize(pbRawData, &dwNewIndex, dwRawDataSize, AAS_DATATYPE_32BITSINT);
	SetDataAttributes((PVOID*)&(pAasBlockHeader->pdwVersion), AAS_DATATYPE_32BITSINT, pbRawData, &dwNewIndex, dwDatalen, AAS_DATATYPE_32BITSINT);

	// Fill TimeStamp field
	dwDatalen = VerifyDataTypeAndGetDataSize(pbRawData, &dwNewIndex, dwRawDataSize, AAS_DATATYPE_32BITSINT);
	SetDataAttributes((PVOID*)&(pAasBlockHeader->pdwDosTimeStamp), AAS_DATATYPE_32BITSINT, pbRawData, &dwNewIndex, dwDatalen, AAS_DATATYPE_32BITSINT);

	// Fill LangId field
	dwDatalen = VerifyDataTypeAndGetDataSize(pbRawData, &dwNewIndex, dwRawDataSize, AAS_DATATYPE_32BITSINT);
	SetDataAttributes((PVOID*)&(pAasBlockHeader->pdwLangID), AAS_DATATYPE_32BITSINT, pbRawData, &dwNewIndex, dwDatalen, AAS_DATATYPE_32BITSINT);

	// Fill PlatForm field
	dwDatalen = VerifyDataTypeAndGetDataSize(pbRawData, &dwNewIndex, dwRawDataSize, AAS_DATATYPE_32BITSINT);
	SetDataAttributes((PVOID*)&(pAasBlockHeader->pdwPlatform), AAS_DATATYPE_32BITSINT, pbRawData, &dwNewIndex, dwDatalen, AAS_DATATYPE_32BITSINT);

	// Fill ScriptType field
	dwDatalen = VerifyDataTypeAndGetDataSize(pbRawData, &dwNewIndex, dwRawDataSize, AAS_DATATYPE_32BITSINT);
	SetDataAttributes((PVOID*)&(pAasBlockHeader->pdwScriptType), AAS_DATATYPE_32BITSINT, pbRawData, &dwNewIndex, dwDatalen, AAS_DATATYPE_32BITSINT);

	// Fill ScriptMajorVersion field
	dwDatalen = VerifyDataTypeAndGetDataSize(pbRawData, &dwNewIndex, dwRawDataSize, AAS_DATATYPE_32BITSINT);
	SetDataAttributes((PVOID*)&(pAasBlockHeader->pdwScriptMajorVersion), AAS_DATATYPE_32BITSINT, pbRawData, &dwNewIndex, dwDatalen, AAS_DATATYPE_32BITSINT);

	// Fill ScriptMinorVersion field
	dwDatalen = VerifyDataTypeAndGetDataSize(pbRawData, &dwNewIndex, dwRawDataSize, AAS_DATATYPE_32BITSINT);
	SetDataAttributes((PVOID*)&(pAasBlockHeader->pdwScriptMinorVersion), AAS_DATATYPE_32BITSINT, pbRawData, &dwNewIndex, dwDatalen, AAS_DATATYPE_32BITSINT);

	// Fill ScriptAttributes field
	dwDatalen = VerifyDataTypeAndGetDataSize(pbRawData, &dwNewIndex, dwRawDataSize, AAS_DATATYPE_32BITSINT);
	SetDataAttributes((PVOID*)&(pAasBlockHeader->pdwScriptAttributes), AAS_DATATYPE_32BITSINT, pbRawData, &dwNewIndex, dwDatalen, AAS_DATATYPE_32BITSINT);

	*pdwIndex = dwNewIndex;
	return pAasBlockHeader;
}

PAAS_BLOCK_PRODUCT_INFO FillAasProductInfo(_In_ PBYTE pbRawData, _In_ PDWORD pdwIndex, _In_ DWORD dwRawDataSize)
{
	PAAS_BLOCK_PRODUCT_INFO pAasBlockProductInfo = NULL;
	DWORD dwNewIndex = 0;
	DWORD dwDatalen = 0;

	if (!pbRawData || (*pdwIndex >= dwRawDataSize))
	{
		DEBUG_LOG(D_ERROR, "pbAASRawDATA pointer, pdwIndex or dwRawDataSize invalid.\r\nExiting now...");
		DoExit(D_ERROR);
	}
	dwNewIndex = *pdwIndex;

	if ((dwRawDataSize - dwNewIndex) < AAS_BLOCK_PRODUCT_INFO_SIZE)
	{
		DEBUG_LOG(D_ERROR, "Size of pbRawData is invalid: less than the minimum length required for AAS Product Info.\r\nExiting now...");
		DoExit(D_ERROR);
	}

	pAasBlockProductInfo = (PAAS_BLOCK_PRODUCT_INFO) HeapAlloc(hCrawlerHeap, HEAP_ZERO_MEMORY, sizeof (AAS_BLOCK_PRODUCT_INFO));
	if (!pAasBlockProductInfo)
	{
		DEBUG_LOG(D_ERROR, "Unable to allocate AAS_BLOCK_PRODUCT_INFO structure.\r\nExiting now...");
		DoExit(D_ERROR);
	}

	// Ensure that current block is a PRODUCT_INFO block
	if (((BYTE)*(pbRawData + dwNewIndex) != AAS_OPCODE_PRODUCTINFO)
		|| ((BYTE)*(pbRawData + dwNewIndex + 1) != 0x10))
	{
		DEBUG_LOG(D_ERROR, "The current block doesn't seems to be a PRODUCTINFO block.\r\nExiting now...");
		DoExit(D_ERROR);
	}
	else
		dwNewIndex += sizeof(BYTE) * 2;

	// Fill ProductKey field
	dwDatalen = VerifyDataTypeAndGetDataSize(pbRawData, &dwNewIndex, dwRawDataSize, AAS_DATATYPE_ASCIICHAR);
	SetDataAttributes((PVOID*)&(pAasBlockProductInfo->pwProductKey), AAS_DATATYPE_ASCIICHAR, pbRawData, &dwNewIndex, dwDatalen, AAS_DATATYPE_ASCIICHAR);

	// Fill ProductName field following expected data
	if (((*((PWORD)(pbRawData + dwNewIndex))) & AAS_DATATYPE_UNICODESTR)
		|| (((*((PWORD)(pbRawData + dwNewIndex))) & AAS_DATATYPE_NULLARG)))
	{
		dwDatalen = VerifyDataTypeAndGetDataSize(pbRawData, &dwNewIndex, dwRawDataSize, AAS_DATATYPE_UNICODESTR);

		if (!dwDatalen)
			SetDataAttributes((PVOID*)&(pAasBlockProductInfo->pwProductName), AAS_DATATYPE_NULLARG, pbRawData, &dwNewIndex, dwDatalen, AAS_DATATYPE_NULLARG);
		else
		{
			SetDataAttributes((PVOID*)&(pAasBlockProductInfo->pwProductName), AAS_DATATYPE_UNICODESTR, pbRawData, &dwNewIndex, dwDatalen, AAS_DATATYPE_UNICODESTR + dwDatalen);
			pAasBlockProductInfo->isProductNameUNICODE = TRUE;
		}

	}
	else
	{
		dwDatalen = VerifyDataTypeAndGetDataSize(pbRawData, &dwNewIndex, dwRawDataSize, AAS_DATATYPE_ASCIICHAR);
		SetDataAttributes((PVOID*)&(pAasBlockProductInfo->pwProductName), AAS_DATATYPE_ASCIICHAR, pbRawData, &dwNewIndex, dwDatalen, AAS_DATATYPE_ASCIICHAR);
	}

	// Fill PackageName field following expected data
	if (((*((PWORD)(pbRawData + dwNewIndex))) & AAS_DATATYPE_UNICODESTR)
		|| (((*((PWORD)(pbRawData + dwNewIndex))) & AAS_DATATYPE_NULLARG)))
	{
		dwDatalen = VerifyDataTypeAndGetDataSize(pbRawData, &dwNewIndex, dwRawDataSize, AAS_DATATYPE_UNICODESTR);

		if (!dwDatalen)
			SetDataAttributes((PVOID*)&(pAasBlockProductInfo->pwPackageName), AAS_DATATYPE_NULLARG, pbRawData, &dwNewIndex, dwDatalen, AAS_DATATYPE_NULLARG);
		else
		{
			SetDataAttributes((PVOID*)&(pAasBlockProductInfo->pwPackageName), AAS_DATATYPE_UNICODESTR, pbRawData, &dwNewIndex, dwDatalen, AAS_DATATYPE_UNICODESTR + dwDatalen);
			pAasBlockProductInfo->isPackageNameUNICODE = TRUE;
		}
	}
	else
	{
		dwDatalen = VerifyDataTypeAndGetDataSize(pbRawData, &dwNewIndex, dwRawDataSize, AAS_DATATYPE_ASCIICHAR);
		SetDataAttributes((PVOID*)&(pAasBlockProductInfo->pwPackageName), AAS_DATATYPE_ASCIICHAR, pbRawData, &dwNewIndex, dwDatalen, AAS_DATATYPE_ASCIICHAR);
	}

	// Fill Language field
	dwDatalen = VerifyDataTypeAndGetDataSize(pbRawData, &dwNewIndex, dwRawDataSize, AAS_DATATYPE_32BITSINT);
	SetDataAttributes((PVOID*)&(pAasBlockProductInfo->pdwLanguage), AAS_DATATYPE_32BITSINT, pbRawData, &dwNewIndex, dwDatalen, AAS_DATATYPE_32BITSINT);

	// Fill Version field
	dwDatalen = VerifyDataTypeAndGetDataSize(pbRawData, &dwNewIndex, dwRawDataSize, AAS_DATATYPE_32BITSINT);
	SetDataAttributes((PVOID*)&(pAasBlockProductInfo->pdwVersion), AAS_DATATYPE_32BITSINT, pbRawData, &dwNewIndex, dwDatalen, AAS_DATATYPE_32BITSINT);

	// Fill Assignment field
	dwDatalen = VerifyDataTypeAndGetDataSize(pbRawData, &dwNewIndex, dwRawDataSize, AAS_DATATYPE_32BITSINT);
	SetDataAttributes((PVOID*)&(pAasBlockProductInfo->pdwAssignment), AAS_DATATYPE_32BITSINT, pbRawData, &dwNewIndex, dwDatalen, AAS_DATATYPE_32BITSINT);

	// Fill ObsoleteArg field
	dwDatalen = VerifyDataTypeAndGetDataSize(pbRawData, &dwNewIndex, dwRawDataSize, AAS_DATATYPE_32BITSINT);
	SetDataAttributes((PVOID*)&(pAasBlockProductInfo->pdwObsoleteArg), AAS_DATATYPE_32BITSINT, pbRawData, &dwNewIndex, dwDatalen, AAS_DATATYPE_32BITSINT);

	// Fill ProductIcon field
	dwDatalen = VerifyDataTypeAndGetDataSize(pbRawData, &dwNewIndex, dwRawDataSize, AAS_DATATYPE_ASCIICHAR);
	SetDataAttributes((PVOID*)&(pAasBlockProductInfo->pwProductIcon), AAS_DATATYPE_ASCIICHAR, pbRawData, &dwNewIndex, dwDatalen, AAS_DATATYPE_ASCIICHAR);

	// Fill PackagemediaPath following expect format
	if (((*((PWORD)(pbRawData + dwNewIndex))) & AAS_DATATYPE_UNICODESTR)
		|| (((*((PWORD)(pbRawData + dwNewIndex))) & AAS_DATATYPE_NULLARG)))
	{
		dwDatalen = VerifyDataTypeAndGetDataSize(pbRawData, &dwNewIndex, dwRawDataSize, AAS_DATATYPE_UNICODESTR);
		if (!dwDatalen)
			SetDataAttributes((PVOID*)&(pAasBlockProductInfo->pwPackageMediaPath), AAS_DATATYPE_NULLARG, pbRawData, &dwNewIndex, dwDatalen, AAS_DATATYPE_NULLARG);
		else
		{
			SetDataAttributes((PVOID*)&(pAasBlockProductInfo->pwPackageMediaPath), AAS_DATATYPE_UNICODESTR, pbRawData, &dwNewIndex, dwDatalen, AAS_DATATYPE_UNICODESTR + dwDatalen);
			pAasBlockProductInfo->isPackageMediaPathUNICODE = TRUE;
		}
	}
	else
	{
		dwDatalen = VerifyDataTypeAndGetDataSize(pbRawData, &dwNewIndex, dwRawDataSize, AAS_DATATYPE_ASCIICHAR);
		SetDataAttributes((PVOID*)&(pAasBlockProductInfo->pwPackageMediaPath), AAS_DATATYPE_ASCIICHAR, pbRawData, &dwNewIndex, dwDatalen, AAS_DATATYPE_ASCIICHAR);
	}

	// Fill PackageCode field
	dwDatalen = VerifyDataTypeAndGetDataSize(pbRawData, &dwNewIndex, dwRawDataSize, AAS_DATATYPE_ASCIICHAR);
	SetDataAttributes((PVOID*)&(pAasBlockProductInfo->pwPackageCode), AAS_DATATYPE_ASCIICHAR, pbRawData, &dwNewIndex, dwDatalen, AAS_DATATYPE_ASCIICHAR);

	// Fill NullArgument 1 
	dwDatalen = VerifyDataTypeAndGetDataSize(pbRawData, &dwNewIndex, dwRawDataSize, AAS_DATATYPE_NULLARG);
	pAasBlockProductInfo->pbNullArgument1 = '\x0';

	// Fill NullArgument 2
	dwDatalen = VerifyDataTypeAndGetDataSize(pbRawData, &dwNewIndex, dwRawDataSize, AAS_DATATYPE_NULLARG);
	pAasBlockProductInfo->pbNullArgument2 = '\x0';

	// Fill InstanceType field
	dwDatalen = VerifyDataTypeAndGetDataSize(pbRawData, &dwNewIndex, dwRawDataSize, AAS_DATATYPE_32BITSINT);
	SetDataAttributes((PVOID*)&(pAasBlockProductInfo->pdwInstanceType), AAS_DATATYPE_32BITSINT, pbRawData, &dwNewIndex, dwDatalen, AAS_DATATYPE_32BITSINT);

	// Fill LuaSetting field
	dwDatalen = VerifyDataTypeAndGetDataSize(pbRawData, &dwNewIndex, dwRawDataSize, AAS_DATATYPE_32BITSINT);
	SetDataAttributes((PVOID*)&(pAasBlockProductInfo->pdwLUASetting), AAS_DATATYPE_32BITSINT, pbRawData, &dwNewIndex, dwDatalen, AAS_DATATYPE_32BITSINT);

	// Fill RemoteURTInstalls field
	dwDatalen = VerifyDataTypeAndGetDataSize(pbRawData, &dwNewIndex, dwRawDataSize, AAS_DATATYPE_32BITSINT);
	SetDataAttributes((PVOID*)&(pAasBlockProductInfo->pdwRemoteURTInstalls), AAS_DATATYPE_32BITSINT, pbRawData, &dwNewIndex, dwDatalen, AAS_DATATYPE_32BITSINT);

	// Fill ProductDeploymentFlags field
	dwDatalen = VerifyDataTypeAndGetDataSize(pbRawData, &dwNewIndex, dwRawDataSize, AAS_DATATYPE_32BITSINT);
	SetDataAttributes((PVOID*)&(pAasBlockProductInfo->pdwProductDeploymentFlags), AAS_DATATYPE_32BITSINT, pbRawData, &dwNewIndex, dwDatalen, AAS_DATATYPE_32BITSINT);

	*pdwIndex = dwNewIndex;
	return pAasBlockProductInfo;
}

PAAS_BLOCK_SOURCE_LIST_PUBLISH FillAasSourceListPublish(_In_ PBYTE pbRawData, _In_ PDWORD pdwIndex, _In_ DWORD dwRawDataSize)
{
	PAAS_BLOCK_SOURCE_LIST_PUBLISH pAasBlockSrcListPub = NULL;
	DWORD dwNewIndex = 0;
	DWORD dwDatalen = 0;

	if (!pbRawData || (*pdwIndex >= dwRawDataSize))
	{
		DEBUG_LOG(D_ERROR, "pbAASRawDATA pointer, pdwIndex or dwRawDataSize invalid.\r\nExiting now...");
		DoExit(D_ERROR);
	}
	dwNewIndex = *pdwIndex;

	pAasBlockSrcListPub = (PAAS_BLOCK_SOURCE_LIST_PUBLISH) HeapAlloc(hCrawlerHeap, HEAP_ZERO_MEMORY, sizeof (AAS_BLOCK_SOURCE_LIST_PUBLISH));
	if (!pAasBlockSrcListPub)
	{
		DEBUG_LOG(D_ERROR, "Unable to allocate AAS_BLOCK_END structure.\r\nExiting now...");
		DoExit(D_ERROR);
	}

	//Ensure that current block is an END block
	if ((BYTE)*(pbRawData + dwNewIndex) != AAS_OPCODE_SRCLISTPUB)
	{
		DEBUG_LOG(D_ERROR, "The current block doesn't seems to be a END block.\r\nExiting now...");
		DoExit(D_ERROR);
	}
	else
		dwNewIndex += sizeof(BYTE) * 2;

	// Fill PatchCode field
	if ((*((PWORD)(pbRawData + dwNewIndex))) == AAS_DATATYPE_NULLARG)
	{
		dwDatalen = VerifyDataTypeAndGetDataSize(pbRawData, &dwNewIndex, dwRawDataSize, AAS_DATATYPE_NULLARG);
		SetDataAttributes((PVOID*)&(pAasBlockSrcListPub->pwPatchCode), AAS_DATATYPE_NULLARG, pbRawData, &dwNewIndex, dwDatalen, AAS_DATATYPE_NULLARG);
	}
	else
	{
		dwDatalen = VerifyDataTypeAndGetDataSize(pbRawData, &dwNewIndex, dwRawDataSize, AAS_DATATYPE_ASCIICHAR);
		SetDataAttributes((PVOID*)&(pAasBlockSrcListPub->pwPatchCode), AAS_DATATYPE_ASCIICHAR, pbRawData, &dwNewIndex, dwDatalen, AAS_DATATYPE_ASCIICHAR);
	}

	// Fill PatchpackageName field
	if ((*((PWORD)(pbRawData + dwNewIndex))) == AAS_DATATYPE_NULLARG)
	{
		dwDatalen = VerifyDataTypeAndGetDataSize(pbRawData, &dwNewIndex, dwRawDataSize, AAS_DATATYPE_NULLARG);
		SetDataAttributes((PVOID*)&(pAasBlockSrcListPub->pwPatchPackageName), AAS_DATATYPE_NULLARG, pbRawData, &dwNewIndex, dwDatalen, AAS_DATATYPE_NULLARG);
	}
	else
	{
		dwDatalen = VerifyDataTypeAndGetDataSize(pbRawData, &dwNewIndex, dwRawDataSize, AAS_DATATYPE_ASCIICHAR);
		SetDataAttributes((PVOID*)&(pAasBlockSrcListPub->pwPatchPackageName), AAS_DATATYPE_ASCIICHAR, pbRawData, &dwNewIndex, dwDatalen, AAS_DATATYPE_ASCIICHAR);
	}
	
	// Fill DiskPromptTemplate field
	if (((*((PWORD)(pbRawData + dwNewIndex))) & AAS_DATATYPE_UNICODESTR)
		|| (((*((PWORD)(pbRawData + dwNewIndex))) & AAS_DATATYPE_NULLARG)))
	{
		dwDatalen = VerifyDataTypeAndGetDataSize(pbRawData, &dwNewIndex, dwRawDataSize, AAS_DATATYPE_UNICODESTR);

		if (!dwDatalen)
			SetDataAttributes((PVOID*)&(pAasBlockSrcListPub->pwDiskPromptTemplate), AAS_DATATYPE_NULLARG, pbRawData, &dwNewIndex, dwDatalen, AAS_DATATYPE_NULLARG);
		else
		{
			SetDataAttributes((PVOID*)&(pAasBlockSrcListPub->pwDiskPromptTemplate), AAS_DATATYPE_UNICODESTR, pbRawData, &dwNewIndex, dwDatalen, AAS_DATATYPE_UNICODESTR + dwDatalen);
			pAasBlockSrcListPub->isDiskPromptTemplateUNICODE = TRUE;
		}
	}
	else
	{
		dwDatalen = VerifyDataTypeAndGetDataSize(pbRawData, &dwNewIndex, dwRawDataSize, AAS_DATATYPE_ASCIICHAR);
		SetDataAttributes((PVOID*)&(pAasBlockSrcListPub->pwDiskPromptTemplate), AAS_DATATYPE_ASCIICHAR, pbRawData, &dwNewIndex, dwDatalen, AAS_DATATYPE_ASCIICHAR);
	}

		
	// Fill PackagePath field
	if (((*((PWORD)(pbRawData + dwNewIndex))) & AAS_DATATYPE_UNICODESTR)
		|| (((*((PWORD)(pbRawData + dwNewIndex))) & AAS_DATATYPE_NULLARG)))
	{
		dwDatalen = VerifyDataTypeAndGetDataSize(pbRawData, &dwNewIndex, dwRawDataSize, AAS_DATATYPE_UNICODESTR);

		if (!dwDatalen)
			SetDataAttributes((PVOID*)&(pAasBlockSrcListPub->pwPackagePath), AAS_DATATYPE_NULLARG, pbRawData, &dwNewIndex, dwDatalen, AAS_DATATYPE_NULLARG);
		else
		{
			SetDataAttributes((PVOID*)&(pAasBlockSrcListPub->pwPackagePath), AAS_DATATYPE_UNICODESTR, pbRawData, &dwNewIndex, dwDatalen, AAS_DATATYPE_UNICODESTR + dwDatalen);
			pAasBlockSrcListPub->isPackagePathUNICODE = TRUE;
		}
	}
	else
	{
		dwDatalen = VerifyDataTypeAndGetDataSize(pbRawData, &dwNewIndex, dwRawDataSize, AAS_DATATYPE_ASCIICHAR);
		SetDataAttributes((PVOID*)&(pAasBlockSrcListPub->pwPackagePath), AAS_DATATYPE_ASCIICHAR, pbRawData, &dwNewIndex, dwDatalen, AAS_DATATYPE_ASCIICHAR);
	}	
	
	// Fill NumberOfDisk field
	dwDatalen = VerifyDataTypeAndGetDataSize(pbRawData, &dwNewIndex, dwRawDataSize, AAS_DATATYPE_32BITSINT);
	SetDataAttributes((PVOID*)&(pAasBlockSrcListPub->pdwNumberOfDisks), AAS_DATATYPE_32BITSINT, pbRawData, &dwNewIndex, dwDatalen,AAS_DATATYPE_32BITSINT);
			
	for (DWORD i = 0; i < *(pAasBlockSrcListPub->pdwNumberOfDisks); ++i)
	{
		// Fill DiskID field
		dwDatalen = VerifyDataTypeAndGetDataSize(pbRawData, &dwNewIndex, dwRawDataSize, AAS_DATATYPE_32BITSINT);
		SetDataAttributes((PVOID*)&(pAasBlockSrcListPub->sDisks[i].pdwDiskId), AAS_DATATYPE_32BITSINT, pbRawData, &dwNewIndex, dwDatalen, AAS_DATATYPE_32BITSINT);

		// Fill VolumeName for each disk
		if (((*((PWORD)(pbRawData + dwNewIndex))) & AAS_DATATYPE_UNICODESTR)
			|| (((*((PWORD)(pbRawData + dwNewIndex))) & AAS_DATATYPE_NULLARG)))
		{
			dwDatalen = VerifyDataTypeAndGetDataSize(pbRawData, &dwNewIndex, dwRawDataSize, AAS_DATATYPE_UNICODESTR);
	
			if (!dwDatalen)
				SetDataAttributes((PVOID*)&(pAasBlockSrcListPub->sDisks[i].pwVolumeName), AAS_DATATYPE_NULLARG, pbRawData, &dwNewIndex, dwDatalen, AAS_DATATYPE_NULLARG);
			else
			{
				SetDataAttributes((PVOID*)&(pAasBlockSrcListPub->sDisks[i].pwVolumeName), AAS_DATATYPE_UNICODESTR, pbRawData, &dwNewIndex, dwDatalen, AAS_DATATYPE_UNICODESTR + dwDatalen);
				pAasBlockSrcListPub->sDisks[i].isVolumeNameUNICODE = TRUE;
			}
		}
		else
		{
			dwDatalen = VerifyDataTypeAndGetDataSize(pbRawData, &dwNewIndex, dwRawDataSize, AAS_DATATYPE_ASCIICHAR);
			SetDataAttributes((PVOID*)&(pAasBlockSrcListPub->sDisks[i].pwVolumeName), AAS_DATATYPE_ASCIICHAR, pbRawData, &dwNewIndex, dwDatalen, AAS_DATATYPE_ASCIICHAR);
		}

		// Fill DiskPrompt for each disk
		if (((*((PWORD)(pbRawData + dwNewIndex))) & AAS_DATATYPE_UNICODESTR)
			|| (((*((PWORD)(pbRawData + dwNewIndex))) & AAS_DATATYPE_NULLARG)))
		{
			dwDatalen = VerifyDataTypeAndGetDataSize(pbRawData, &dwNewIndex, dwRawDataSize, AAS_DATATYPE_UNICODESTR);

			if (!dwDatalen)
				SetDataAttributes((PVOID*)&(pAasBlockSrcListPub->sDisks[i].pwDiskPrompt), AAS_DATATYPE_NULLARG, pbRawData, &dwNewIndex, dwDatalen, AAS_DATATYPE_NULLARG);
			else
			{
				SetDataAttributes((PVOID*)&(pAasBlockSrcListPub->sDisks[i].pwDiskPrompt), AAS_DATATYPE_UNICODESTR, pbRawData, &dwNewIndex, dwDatalen, AAS_DATATYPE_UNICODESTR + dwDatalen);
				pAasBlockSrcListPub->sDisks[i].isDiskPromptUNICODE = TRUE;
			}
		}
		else
		{
			dwDatalen = VerifyDataTypeAndGetDataSize(pbRawData, &dwNewIndex, dwRawDataSize, AAS_DATATYPE_ASCIICHAR);
			SetDataAttributes((PVOID*)&(pAasBlockSrcListPub->sDisks[i].pwDiskPrompt), AAS_DATATYPE_ASCIICHAR, pbRawData, &dwNewIndex, dwDatalen, AAS_DATATYPE_ASCIICHAR);
		}
	}

	// Fill LaunchPath field
	if (((*((PWORD)(pbRawData + dwNewIndex))) & AAS_DATATYPE_UNICODESTR)
		|| (((*((PWORD)(pbRawData + dwNewIndex))) & AAS_DATATYPE_NULLARG)))
	{
		dwDatalen = VerifyDataTypeAndGetDataSize(pbRawData, &dwNewIndex, dwRawDataSize, AAS_DATATYPE_UNICODESTR);

		if (!dwDatalen)
			SetDataAttributes((PVOID*)&(pAasBlockSrcListPub->pwLaunchPath), AAS_DATATYPE_NULLARG, pbRawData, &dwNewIndex, dwDatalen, AAS_DATATYPE_NULLARG);
		else
		{
			SetDataAttributes((PVOID*)&(pAasBlockSrcListPub->pwLaunchPath), AAS_DATATYPE_UNICODESTR, pbRawData, &dwNewIndex, dwDatalen, AAS_DATATYPE_UNICODESTR + dwDatalen);
			pAasBlockSrcListPub->isLaunchPathUNICODE = TRUE;
		}
	}
	else
	{
		dwDatalen = VerifyDataTypeAndGetDataSize(pbRawData, &dwNewIndex, dwRawDataSize, AAS_DATATYPE_ASCIICHAR);
		SetDataAttributes((PVOID*)&(pAasBlockSrcListPub->pwLaunchPath), AAS_DATATYPE_ASCIICHAR, pbRawData, &dwNewIndex, dwDatalen, AAS_DATATYPE_ASCIICHAR);
	}

	*pdwIndex = dwNewIndex;
	return pAasBlockSrcListPub;
}

PAAS_BLOCK_PRODUCT_PUBLISH FillAasProductPublish(_In_ PBYTE pbRawData, _In_ PDWORD pdwIndex, _In_ DWORD dwRawDataSize)
{
	PAAS_BLOCK_PRODUCT_PUBLISH pAasBlockProductPublish = NULL;
	DWORD dwNewIndex = 0;
	DWORD dwDatalen = 0;

	if (!pbRawData || (*pdwIndex >= dwRawDataSize))
	{
		DEBUG_LOG(D_ERROR, "pbAASRawDATA pointer, pdwIndex or dwRawDataSize invalid.\r\nExiting now...");
		DoExit(D_ERROR);
	}
	dwNewIndex = *pdwIndex;

	if ((dwRawDataSize - dwNewIndex) < AAS_BLOCK_PRODUCT_PUBLISH_SIZE)
	{
		DEBUG_LOG(D_ERROR, "Size of pbRawData is invalid: less than the minimum length required for AAS Product Info.\r\nExiting now...");
		DoExit(D_ERROR);
	}

	pAasBlockProductPublish = (PAAS_BLOCK_PRODUCT_PUBLISH) HeapAlloc(hCrawlerHeap, HEAP_ZERO_MEMORY, sizeof (AAS_BLOCK_PRODUCT_PUBLISH));
	if (!pAasBlockProductPublish)
	{
		DEBUG_LOG(D_ERROR, "Unable to allocate AAS_BLOCK_PRODUCT_PUBLISH structure.\r\nExiting now...");
		DoExit(D_ERROR);
	}

	//Ensure that current block is a PRODUCT_PUBLISH block
	if ((BYTE)*(pbRawData + dwNewIndex) != AAS_OPCODE_PRODUCTPUB)
	{
		DEBUG_LOG(D_ERROR, "The current block doesn't seems to be a PRODUCTINFO block.\r\nExiting now...");
		DoExit(D_ERROR);
	}
	else
		dwNewIndex += sizeof(BYTE) * 2;

	// Fill Package Key field
	dwDatalen = VerifyDataTypeAndGetDataSize(pbRawData, &dwNewIndex, dwRawDataSize, AAS_DATATYPE_ASCIICHAR);
	SetDataAttributes((PVOID*)&(pAasBlockProductPublish->pwProductPublish), AAS_DATATYPE_ASCIICHAR, pbRawData, &dwNewIndex, dwDatalen, AAS_DATATYPE_ASCIICHAR);

	*pdwIndex = dwNewIndex;
	return pAasBlockProductPublish;
}

PAAS_BLOCK_END FillAasEnd(_In_ PBYTE pbRawData, _In_ PDWORD pdwIndex, _In_ DWORD dwRawDataSize)
{
	PAAS_BLOCK_END pAasBlockEnd = NULL;
	DWORD dwNewIndex = 0;
	DWORD dwDatalen = 0;

	if (!pbRawData || (*pdwIndex >= dwRawDataSize))
	{
		DEBUG_LOG(D_ERROR, "pbAASRawDATA pointer, pdwIndex or dwRawDataSize invalid.\r\nExiting now...");
		DoExit(D_ERROR);
	}
	dwNewIndex = *pdwIndex;

	if ((dwRawDataSize - dwNewIndex) < AAS_BLOCK_END_SIZE)
	{
		DEBUG_LOG(D_ERROR, "Size of pbRawData is invalid: less than the minimum length required for AAS Product Info.\r\nExiting now...");
		DoExit(D_ERROR);
	}

	pAasBlockEnd = (PAAS_BLOCK_END) HeapAlloc(hCrawlerHeap, HEAP_ZERO_MEMORY, sizeof (AAS_BLOCK_END));
	if (!pAasBlockEnd)
	{
		DEBUG_LOG(D_ERROR, "Unable to allocate AAS_BLOCK_END structure.\r\nExiting now...");
		DoExit(D_ERROR);
	}

	//Ensure that current block is an END block
	if ((BYTE)*(pbRawData + dwNewIndex) != AAS_OPCODE_END)
	{
		DEBUG_LOG(D_ERROR, "The current block doesn't seems to be a END block.\r\nExiting now...");
		DoExit(D_ERROR);
	}
	else
		dwNewIndex += sizeof(BYTE) * 2;

	// Fill Checksum field
	dwDatalen = VerifyDataTypeAndGetDataSize(pbRawData, &dwNewIndex, dwRawDataSize, AAS_DATATYPE_32BITSINT);
	SetDataAttributes((PVOID*)&(pAasBlockEnd->pdwChecksum), AAS_DATATYPE_32BITSINT, pbRawData, &dwNewIndex, dwDatalen, AAS_DATATYPE_32BITSINT);

	// Fill ProgressTotalHDWorld field
	dwDatalen = VerifyDataTypeAndGetDataSize(pbRawData, &dwNewIndex, dwRawDataSize, AAS_DATATYPE_32BITSINT);
	SetDataAttributes((PVOID*)&(pAasBlockEnd->pdwProgressTotalHDWord), AAS_DATATYPE_32BITSINT, pbRawData, &dwNewIndex, dwDatalen, AAS_DATATYPE_32BITSINT);

	// Fill ProgressTotalLDWord field
	dwDatalen = VerifyDataTypeAndGetDataSize(pbRawData, &dwNewIndex, dwRawDataSize, AAS_DATATYPE_32BITSINT);
	SetDataAttributes((PVOID*)&(pAasBlockEnd->pdwProgressTotalLDWord), AAS_DATATYPE_32BITSINT, pbRawData, &dwNewIndex, dwDatalen, AAS_DATATYPE_32BITSINT);

	*pdwIndex = dwNewIndex;
	return pAasBlockEnd;
}

PAAS_BLOCK_UNKNOWN FillAasUnknownBlock(_In_ PBYTE pbRawData, _In_ PDWORD pdwIndex, _In_ DWORD dwRawDataSize)
{
	PAAS_BLOCK_UNKNOWN pAasUnkwnownBlock = NULL;
	DWORD dwNewIndex = 0;
	DWORD dwDatalen = 0;
	BYTE bOpCode = 0, bNumberOfArg = 0;

	if (!pbRawData || (*pdwIndex >= dwRawDataSize))
	{
		DEBUG_LOG(D_ERROR, "pbAASRawDATA pointer, pdwIndex or dwRawDataSize invalid.\r\nExiting now...");
		DoExit(D_ERROR);
	}
	dwNewIndex = *pdwIndex;

	pAasUnkwnownBlock = (PAAS_BLOCK_UNKNOWN) HeapAlloc(hCrawlerHeap, HEAP_ZERO_MEMORY, sizeof (AAS_BLOCK_UNKNOWN));
	if (!pAasUnkwnownBlock)
	{
		DEBUG_LOG(D_ERROR, "Unable to allocate AAS_BLOCK_UNKNOWN structure.\r\nExiting now...");
		DoExit(D_ERROR);
	}

	bOpCode = ((BYTE)*(pbRawData + dwNewIndex));
	bNumberOfArg += ((BYTE)*(pbRawData + dwNewIndex + 1));
	dwNewIndex += sizeof(BYTE) * 2;

	pAasUnkwnownBlock->bOpcodeNumber = bOpCode;
	pAasUnkwnownBlock->bArgumentNumber = bNumberOfArg;

	for (BYTE i = 0; i < bNumberOfArg; ++i)
	{
		WORD wDataType = 0;
		DWORD wDataLen = 0;

		if ((dwNewIndex + 2) >= dwRawDataSize)
		{
			DEBUG_LOG(D_ERROR, "Size of pbRawData is invalid: less than the minimum length required for DATA_TYPE Block.\r\nExiting now...");
			DoExit(D_ERROR);
		}
		wDataType = *((PWORD)(pbRawData + dwNewIndex));
		wDataLen = (DWORD) GetDataSize(pbRawData, &dwNewIndex, dwRawDataSize);
		pAasUnkwnownBlock->sDataUnkwnown[i].wDataType = wDataType;
		pAasUnkwnownBlock->sDataUnkwnown[i].wDataLen = wDataLen;
		
		if (!wDataType)
		{
			pAasUnkwnownBlock->sDataUnkwnown[i].wDataType = AAS_DATATYPE_NULLSTRING;
			SetDataAttributes((PVOID *)&(pAasUnkwnownBlock->sDataUnkwnown[i].pbData), AAS_DATATYPE_NULLSTRING, pbRawData, &dwNewIndex, wDataLen, wDataType);
		}
		else if ((wDataType == AAS_DATATYPE_32BITSINT)  && (wDataType != AAS_DATATYPE_EXTDEDSIZE))
		{
			pAasUnkwnownBlock->sDataUnkwnown[i].wDataType = AAS_DATATYPE_32BITSINT;
			SetDataAttributes((PVOID *)&(pAasUnkwnownBlock->sDataUnkwnown[i].pbData), AAS_DATATYPE_32BITSINT, pbRawData, &dwNewIndex, wDataLen, wDataType);
		}
		else if (wDataType == AAS_DATATYPE_NULLARG)
		{
			pAasUnkwnownBlock->sDataUnkwnown[i].wDataType = AAS_DATATYPE_NULLARG;
			SetDataAttributes((PVOID *)&(pAasUnkwnownBlock->sDataUnkwnown[i].pbData), AAS_DATATYPE_NULLARG, pbRawData, &dwNewIndex, wDataLen, wDataType);
		}
		else if (wDataType == AAS_DATATYPE_EXTDEDSIZE)
		{
			pAasUnkwnownBlock->sDataUnkwnown[i].wDataType = AAS_DATATYPE_EXTDEDSIZE;
			SetDataAttributes((PVOID *)&(pAasUnkwnownBlock->sDataUnkwnown[i].pbData), AAS_DATATYPE_EXTDEDSIZE, pbRawData, &dwNewIndex, wDataLen, wDataType);
		}
		else if ((wDataType & AAS_DATATYPE_BINARYSTRM) && (wDataType > AAS_DATATYPE_NULLARG) && (wDataType < AAS_DATATYPE_EXTDEDSIZE))
		{
			pAasUnkwnownBlock->sDataUnkwnown[i].wDataType = AAS_DATATYPE_BINARYSTRM;
			SetDataAttributes((PVOID *)&(pAasUnkwnownBlock->sDataUnkwnown[i].pbData), AAS_DATATYPE_BINARYSTRM, pbRawData, &dwNewIndex, wDataLen, wDataType);
		}
		else if ((wDataType & AAS_DATATYPE_UNICODESTR) && (wDataType > AAS_DATATYPE_UNICODESTR))
		{
			pAasUnkwnownBlock->sDataUnkwnown[i].wDataType = AAS_DATATYPE_UNICODESTR;
			SetDataAttributes((PVOID *)&(pAasUnkwnownBlock->sDataUnkwnown[i].pbData), AAS_DATATYPE_UNICODESTR, pbRawData, &dwNewIndex, wDataLen, wDataType + wDataLen);
		}
		else if (wDataType > 0)
		{
			pAasUnkwnownBlock->sDataUnkwnown[i].wDataType = AAS_DATATYPE_ASCIICHAR;
			SetDataAttributes((PVOID *)&(pAasUnkwnownBlock->sDataUnkwnown[i].pbData), AAS_DATATYPE_ASCIICHAR, pbRawData, &dwNewIndex, wDataLen, wDataType);
		}
		else
		{
			DEBUG_LOG(D_ERROR, "wDataType invalid !.\r\nExiting now...");
			DoExit(D_ERROR);
		}
	}

	*pdwIndex = dwNewIndex;
	return pAasUnkwnownBlock;
}

BOOL FreeAasHeaderBlock(_Inout_ PAAS_BLOCK_HEADER pAasHeaderBlock)
{
	if (pAasHeaderBlock == NULL)
	{
		DEBUG_LOG(D_ERROR, "AAS_BLOCK_HEADER pointer invalid.\r\nExiting now...");
		DoExit(D_ERROR);
	}

	if (pAasHeaderBlock->pdwSignature)
		HeapFree(hCrawlerHeap, NULL, pAasHeaderBlock->pdwSignature);
	if (pAasHeaderBlock->pdwVersion)
		HeapFree(hCrawlerHeap, NULL, pAasHeaderBlock->pdwVersion);
	if (pAasHeaderBlock->pdwDosTimeStamp)
		HeapFree(hCrawlerHeap, NULL, pAasHeaderBlock->pdwDosTimeStamp);
	if (pAasHeaderBlock->pdwLangID)
		HeapFree(hCrawlerHeap, NULL, pAasHeaderBlock->pdwLangID);
	if (pAasHeaderBlock->pdwPlatform)
		HeapFree(hCrawlerHeap, NULL, pAasHeaderBlock->pdwPlatform);
	if (pAasHeaderBlock->pdwScriptType)
		HeapFree(hCrawlerHeap, NULL, pAasHeaderBlock->pdwScriptType);
	if (pAasHeaderBlock->pdwScriptMajorVersion)
		HeapFree(hCrawlerHeap, NULL, pAasHeaderBlock->pdwScriptMajorVersion);
	if (pAasHeaderBlock->pdwScriptMinorVersion)
		HeapFree(hCrawlerHeap, NULL, pAasHeaderBlock->pdwScriptMinorVersion);
	if (pAasHeaderBlock->pdwScriptAttributes)
		HeapFree(hCrawlerHeap, NULL, pAasHeaderBlock->pdwScriptAttributes);

	HeapFree(hCrawlerHeap, NULL, pAasHeaderBlock);
	pAasHeaderBlock = NULL;
	return TRUE;
}

BOOL FreeAasProductInfoBlock(_Inout_ PAAS_BLOCK_PRODUCT_INFO pAasProductInfoBlock)
{
	if (pAasProductInfoBlock == NULL)
	{
		DEBUG_LOG(D_ERROR, "AAS_BLOCK_PRODUCT_INFO pointer invalid.\r\nExiting now...");
		DoExit(D_ERROR);
	}

	if (pAasProductInfoBlock->pwProductKey)
		HeapFree(hCrawlerHeap, NULL, pAasProductInfoBlock->pwProductKey);
	if (pAasProductInfoBlock->pwProductName)
		HeapFree(hCrawlerHeap, NULL, pAasProductInfoBlock->pwProductName);
	if (pAasProductInfoBlock->pwPackageName)
		HeapFree(hCrawlerHeap, NULL, pAasProductInfoBlock->pwPackageName);
	if (pAasProductInfoBlock->pdwLanguage)
		HeapFree(hCrawlerHeap, NULL, pAasProductInfoBlock->pdwLanguage);
	if (pAasProductInfoBlock->pdwVersion)
		HeapFree(hCrawlerHeap, NULL, pAasProductInfoBlock->pdwVersion);
	if (pAasProductInfoBlock->pdwAssignment)
		HeapFree(hCrawlerHeap, NULL, pAasProductInfoBlock->pdwAssignment);
	if (pAasProductInfoBlock->pdwObsoleteArg)
		HeapFree(hCrawlerHeap, NULL, pAasProductInfoBlock->pdwObsoleteArg);
	if (pAasProductInfoBlock->pwProductIcon)
		HeapFree(hCrawlerHeap, NULL, pAasProductInfoBlock->pwProductIcon);
	if (pAasProductInfoBlock->pwPackageMediaPath)
		HeapFree(hCrawlerHeap, NULL, pAasProductInfoBlock->pwPackageMediaPath);
	if (pAasProductInfoBlock->pwPackageCode)
		HeapFree(hCrawlerHeap, NULL, pAasProductInfoBlock->pwPackageCode);
	if (pAasProductInfoBlock->pbNullArgument1)
		HeapFree(hCrawlerHeap, NULL, pAasProductInfoBlock->pbNullArgument1);
	if (pAasProductInfoBlock->pbNullArgument2)
		HeapFree(hCrawlerHeap, NULL, pAasProductInfoBlock->pbNullArgument2);
	if (pAasProductInfoBlock->pdwInstanceType)
		HeapFree(hCrawlerHeap, NULL, pAasProductInfoBlock->pdwInstanceType);
	if (pAasProductInfoBlock->pdwLUASetting)
		HeapFree(hCrawlerHeap, NULL, pAasProductInfoBlock->pdwLUASetting);
	if (pAasProductInfoBlock->pdwRemoteURTInstalls)
		HeapFree(hCrawlerHeap, NULL, pAasProductInfoBlock->pdwRemoteURTInstalls);
	if (pAasProductInfoBlock->pdwProductDeploymentFlags)
		HeapFree(hCrawlerHeap, NULL, pAasProductInfoBlock->pdwProductDeploymentFlags);

	HeapFree(hCrawlerHeap, NULL, pAasProductInfoBlock);
	pAasProductInfoBlock = NULL;
	return TRUE;
}

BOOL FreeAasSourceListPublishBlock(_Inout_ PAAS_BLOCK_SOURCE_LIST_PUBLISH pAasSourceListPublishBlock)
{
	if (pAasSourceListPublishBlock == NULL)
	{
		DEBUG_LOG(D_ERROR, "AAS_BLOCK_SOURCE_LIST_PUBLISH pointer invalid.\r\nExiting now...");
		DoExit(D_ERROR);
	}

	if (pAasSourceListPublishBlock->pwPatchCode)
		HeapFree(hCrawlerHeap, NULL, pAasSourceListPublishBlock->pwPatchCode);
	if (pAasSourceListPublishBlock->pwPatchPackageName)
		HeapFree(hCrawlerHeap, NULL, pAasSourceListPublishBlock->pwPatchPackageName);
	if (pAasSourceListPublishBlock->pwDiskPromptTemplate)
		HeapFree(hCrawlerHeap, NULL, pAasSourceListPublishBlock->pwDiskPromptTemplate);
	if (pAasSourceListPublishBlock->pwPackagePath)
		HeapFree(hCrawlerHeap, NULL, pAasSourceListPublishBlock->pwPackagePath);
	if (pAasSourceListPublishBlock->pwLaunchPath)
		HeapFree(hCrawlerHeap, NULL, pAasSourceListPublishBlock->pwLaunchPath);

	if (pAasSourceListPublishBlock->pdwNumberOfDisks)
	{
		for (DWORD i = 0; i < *(pAasSourceListPublishBlock->pdwNumberOfDisks); ++i)
		{
			if (pAasSourceListPublishBlock->sDisks[i].pwDiskPrompt)
				HeapFree(hCrawlerHeap, NULL, pAasSourceListPublishBlock->sDisks[i].pwDiskPrompt);
			if (pAasSourceListPublishBlock->sDisks[i].pwVolumeName)
				HeapFree(hCrawlerHeap, NULL, pAasSourceListPublishBlock->sDisks[i].pwVolumeName);
			if (pAasSourceListPublishBlock->sDisks[i].pdwDiskId)
				HeapFree(hCrawlerHeap, NULL, pAasSourceListPublishBlock->sDisks[i].pdwDiskId);
		}
		HeapFree(hCrawlerHeap, NULL, pAasSourceListPublishBlock->pdwNumberOfDisks);
	}

	HeapFree(hCrawlerHeap, NULL, pAasSourceListPublishBlock);
	pAasSourceListPublishBlock = NULL;
	return TRUE;
}

BOOL FreeAasProductPublishBlock(_Inout_ PAAS_BLOCK_PRODUCT_PUBLISH pAasProductPublishBlock)
{
	if (pAasProductPublishBlock == NULL)
	{
		DEBUG_LOG(D_ERROR, "AAS_BLOCK_PRODUCT_PUBLISH pointer invalid.\r\nExiting now...");
		DoExit(D_ERROR);
	}

	if (pAasProductPublishBlock->pwProductPublish)
		HeapFree(hCrawlerHeap, NULL, pAasProductPublishBlock->pwProductPublish);

	HeapFree(hCrawlerHeap, NULL, pAasProductPublishBlock);
	pAasProductPublishBlock = NULL;
	return TRUE;
}

BOOL FreeAasEndBlock(_Inout_ PAAS_BLOCK_END pAasEndBlock)
{
	if (pAasEndBlock == NULL)
	{
		DEBUG_LOG(D_ERROR, "AAS_BLOCK_END pointer invalid.\r\nExiting now...");
		DoExit(D_ERROR);
	}

	if (pAasEndBlock->pdwChecksum)
		HeapFree(hCrawlerHeap, NULL, pAasEndBlock->pdwChecksum);
	if (pAasEndBlock->pdwProgressTotalHDWord)
		HeapFree(hCrawlerHeap, NULL, pAasEndBlock->pdwProgressTotalHDWord);
	if (pAasEndBlock->pdwProgressTotalLDWord)
		HeapFree(hCrawlerHeap, NULL, pAasEndBlock->pdwProgressTotalLDWord);

	HeapFree(hCrawlerHeap, NULL, pAasEndBlock);
	pAasEndBlock = NULL;
	return TRUE;
}

BOOL FreeAasUnknownBlock(_Inout_ PAAS_BLOCK_UNKNOWN pAasUnkwnownBlock)
{
	if (pAasUnkwnownBlock == NULL)
	{
		DEBUG_LOG(D_ERROR, "AAS_BLOCK_END pointer invalid.\r\nExiting now...");
		DoExit(D_ERROR);
	}

	if (pAasUnkwnownBlock->bArgumentNumber)
	{
		for (DWORD i = 0; i < pAasUnkwnownBlock->bArgumentNumber; ++i)
		{
			if (pAasUnkwnownBlock->sDataUnkwnown[i].pbData)
				HeapFree(hCrawlerHeap, NULL, pAasUnkwnownBlock->sDataUnkwnown[i].pbData);
		}
	}

	HeapFree(hCrawlerHeap, NULL, pAasUnkwnownBlock);
	pAasUnkwnownBlock = NULL;
	return TRUE;
}

DWORD GetDataSize(_In_ PBYTE pbRawData, _In_ PDWORD pdwIndex, _In_ DWORD dwRawDataSize)
{
	WORD wDataType = 0;
	DWORD dwOutputSize = 0;
	DWORD dwSizeRes = 0;

	if (!pbRawData || !pdwIndex)
	{
		DEBUG_LOG(D_ERROR, "pbRawData pointer, pdwIndex pointer is invalid.\r\nExiting now...");
		DoExit(D_ERROR);
	}

	if ((*pdwIndex + 2) >= dwRawDataSize)
	{
		DEBUG_LOG(D_ERROR, "Size of pbRawData is invalid: less than the minimum length required for DATA_TYPE Block.\r\nExiting now...");
		DoExit(D_ERROR);
	}
	wDataType = *((PWORD)(pbRawData + *pdwIndex));

	if (!wDataType)
		dwSizeRes = 0;
	else if ((wDataType == AAS_DATATYPE_32BITSINT) && (wDataType != AAS_DATATYPE_EXTDEDSIZE))
		dwSizeRes = 4;
	else if (wDataType == AAS_DATATYPE_NULLARG)
		dwSizeRes = 0;
	else if (wDataType == AAS_DATATYPE_EXTDEDSIZE)
	{
		DWORD dwNewSize = 0;
	
		dwNewSize = *((PDWORD)(pbRawData + *pdwIndex + 2));
		*pdwIndex += sizeof(DWORD);

		dwSizeRes = (dwNewSize << 2) >> 2;
	}
	else if ((wDataType & AAS_DATATYPE_BINARYSTRM) && (wDataType > AAS_DATATYPE_NULLARG) && (wDataType < AAS_DATATYPE_UNICODESTR))
		dwSizeRes = (wDataType - AAS_DATATYPE_NULLARG);
	else if ((wDataType & AAS_DATATYPE_UNICODESTR) && (wDataType > AAS_DATATYPE_UNICODESTR))
		dwSizeRes = (wDataType - AAS_DATATYPE_UNICODESTR);
	else if (wDataType > 0)
		dwSizeRes = wDataType;
	else
	{
		DEBUG_LOG(D_ERROR, "wDataType invalid !.\r\nExiting now...");
		DoExit(D_ERROR);
		return 0;
	}
	*pdwIndex += sizeof(BYTE) * 2;
	return dwSizeRes;
}

VOID SetDataAttributes(_Inout_ PVOID *pvAttribute, _In_ AAS_BLOCK_DATATYPE wDataType, _In_ PBYTE pbRawData, _In_ PDWORD pdwIndex, _In_ DWORD dwDatalen, _In_ DWORD dwRealDataType)
{
	DWORD dwNewIndex = 0;

	if (!pbRawData || !pdwIndex)
	{
		DEBUG_LOG(D_ERROR, "pbRawData pointer or pdwIndex invalid.\r\nExiting now...");
		DoExit(D_ERROR);
	}
	dwNewIndex = *pdwIndex;

	if ((!wDataType) && (!dwDatalen))
	{
		PWCHAR tNull = TEXT("(null)");
		*pvAttribute = (PDWORD) HeapAlloc(hCrawlerHeap, NULL, (_tcslen(tNull) + 1) * sizeof(WCHAR));
		memcpy_s(*pvAttribute, _tcslen(tNull) * sizeof(WCHAR), tNull, _tcslen(tNull) * sizeof(WCHAR));
		(*(PWCHAR *)pvAttribute)[_tcslen(tNull)] = TEXT('\0');
	}
	else if (wDataType == AAS_DATATYPE_32BITSINT)
	{
		*pvAttribute = (PDWORD) HeapAlloc(hCrawlerHeap, NULL, sizeof(DWORD));
		memcpy_s(*pvAttribute, sizeof(DWORD), ((PBYTE)(pbRawData + dwNewIndex)), sizeof(DWORD));
		dwNewIndex += sizeof(DWORD);
	}
	else if ((wDataType == AAS_DATATYPE_NULLARG) && (!dwDatalen))
	{
		PWCHAR tNull = TEXT("(null)");
		*pvAttribute = (PDWORD) HeapAlloc(hCrawlerHeap, NULL, (_tcslen(tNull) + 1) * sizeof(WCHAR));
		memcpy_s(*pvAttribute, _tcslen(tNull) * sizeof(WCHAR), tNull, _tcslen(tNull) * sizeof(WCHAR));
		(*(PWCHAR *)pvAttribute)[_tcslen(tNull)] = TEXT('\0');
	}
	else if ((wDataType == AAS_DATATYPE_EXTDEDSIZE) && (dwDatalen) && (dwRealDataType == AAS_DATATYPE_EXTDEDSIZE))
	{
		*pvAttribute = (PBYTE) HeapAlloc(hCrawlerHeap, NULL, sizeof(BYTE) * (dwDatalen));
		memcpy_s(*pvAttribute, sizeof(BYTE) * (dwDatalen), ((PBYTE)(pbRawData + dwNewIndex)), dwDatalen);
		dwNewIndex += sizeof(BYTE) * dwDatalen;
	}
	else if ((wDataType == AAS_DATATYPE_BINARYSTRM) && (dwDatalen))
	{
		*pvAttribute = (PBYTE) HeapAlloc(hCrawlerHeap, NULL, sizeof(BYTE) * (dwDatalen));
		memcpy_s(*pvAttribute, sizeof(BYTE) * (dwDatalen), ((PBYTE)(pbRawData + dwNewIndex)), dwDatalen);
		dwNewIndex += sizeof(BYTE) * dwDatalen;
	}
	else if ((wDataType == AAS_DATATYPE_UNICODESTR)  && (dwDatalen) && (dwRealDataType > AAS_DATATYPE_UNICODESTR))
	{
		*pvAttribute = (PWCHAR) HeapAlloc(hCrawlerHeap, NULL, sizeof(WCHAR) * (dwDatalen + 1));
		memcpy_s(*pvAttribute, sizeof(WCHAR) * (dwDatalen + 1), ((PWCHAR)(pbRawData + dwNewIndex)), dwDatalen);
		(*(PWCHAR *)pvAttribute)[dwDatalen] = TEXT('\0');
		dwNewIndex += sizeof(WCHAR) * dwDatalen;
	}
	else if (wDataType == AAS_DATATYPE_ASCIICHAR)
	{
		PCHAR pcTmp = NULL;

		pcTmp = (PCHAR) HeapAlloc(hCrawlerHeap, NULL, sizeof(CHAR) * (dwDatalen + 1));
		memcpy_s(pcTmp, sizeof(CHAR) * (dwDatalen + 1), ((PCHAR)(pbRawData + dwNewIndex)), dwDatalen);
		pcTmp[dwDatalen] = '\0';
		
		*pvAttribute = CStrToPtchar((PBYTE)pcTmp, (DWORD) strlen(pcTmp));
		dwNewIndex += sizeof(CHAR) * dwDatalen;
		HeapFree(hCrawlerHeap, NULL, pcTmp);
	}

	*pdwIndex = dwNewIndex;
}

DWORD VerifyDataTypeAndGetDataSize(_In_ PBYTE pbRawData, _In_ PDWORD pdwIndex, _In_ DWORD dwRawDataSize, _In_ AAS_BLOCK_DATATYPE dwRequiredBlockDataType)
{
	DWORD dwDataSize = 0;

	if (!pbRawData || !pdwIndex)
	{
		DEBUG_LOG(D_ERROR, "pbRawData pointer, pdwIndex pointer is invalid.\r\nExiting now...");
		DoExit(D_ERROR);
	}

	dwDataSize = GetDataSize(pbRawData, pdwIndex, dwRawDataSize);

	if (((dwRequiredBlockDataType == AAS_DATATYPE_NULLSTRING) && (!dwDataSize))
		|| ((dwRequiredBlockDataType == AAS_DATATYPE_32BITSINT) && (dwDataSize == 4))
		|| ((dwRequiredBlockDataType == AAS_DATATYPE_NULLARG) && (!dwDataSize))
		|| ((dwRequiredBlockDataType == AAS_DATATYPE_EXTDEDSIZE) && (!dwDataSize))
		|| ((dwRequiredBlockDataType == AAS_DATATYPE_BINARYSTRM) && (dwDataSize))
		|| ((dwRequiredBlockDataType == AAS_DATATYPE_UNICODESTR)  && (dwDataSize))
		|| ((dwRequiredBlockDataType == AAS_DATATYPE_ASCIICHAR) && (dwDataSize)))
		return dwDataSize;
	else
	{
		DEBUG_LOG(D_ERROR, "The DataTypeRequired doesn't match the current block datatype.\r\nExiting now...");
		DoExit(D_ERROR);
		return 0;
	}
}