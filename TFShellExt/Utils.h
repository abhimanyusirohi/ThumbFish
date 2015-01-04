#pragma once
#include "stdafx.h"
#include "ThumbFishDocument.h"

const TCHAR c_OnlineLink[] = _T("http://abhimanyusirohi.github.io/ThumbFish/");

// Change ONEBASED_MOLV3FILTERINDEX and ONEBASED_RXNV3FILTERINDEX values when either
// the content OR order of items changes in SaveTypes array
const COMDLG_FILTERSPEC c_SaveTypes[] =
{
	{L"Chemical Markup Language (*.cml)", L"*.cml"},
	{L"ChemDraw XML Format (*.cdxml)", L"*.cdxml"},
	{L"Connection Table (*.ct)", L"*.ct"},
    {L"Enhanced Metafile (*.emf)", L"*.emf"},
	{L"MDL Molfile V2000 (*.mol)",	L"*.mol"},
	{L"MDL Molfile V3000 (*.mol)",	L"*.mol"},
	{L"MDL Reaction V2000 (*.rxn)",	L"*.rxn"},
	{L"MDL Reaction V3000 (*.rxn)",	L"*.rxn"},
    {L"Portable Network Graphics (*.png)",	L"*.png"},
	{L"Portable Document Format (*.pdf)",	L"*.pdf"},
    {L"Scalable Vector Graphics (*.svg)", L"*.svg"},
	{L"SMILES (*.smi)", L"*.smi"},
};

#define	ONEBASED_MOLV3FILTERINDEX	6
#define	ONEBASED_RXNV3FILTERINDEX	8

class Utils
{
public:
	Utils(void);
	~Utils(void);

	static int CALLBACK BrowseFolderCallback(HWND hwnd, UINT uMsg, LPARAM lParam, LPARAM lpData);

	// 'DoXXX' functions interact with users by displaying a user interface
	static HRESULT		DoFolderDialog(HWND owner, LPCTSTR title, LPTSTR startFolder, PWSTR folderPath);
	static HRESULT		DoFileSaveDialog(HWND owner, PWSTR filePath, UINT* filterIndex);
	static void			DoSaveStructure(HWND parentWnd, LPBUFFER buffer, LPOPTIONS options);
	static bool			CopyToClipboard(PVOID data, size_t dataLength, int format);
	static void			ConvertAndCopy(LPBUFFER buffer, ChemFormat convertTo, LPOPTIONS options);

	static HRESULT		GetSystemFolder(const KNOWNFOLDERID folderID, TCHAR* outPath);
	static bool			ShellExecuteLink(const TCHAR* link);
	static bool			IsMultiMolFile(TCHAR* fileName);
	static HMENU		CreateCopyMenu(ChemFormat srcFormat, UINT* idStart);
	static YOURS PCHAR	PrettyMDLCT(PCHAR mdlct, int dataLen);
};
