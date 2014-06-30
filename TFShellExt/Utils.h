#pragma once
#include "stdafx.h"
#include "ThumbFishDocument.h"

const TCHAR c_OnlineLink[] = _T("http://abhimanyusirohi.github.io/ThumbFish/");
const COMDLG_FILTERSPEC c_SaveTypes[] =
{
    {L"Portable Network Graphics (*.png)",	L"*.png"},
	{L"Portable Document Format (*.pdf)",	L"*.pdf"},
    {L"Scalable Vector Graphics (*.svg)", L"*.svg"},
    {L"Enhanced Metafile (*.emf)", L"*.emf"},
    {L"ChemDraw XML Format (*.cdxml)", L"*.cdxml"}
};

const enum ConvertFormats { SMILES = 1, InChi, InChiKey, MolV2000, MolV3000, CDXML, EMF };

class Utils
{
public:
	Utils(void);
	~Utils(void);

	// 'DoXXX' functions interact with users by displaying a user interface
	static HRESULT		DoFileSaveDialog(HWND owner, PWSTR filePath);
	static void			DoSaveStructure(HWND parentWnd, LPBUFFER buffer, LPOPTIONS options);
	static bool			CopyToClipboard(const char* data, int dataLength, int format);
	static char*		ConvertStructure(LPBUFFER buffer, ConvertFormats convertTo, LPOPTIONS options);

	static HRESULT		GetSystemFolder(const KNOWNFOLDERID folderID, TCHAR* outPath);
	static bool			ShellExecuteLink(const TCHAR* link);
	static Extension	GetExtension(TCHAR* fileName);
	static bool			IsMultiMolFile(TCHAR* fileName);			
};
