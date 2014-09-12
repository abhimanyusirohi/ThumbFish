#pragma once

class ExtractParams
{
public:
	ChemFormat			exportFormat;			// user option
	bool				overwriteFiles;			// user option
	int					extractMolCount;		// user option
	TCHAR				fileFormat[50];			// user option
	TCHAR				folderPath[MAX_PATH];	// user option
	TCHAR				sourceFile[MAX_PATH];
	ChemFormat			sourceFormat;

	LPVOID				caller;					// CExtractDlg class ref
	ProgressCallback	callback;				// callback method for progress reporting

public:
	ExtractParams() : extractMolCount(-1), overwriteFiles(false), callback(NULL)
	{
		exportFormat = fmtCDXML;
		_tcscpy_s(fileFormat, 50, _T("Structure%d"));
		_tcscpy_s(folderPath, MAX_PATH, _T("Select Folder"));
	}

	void GetStrMolCount(TCHAR* outBuffer, int bufferLength)
	{
		if(extractMolCount == -1)
			_tcscpy_s(outBuffer, bufferLength, _T("ALL"));
		else
			_snwprintf_s(outBuffer, bufferLength, bufferLength, _T("%d"), extractMolCount);
	}

	void GetStrOverwriteFiles(TCHAR* outBuffer, int bufferLength)
	{
		_tcscpy_s(outBuffer, bufferLength, overwriteFiles ? _T("OVERWRITTEN") : _T("SKIPPED"));
	}
};

typedef ExtractParams* LPEXTRACTPARAMS;
