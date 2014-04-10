/**************************************************
* SysvolCrawler - AASParser.h
* AUTHOR: Luc Delsalle	
*
* Parsing engine for .aas file
* (Application Advertise Script)
*	
* ANSSI/COSSI/DTO/BAI - 2014
***************************************************/

#ifndef __AAS_PARSER_H__
#define __AAS_PARSER_H__

#include "Common.h"

//************** <PARSER DEFINITION> *********************
#define AAS_PARSER_NAME						TEXT("AAS parser")
#define AAS_MATCHING_FILE_REGEXP			TEXT("*.aas")
#define AAS_MATCHING_FOLDER_REGEXP			TEXT("[NON SUPPORTED]")
//************** </PARSER DEFINITION> ********************

// AAS file signature
#define AAS_FILE_SIGNATURE					1397708873
#define AAS_ARG_NUMBER_MAX					8048
#define AAS_SRC_LST_PUB_MAX_DISK			256

// Opcode number for AAS data block
#define AAS_OPCODE_HEADER					2
#define AAS_OPCODE_PRODUCTINFO				4
#define AAS_OPCODE_SRCLISTPUB				9
#define AAS_OPCODE_PRODUCTPUB				16
#define AAS_OPCODE_END						3

// Define argument type for AAS data bloc
typedef WORD								AAS_BLOCK_DATATYPE;
#define AAS_DATATYPE_NULLSTRING				0x0000
#define AAS_DATATYPE_32BITSINT				0x4000
#define AAS_DATATYPE_NULLARG				0x8000
#define AAS_DATATYPE_EXTDEDSIZE				0xc000
#define AAS_DATATYPE_ASCIICHAR				0x0000
#define AAS_DATATYPE_BINARYSTRM				0x8000
#define AAS_DATATYPE_UNICODESTR				0xc000

// Define expected size AAS for data block
#define AAS_BLOCK_HEADER_SIZE				0x24
#define AAS_BLOCK_PRODUCT_INFO_SIZE			0x4c
#define AAS_BLOCK_SOURCE_LIST_PUBLISH_SIZE	0x1424
#define AAS_BLOCK_PRODUCT_PUBLISH_SIZE		0x4
#define AAS_BLOCK_END_SIZE					0xc

//******* <STORE DATA FOR FILE AAS> ******
typedef struct _AAS_DATA_UNKNOWN
{
	AAS_BLOCK_DATATYPE						wDataType;
	DWORD									wDataLen;
	PBYTE									pbData;
} AAS_DATA_UNKNOWN, *PAAS_DATA_UNKNOWN;

typedef struct _AAS_BLOCK_UNKNOWN
{
	BYTE									bOpcodeNumber;
	BYTE									bArgumentNumber;

	AAS_DATA_UNKNOWN						sDataUnkwnown[AAS_ARG_NUMBER_MAX];
} AAS_BLOCK_UNKNOWN, *PAAS_BLOCK_UNKNOWN;

// Store HEADER data block
typedef struct _AAS_BLOCK_HEADER
{
	PDWORD									pdwSignature;
	PDWORD									pdwVersion;
	PDWORD									pdwDosTimeStamp;

	PLCID									pdwLangID;
	PDWORD									pdwPlatform;

	PDWORD									pdwScriptType;
	PDWORD									pdwScriptMajorVersion;
	PDWORD									pdwScriptMinorVersion;
	PDWORD									pdwScriptAttributes;

} AAS_BLOCK_HEADER, *PAAS_BLOCK_HEADER;

// Store PRODUCT_INFO
typedef struct _AAS_BLOCK_PRODUCT_INFO
{
	PWCHAR									pwProductKey;

	BOOL									isProductNameUNICODE;
	PWCHAR									pwProductName;
	
	BOOL									isPackageNameUNICODE;
	PWCHAR									pwPackageName;
		
	PLCID									pdwLanguage;
	PDWORD									pdwVersion;
	PDWORD									pdwAssignment;
	PDWORD									pdwObsoleteArg;

	PWCHAR									pwProductIcon;

	BOOL									isPackageMediaPathUNICODE;
	PWCHAR									pwPackageMediaPath;

	PWCHAR									pwPackageCode;

	PBYTE									pbNullArgument1;
	PBYTE									pbNullArgument2;

	PDWORD									pdwInstanceType;
	PDWORD									pdwLUASetting;
	PDWORD									pdwRemoteURTInstalls;
	PDWORD									pdwProductDeploymentFlags;
} AAS_BLOCK_PRODUCT_INFO, *PAAS_BLOCK_PRODUCT_INFO;

// Store SOURCE_LIST_PUBLISH data block
typedef struct _AAS_BLOCK_SOURCE_LIST_PUBLISH_DISK
{
	PDWORD									pdwDiskId;

	BOOL									isVolumeNameUNICODE;
	PWCHAR									pwVolumeName;

	BOOL									isDiskPromptUNICODE;
	PWCHAR									pwDiskPrompt;
} AAS_BLOCK_SOURCE_LIST_PUBLISH_DISK, *PAAS_BLOCK_SOURCE_LIST_PUBLISH_DISK;

