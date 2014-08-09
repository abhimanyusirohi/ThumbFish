#pragma once
#include "stdafx.h"
#include "ThumbFishDocument.h"

const TCHAR c_OnlineLink[] = _T("http://abhimanyusirohi.github.io/ThumbFish/");
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

class Utils
{
public:
	Utils(void);
	~Utils(void);

	// 'DoXXX' functions interact with users by displaying a user interface
	static HRESULT	DoFileSaveDialog(HWND owner, PWSTR filePath);
	static void		DoSaveStructure(HWND parentWnd, LPBUFFER buffer, LPOPTIONS options);
	static bool		CopyToClipboard(const char* data, int dataLength, int format);
	static void		ConvertAndCopy(LPBUFFER buffer, ChemFormat convertTo, LPOPTIONS options);

	static HRESULT	GetSystemFolder(const KNOWNFOLDERID folderID, TCHAR* outPath);
	static bool		ShellExecuteLink(const TCHAR* link);
	static bool		IsMultiMolFile(TCHAR* fileName);
	static HMENU	CreateCopyMenu(ChemFormat srcFormat, UINT* idStart);
};
