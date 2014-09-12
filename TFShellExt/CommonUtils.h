
#pragma once

#include <map>

#define MAX_FORMAT 10
#define MAX_EXTENSION 10
#define MAX_FORMAT_LONG 255
#define FOREACH_FORMATINFO(x) for(int index = 0; index < (sizeof(cFormatInfo)/sizeof(cFormatInfo[0])); index++) { x }

// Parameters required to execute a command on a new thread
typedef struct CommandParams
{
	int		CommandID;	// command id of operation
	LPVOID	Data;		// data required to execute this command

	CommandParams(int id, LPVOID data) : CommandID(id), Data(data) {}
} COMMANDPARAMS, *LPCOMMANDPARAMS;

typedef struct
{
	ChemFormat format;
	TCHAR extension[MAX_EXTENSION];
	TCHAR formatTag[MAX_FORMAT];
	TCHAR formatTagLong[MAX_FORMAT_LONG];
} FORMATINFO, *LPFORMATINFO;

const FORMATINFO cFormatInfo[] = {
	{ fmtMOLV2, _T(".mol"), _T("MOL V2000"), _T("MDL MOL V2000") },
	{ fmtMOLV3, _T(".mol"), _T("MOL V3000"), _T("MDL MOL V3000") },
	{ fmtRXNV2, _T(".rxn"), _T("RXN V2000"), _T("MDL RXN V2000") },
	{ fmtRXNV2, _T(".rxn"), _T("RXN V3000"), _T("MDL RXN V3000") },
	{ fmtCDXML, _T(".cdxml"), _T("CDXML"), _T("ChemDraw XML") },
	{ fmtSMILES, _T(".smi"), _T("SMILES"), _T("SMILES") },
	{ fmtSMILES, _T(".smiles"), _T("SMILES"), _T("SMILES") },
	{ fmtSMARTS, _T(".sma"), _T("SMARTS"), _T("SMARTS") },
	{ fmtSMARTS, _T(".smarts"), _T("SMARTS"), _T("SMARTS") },
	{ fmtSDF, _T(".sdf"), _T("SDF"), _T("Structure Data File") },
	{ fmtRDF, _T(".rdf"), _T("RDF"), _T("Reaction Data File") },
	{ fmtEMF, _T(".emf"), _T("EMF"), _T("Enhanced Meta File") },
	{ fmtCML, _T(".cml"), _T("CML"), _T("Chemical Markup Language") },
	{ fmtPNG, _T(".png"), _T("PNG"), _T("Portable Network Graphics") },
	{ fmtPDF, _T(".pdf"), _T("PDF"), _T("Portable Document Format") },
	{ fmtSVG, _T(".svg"), _T("SVG"), _T("Scalable Vector Graphics") },
	{ fmtINCHI, _T(".inchi"), _T("INCHI"), _T("InChi") },
	{ fmtINCHIKEY, _T(".inchik"), _T("INCHIKEY"), _T("InChi Key") },
	{ fmtMDLCT, _T(".ct"), _T("MDLCT"), _T("MDL Connection Table") }
};

/**
	Contains helper functions common between the main dll and provider.
*/
class CommonUtils
{
public:
	static ChemFormat GetFormatFromFileName(TCHAR* fileName)
	{
		LPWSTR ext = ::PathFindExtension(fileName);
		FOREACH_FORMATINFO(if(TEQUAL(ext, cFormatInfo[index].extension)) return cFormatInfo[index].format;)
		return fmtUnknown;
	}

	static bool GetFormatString(ChemFormat fmt, TCHAR* outBuffer, int bufferLength)
	{
		FOREACH_FORMATINFO(if(fmt == cFormatInfo[index].format) 
			return (_tcscpy_s(outBuffer, bufferLength, cFormatInfo[index].formatTag) == 0);)
		return false;
	}

	static bool GetFormatExtension(ChemFormat fmt, TCHAR* outBuffer, int bufferLength)
	{
		FOREACH_FORMATINFO(if(fmt == cFormatInfo[index].format)
				return (_tcscpy_s(outBuffer, bufferLength, cFormatInfo[index].extension) == 0);)
		return false;
	}

	static bool IsMultiMolFormat(ChemFormat format)
	{
		return ((format == fmtSDF) || (format == fmtRDF) || (format == fmtCML) || (format == fmtSMILES));
	}

	static bool IsReadableFormat(ChemFormat format)
	{
		return ((format == fmtMOLV2) || (format == fmtMOLV3) || (format == fmtRXNV2) || (format == fmtRXNV3) 
			|| (format == fmtSDF) || (format == fmtRDF) || (format == fmtCML) || (format == fmtSMILES) || (format == fmtSMARTS));
	}
};
