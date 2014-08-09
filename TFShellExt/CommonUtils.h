
#pragma once

#define MAX_FORMAT 10

/**
	Contains helper functions common between the main dll and provider.
*/
class CommonUtils
{
public:
	static ChemFormat GetFormatFromFileName(TCHAR* fileName)
	{
		LPWSTR ext = ::PathFindExtension(fileName);

		if(TEQUAL(ext, ".mol")) return fmtMOLV2;
		else if(TEQUAL(ext, ".rxn")) return fmtRXNV2;
		else if(TEQUAL(ext, ".smi")) return fmtSMILES;
		else if(TEQUAL(ext, ".smiles")) return fmtSMILES;
		else if(TEQUAL(ext, ".sma")) return fmtSMARTS;
		else if(TEQUAL(ext, ".smarts")) return fmtSMARTS;
		else if(TEQUAL(ext, ".sdf")) return fmtSDF;
		else if(TEQUAL(ext, ".rdf")) return fmtRDF;
		else if(TEQUAL(ext, ".emf")) return fmtEMF;
		else if(TEQUAL(ext, ".cml")) return fmtCML;
		else if(TEQUAL(ext, ".png")) return fmtPNG;
		else if(TEQUAL(ext, ".pdf")) return fmtPDF;
		else if(TEQUAL(ext, ".svg")) return fmtSVG;
		else if(TEQUAL(ext, ".cdxml")) return fmtCDXML;
		else if(TEQUAL(ext, ".inchi")) return fmtINCHI;
		else if(TEQUAL(ext, ".inchik")) return fmtINCHIKEY;
		else if(TEQUAL(ext, ".ct")) return fmtMDLCT;
		else return fmtUnknown;
	}

	static bool GetStrDataFormat(ChemFormat fmt, TCHAR* outBuffer, int bufferLength)
	{
		if(fmt == fmtMOLV2) _tcscpy_s(outBuffer, bufferLength, _T("MOL V2000"));
		else if(fmt == fmtMOLV3) _tcscpy_s(outBuffer, bufferLength, _T("MOL V3000"));
		else if(fmt == fmtRXNV2) _tcscpy_s(outBuffer, bufferLength, _T("RXN V2000"));
		else if(fmt == fmtRXNV3) _tcscpy_s(outBuffer, bufferLength, _T("RXN V3000"));
		else if(fmt == fmtSMILES) _tcscpy_s(outBuffer, bufferLength, _T("SMILES"));
		else if(fmt == fmtSMARTS) _tcscpy_s(outBuffer, bufferLength, _T("SMARTS"));
		else if(fmt == fmtSDF) _tcscpy_s(outBuffer, bufferLength, _T("SDF"));
		else if(fmt == fmtRDF) _tcscpy_s(outBuffer, bufferLength, _T("RDF"));
		else if(fmt == fmtCML) _tcscpy_s(outBuffer, bufferLength, _T("CML"));
		else if(fmt == fmtCDXML) _tcscpy_s(outBuffer, bufferLength, _T("CDXML"));
		else if(fmt == fmtINCHI) _tcscpy_s(outBuffer, bufferLength, _T("INCHI"));
		else if(fmt == fmtINCHIKEY) _tcscpy_s(outBuffer, bufferLength, _T("INCHIKEY"));
		else if(fmt == fmtEMF) _tcscpy_s(outBuffer, bufferLength, _T("EMF"));
		else if(fmt == fmtPNG) _tcscpy_s(outBuffer, bufferLength, _T("PNG"));
		else if(fmt == fmtPDF) _tcscpy_s(outBuffer, bufferLength, _T("PDF"));
		else if(fmt == fmtSVG) _tcscpy_s(outBuffer, bufferLength, _T("SVG"));
		else if(fmt == fmtMDLCT) _tcscpy_s(outBuffer, bufferLength, _T("MDLCT"));
		else return false;

		return true;
	}
};
