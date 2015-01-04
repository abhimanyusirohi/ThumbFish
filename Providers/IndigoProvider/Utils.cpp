#include "StdAfx.h"
#include "Utils.h"


Utils::Utils(void)
{
}


Utils::~Utils(void)
{
}

// Sets the outPath variable with path to the specified system folder
// outPath must be an array of MAX_PATH size
HRESULT Utils::GetSystemFolder(const KNOWNFOLDERID folderID, TCHAR* outPath)
{
	CComPtr<IShellItem> psiFolder;
	LPWSTR wszPath = NULL;
 
	HRESULT hr = SHGetKnownFolderPath (folderID, KF_FLAG_CREATE, NULL, &wszPath); 
	if (SUCCEEDED(hr))
	{
		hr = SHCreateItemFromParsingName(wszPath, NULL, IID_PPV_ARGS(&psiFolder));
		if (SUCCEEDED(hr))
		{
			_tcscpy_s(outPath, MAX_PATH, wszPath);
		}
		CoTaskMemFree (wszPath);
	}

	pantheios::log_NOTICE(_T("Utils::GetSystemFolder> HRESULT="), pantheios::integer(hr));

	return hr;
}

void Utils::DrawErrorBitmap(HDC hDC, LPRECT lpRect)
{
	if(hInstance == NULL) return;

	// load bitmap from resource
	HBITMAP hBitmap = LoadBitmap(hInstance, MAKEINTRESOURCE(IDB_FISH));
	if(hBitmap != NULL)
	{
		BITMAP bm = {0};
		GetObject(hBitmap, sizeof(bm), &bm);
		LONG bmcx = bm.bmWidth;
		LONG bmcy = bm.bmHeight;

		HDC hdcMem = CreateCompatibleDC(hDC);
		HGDIOBJ oldBitmap = SelectObject(hdcMem, hBitmap);

		::FillRect(hDC, lpRect, (HBRUSH)::GetStockObject(WHITE_BRUSH));

		// center the error bitmap in area
		if(!BitBlt(hDC, lpRect->left + ((lpRect->right - lpRect->left) - bmcx)/2, 
			lpRect->top + ((lpRect->bottom - lpRect->top) - bmcy)/2, lpRect->right,
			lpRect->bottom, hdcMem, 0, 0, SRCCOPY))
		{
			pantheios::log_WARNING(_T("DrawErrorBitmap> BitBlt API FAILED. GetLastError="), 
				pantheios::integer(::GetLastError()));
		}

		SelectObject(hdcMem, oldBitmap);
		DeleteDC(hdcMem);
		DeleteObject(hBitmap);
	}
	else
	{
		pantheios::log_WARNING(_T("DrawErrorBitmap> LoadBitmap FAILED."));
	}
}

void Utils::DrawMOLVersionIndicator(HDC hDC, bool v2000)
{
	HBRUSH bgBrush = ::CreateSolidBrush(RGB(224, 224, 224));
	HPEN oldPen = (HPEN)SelectObject(hDC, GetStockObject(NULL_PEN));
	HBRUSH oldBrush = (HBRUSH)SelectObject(hDC, bgBrush);
	HFONT oldFont = (HFONT)SelectObject(hDC, GetStockObject(DEFAULT_GUI_FONT));

	// draw empty rectangle
	::Ellipse(hDC, 8, 8, 30, 30);
	::SetBkMode(hDC, TRANSPARENT);
	::TextOut(hDC, 12, 12, v2000 ? _T("V2") : _T("V3"), 2);

	// revert to old pens and brushes
	SelectObject(hDC, oldFont);
	SelectObject(hDC, oldPen);
	SelectObject(hDC, oldBrush);

	// delete the newly created pen
	DeleteObject(bgBrush);
}

void Utils::DrawRecordCount(HDC hDC, RECT rect, int recordCount)
{
	SIZE szText;
	wchar_t text[10];
	int len = _snwprintf_s(text, 10, 10, L"%d±", recordCount);

	// create a large font to display molecule count
	//HFONT font = CreateFont(20, 0, 0, 0, FW_BOLD, 0, 0, 0, DEFAULT_CHARSET, OUT_OUTLINE_PRECIS, CLIP_DEFAULT_PRECIS, 
	//	ANTIALIASED_QUALITY, FF_DONTCARE, NULL);
	HFONT oldFont = (HFONT)SelectObject(hDC, GetStockObject(DEFAULT_GUI_FONT));

	// get width of text with currently selected font
	GetTextExtentPoint(hDC, text, len, &szText);

	// add margins to text
	szText.cx += 10;
	szText.cy += 5;

	int destWidth = (rect.right - rect.left);
	int destHeight = (rect.bottom - rect.top);
	
	HBRUSH bgBrush = ::CreateSolidBrush(RGB(224, 224, 224));
	HBRUSH oldBrush = (HBRUSH)SelectObject(hDC, bgBrush);

	//HPEN pen = CreatePen(PS_SOLID, 1, RGB(7, 203, 75));	// little darker green than the fill colour
	HPEN oldPen = (HPEN)SelectObject(hDC, GetStockObject(NULL_PEN));

	Rectangle(hDC, destWidth - szText.cx - 5, 8, destWidth - 5, szText.cy + 8);

	SetBkMode(hDC, TRANSPARENT);
	//SetTextColor(hDC, RGB(3, 82, 31));
	TextOut(hDC, destWidth-szText.cx, 10, text, len);

	SelectObject(hDC, oldFont);
	SelectObject(hDC, oldBrush);
	SelectObject(hDC, oldPen);

	//DeleteObject(pen);
	DeleteObject(bgBrush);
	//DeleteObject(font);
}