typedef struct _AAS_BLOCK_SOURCE_LIST_PUBLISH
{
	PWCHAR									pwPatchCode;
	PWCHAR									pwPatchPackageName;

	BOOL									isDiskPromptTemplateUNICODE;
	PWCHAR									pwDiskPromptTemplate;

	BOOL									isPackagePathUNICODE;
	PWCHAR									pwPackagePath;

	PDWORD									pdwNumberOfDisks;
	AAS_BLOCK_SOURCE_LIST_PUBLISH_DISK		sDisks[AAS_SRC_LST_PUB_MAX_DISK];

	BOOL									isLaunchPathUNICODE;
	PWCHAR									pwLaunchPath;

} AAS_BLOCK_SOURCE_LIST_PUBLISH, *PAAS_BLOCK_SOURCE_LIST_PUBLISH;

// Store PRODUCT_PUBLISH data block
typedef struct _AAS_BLOCK_PRODUCT_PUBLISH
{
	PWCHAR									pwProductPublish;
} AAS_BLOCK_PRODUCT_PUBLISH, *PAAS_BLOCK_PRODUCT_PUBLISH;

// Store END data block
typedef struct _AAS_BLOCK_END
{
	PDWORD									pdwChecksum;
	PDWORD									pdwProgressTotalHDWord;
	PDWORD									pdwProgressTotalLDWord;
} AAS_BLOCK_END, *PAAS_BLOCK_END;

// Gather AAS data
typedef struct _AAS_FILE_DATA
{
	PWCHAR									tFilePath;

	PAAS_BLOCK_HEADER						pAasHeader;
	PAAS_BLOCK_PRODUCT_INFO					pAasProductInfo;
	PAAS_BLOCK_SOURCE_LIST_PUBLISH			pAasSourceListPublish;
	PAAS_BLOCK_PRODUCT_PUBLISH				pAasProductPublish;
	PAAS_BLOCK_END							pAasEnd;

	DWORD									dwNumberOfUnknwownBlock;
	PAAS_BLOCK_UNKNOWN						sBlockUnkwnown[AAS_ARG_NUMBER_MAX];
} AAS_FILE_DATA, *PAAS_FILE_DATA;

//****** </STORE DATA FOR FILE AAS> ******

// Forward declaration for printers
extern BOOL PrintData(_In_ PAAS_FILE_DATA pAasData);
extern BOOL PrintAasDataHeader(_In_ PTCHAR tFilePath);
extern BOOL PrintAasDataFooter(_In_ PTCHAR tFilePath);

// Parser registration
VOID RegisterAasParser(_Inout_ PPARSER_IDENTIFIER *pParserID);
// Entry point for AAS file
BOOL ParseAasFile(_In_ PTCHAR tFilePath);
// Free AAS metastructure
BOOL FreeAasFileData(_Inout_ PAAS_FILE_DATA pAasData);

BOOL DispatchAASFile(_Inout_ PAAS_FILE_DATA pAasFileData, _In_ PBYTE pbRawData, _In_ PDWORD pdwIndex, _In_ DWORD dwRawDataSize);
PAAS_BLOCK_HEADER FillAasHeader(_In_ PBYTE pbRawData, _In_ PDWORD pdwIndex, _In_ DWORD dwRawDataSize);
PAAS_BLOCK_PRODUCT_INFO FillAasProductInfo(_In_ PBYTE pbRawData, _In_ PDWORD pdwIndex, _In_ DWORD dwRawDataSize);
PAAS_BLOCK_SOURCE_LIST_PUBLISH FillAasSourceListPublish(_In_ PBYTE pbRawData, _In_ PDWORD pdwIndex, _In_ DWORD dwRawDataSize);
PAAS_BLOCK_PRODUCT_PUBLISH FillAasProductPublish(_In_ PBYTE pbRawData, _In_ PDWORD pdwIndex, _In_ DWORD dwRawDataSize);
PAAS_BLOCK_END FillAasEnd(_In_ PBYTE pbRawData, _In_ PDWORD pdwIndex, _In_ DWORD dwRawDataSize);
PAAS_BLOCK_UNKNOWN FillAasUnknownBlock(_In_ PBYTE pbRawData, _In_ PDWORD pdwIndex, _In_ DWORD dwRawDataSize);

BOOL FreeAasHeaderBlock(_Inout_ PAAS_BLOCK_HEADER pAasHeaderBlock);
BOOL FreeAasProductInfoBlock(_Inout_ PAAS_BLOCK_PRODUCT_INFO pAasProductInfoBlock);
BOOL FreeAasSourceListPublishBlock(_Inout_ PAAS_BLOCK_SOURCE_LIST_PUBLISH pAasSourceListPublishBlock);
BOOL FreeAasProductPublishBlock(_Inout_ PAAS_BLOCK_PRODUCT_PUBLISH pAasProductPublishBlock);
BOOL FreeAasEndBlock(_Inout_ PAAS_BLOCK_END pAasEndBlock);
BOOL FreeAasUnknownBlock(_Inout_ PAAS_BLOCK_UNKNOWN pAasUnkwnownBlock);

DWORD GetDataSize(_In_ PBYTE pbRawData, _In_ PDWORD pdwIndex, _In_ DWORD dwRawDataSize);
VOID SetDataAttributes(_Inout_ PVOID *pvAttribute, _In_ AAS_BLOCK_DATATYPE wDataType, _In_ PBYTE pbRawData, _In_ PDWORD pdwIndex, _In_ DWORD dwDatalen, _In_ DWORD dwRealDataType);
DWORD VerifyDataTypeAndGetDataSize(_In_ PBYTE pbRawData, _In_ PDWORD pdwIndex, _In_ DWORD dwRawDataSize, _In_ AAS_BLOCK_DATATYPE dwRequiredBlockDataType);

#endif