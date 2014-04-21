#include "StdAfx.h"
#include "PreviewCtrl.h"

#define ONE_KB	1024
#define FOUR_KB	4*ONE_KB

void CPreviewCtrl::DoPaint(HDC hdc)
{
	//pantheios::log_INFORMATIONAL(_T("CPreviewCtrl::DoPaint called"));

	//if(m_pDocument == NULL) return;

	OPTIONS options;
	ThumbFishDocument *pDoc = (ThumbFishDocument*)m_pDocument;

	if((pDrawFunc != NULL) && (pDoc != NULL) && (pDoc->m_Buffer.DataLength > 0))
	{
		//TODO: Remove this DEBUG ONLY
		//if(pDoc->m_Buffer.DataLength < 100) return;

		HWND hPict = GetDlgItem(IDC_PICT);
		if(hPict != NULL)
		{
			RECT rect;

			// get client area of picture box
			::GetClientRect(hPict, &rect);

			// draw thumbnail in client area and get molecule properties
			if(!pDrawFunc(::GetDC(hPict), &rect, &pDoc->m_Buffer, &options))
				pantheios::log_WARNING(_T("CPreviewCtrl::DoPaint> Draw returned FALSE. TODO: Draw something here."));

			// get handle of property ListView
			HWND hWndList = ::GetDlgItem(m_hWnd, IDC_PROPLIST);

			// avoid refilling properties
			if(ListView_GetItemCount(hWndList) == 0)
			{
				TCHAR** props = NULL;
				int propCount = pGetPropsFunc(&pDoc->m_Buffer, &props, &options);

				if((props != NULL) && (propCount > 0))
				{
					// insert property values into list view control
					for(int i = 0; i < propCount; i++)
					{
						InsertLVItem(hWndList, i, props[2*i], props[2*i + 1]);
					}

					// auto size to content - last column
					ListView_SetColumnWidth(hWndList, 1, LVSCW_AUTOSIZE_USEHEADER);
					delete[] props;
				}
				else
				{
					// no properties to display, hide listview and show static message
					::ShowWindow(hWndList, SW_HIDE);
					::ShowWindow(::GetDlgItem(m_hWnd, IDC_NOPREVIEWTEXT), SW_SHOW);
				}
			}
		}
	}
	else
	{
		//pantheios::log_ERROR(_T("CPreviewCtrl::DoPaint> Unable to draw Preview"));/*, 
		//	_T("pDrawThumbnailFunc="), (pDrawFunc == NULL),
		//	_T("pDoc="), (pDoc == NULL), 
		//	_T("BufferLength="), (pDoc == NULL) ? 0 : pDoc->m_Buffer.DataLength);*/
	}
}

LRESULT CPreviewCtrl::OnInitDialog(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	RECT parentRect;

	// autosize the control window to match parent
	::GetClientRect(GetParent().m_hWnd, &parentRect);
	SetWindowPos(NULL, &parentRect, 0);

	// autosize all child controls to match container
	AutoSizeControls(parentRect);

	HWND hWndList = ::GetDlgItem(m_hWnd, IDC_PROPLIST);
	if(hWndList != NULL)
	{
		// Initialize list view columns
		LVCOLUMN column;
		column.mask = LVCF_FMT | LVCF_WIDTH | LVCF_TEXT | LVCF_SUBITEM;
		column.iSubItem = 0;
		column.pszText = _T("Property");
		column.cx = 140;
		column.fmt = LVCFMT_LEFT;
		ListView_InsertColumn(hWndList, 0, &column);

		// second column
		column.iSubItem = 1;
		column.pszText = _T("Value");
		column.cx = 150;
		ListView_InsertColumn(hWndList, 1, &column);

		// set full row select and gridlines
		ListView_SetExtendedListViewStyle(hWndList, LVS_EX_FULLROWSELECT | LVS_EX_GRIDLINES);
	}

	bHandled = TRUE;
	return 1;
}

void CPreviewCtrl::SetRect(const RECT* prc, BOOL bRedraw)
{
	DWORD dwFlags = SWP_NOZORDER;
	if (!bRedraw)
	{
		dwFlags |= SWP_NOREDRAW;
	}

	SetWindowPos(NULL, prc, dwFlags);
	AutoSizeControls(const_cast<RECT&>(*prc));
}

LRESULT CPreviewCtrl::OnContextMenu(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	HWND hWndList = ::GetDlgItem(m_hWnd, IDC_PROPLIST);

	// get selected item index in list
	int selected = ListView_GetNextItem(hWndList, -1, LVNI_SELECTED);
	if(selected != -1)
	{
		HMENU hPopupMenu = LoadMenu(_AtlBaseModule.m_hInst, MAKEINTRESOURCE(IDR_PROPLISTMENU));
		hPopupMenu = GetSubMenu(hPopupMenu, 0);
			
		//TODO: Check bitmap leaks
		// set bitmaps for menu items
		//HBITMAP hbmCopy = LoadBitmap(_AtlBaseModule.m_hInst, MAKEINTRESOURCE(IDB_BITMAPCOPY));
		//SetMenuItemBitmaps(hPopupMenu, 0, MF_BYPOSITION, hbmCopy, hbmCopy);

		TrackPopupMenu(hPopupMenu, TPM_TOPALIGN | TPM_LEFTALIGN, GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam), 0, m_hWnd, NULL);
	}

	return 0;
}

void CPreviewCtrl::InsertLVItem(HWND hWndList, int index, TCHAR* item, int subitem)
{
	TCHAR buffer[20];
	if(swprintf(buffer, 20, _T("%d"), subitem) > 0)
		InsertLVItem(hWndList, index, item, buffer);
}

void CPreviewCtrl::InsertLVItem(HWND hWndList, int index, TCHAR* item, double subitem)
{
	TCHAR buffer[20];
	if(swprintf(buffer, 20, _T("%f"), subitem) > 0)
		InsertLVItem(hWndList, index, item, buffer);
}

void CPreviewCtrl::InsertLVItem(HWND hWndList, int index, TCHAR* item, TCHAR* subitem)
{
	LVITEM lvi;
	lvi.mask = LVIF_TEXT | LVIF_IMAGE | LVIF_STATE;
	lvi.stateMask = 0;
	lvi.state = 0;

	// first value
	lvi.iItem = index;
	lvi.iSubItem = 0;
	lvi.pszText = item;
	ListView_InsertItem(hWndList, &lvi);

	// second value
	lvi.iSubItem = 1;
	lvi.pszText = subitem;
	ListView_SetItem(hWndList, &lvi);
}

void CPreviewCtrl::AutoSizeControls(RECT& parentRect)
{
	RECT pictRect, msgRect;
	int height = (parentRect.bottom - parentRect.top);

	// autosize picture box control to 40% of total height 
	HWND hWndPict = ::GetDlgItem(m_hWnd, IDC_PICT);
	::SetWindowPos(hWndPict, NULL, 0, 0, 
		(parentRect.right - parentRect.left) - 10, 
		((40 * height) / 100), 
		SWP_NOZORDER | SWP_NOMOVE);
	::GetClientRect(hWndPict, &pictRect);

	// autosize list view control to remaining 60% of total height
	HWND hWndList = ::GetDlgItem(m_hWnd, IDC_PROPLIST);
	::SetWindowPos(hWndList, NULL, 0, pictRect.bottom + 10, 
		(parentRect.right - parentRect.left) - 10, 
		((60 * height) / 100),
		SWP_NOZORDER);

	// align the nopreview available message
	HWND hWndMsg = ::GetDlgItem(m_hWnd, IDC_NOPREVIEWTEXT);
	::GetClientRect(hWndMsg, &msgRect);
	::SetWindowPos(hWndMsg, NULL, 
		((pictRect.right - pictRect.left) - (msgRect.right - msgRect.left)) / 2, 
		pictRect.bottom + 50, 0, 0, SWP_NOZORDER | SWP_NOSIZE);
}

bool CPreviewCtrl::CopyToClipboard(const TCHAR* text)
{
	size_t size = _tcslen(text);
	if(size == 0) return false;

	// Allocate string
	HGLOBAL hdst = GlobalAlloc(GMEM_MOVEABLE | GMEM_DDESHARE, (size + 1) * sizeof(WCHAR));
	LPWSTR dst = (LPWSTR)GlobalLock(hdst);
	memcpy(dst, text, size * sizeof(WCHAR));
	dst[size] = NULL;
	GlobalUnlock(hdst);

	// Set clipboard data
	if (OpenClipboard())
	{
		EmptyClipboard();
		if (SetClipboardData(CF_UNICODETEXT, hdst))
		{
			CloseClipboard();
		}
		else
		{
			pantheios::log_ERROR(_T("CPreviewCtrl::CopyToClipboard> SetClipboardData FAILED. GetLastError="));
						//pantheios::integer(GetLastError()));
			return false;
		}
	}
	else
	{
		pantheios::log_ERROR(_T("CPreviewCtrl::CopyToClipboard> OpenClipboard FAILED. GetLastError="),
					pantheios::integer(GetLastError()));
		return false;
	}

	return true;
}

LRESULT CPreviewCtrl::OnOptionsCopy(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
	HWND hWndList = ::GetDlgItem(m_hWnd, IDC_PROPLIST);

	// get selected item index in list
	int selected = ListView_GetNextItem(hWndList, -1, LVNI_SELECTED);
	if(selected != -1)
	{
		TCHAR itemText[ONE_KB];

		// get selected item text and copy it
		ListView_GetItemText(hWndList, selected, 1, itemText, ONE_KB);
		CopyToClipboard(itemText);
	}

	bHandled = TRUE;
	return 0;
}

LRESULT CPreviewCtrl::OnOptionsCopyAll(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
	HWND hWndList = ::GetDlgItem(m_hWnd, IDC_PROPLIST);

	TCHAR largeText[FOUR_KB];
	memset(largeText, 0, FOUR_KB);

	int count = ListView_GetItemCount(hWndList);
	for(int index = 0; index < count; index++)
	{
		TCHAR propText[255], valueText[ONE_KB];

		// get both column values
		ListView_GetItemText(hWndList, index, 0, propText, 255);
		ListView_GetItemText(hWndList, index, 1, valueText, ONE_KB);

		// append "prop:value" to the large array
		_tcscat_s(largeText, FOUR_KB, propText);
		_tcscat_s(largeText, FOUR_KB, _T(": "));
		_tcscat_s(largeText, FOUR_KB, valueText);
		_tcscat_s(largeText, FOUR_KB, _T("\r\n"));
	}

	// copy large text to clipboard
	CopyToClipboard(largeText);

	bHandled = TRUE;
	return 0;
}

LRESULT CPreviewCtrl::OnOptionsAbout(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	TCHAR buffer[1024];
	if(LoadString(_AtlBaseModule.m_hInst, IDS_OPTIONS_ABOUT, buffer, 1024) > 0)
		::MessageBox(NULL, buffer,	_T("About ThumbFish"), MB_OK | MB_ICONINFORMATION);

	return 0;
}

LRESULT CPreviewCtrl::OnOptionsSaveStructure(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	WCHAR filePath[MAX_PATH];

	//TODO: for some reason, setting owner to our preview control handle suppresses the dialog
	// passing NULL as owner handle has the side effect of showing the Save dialog in taskbar
	if(Utils::DoFileSaveDialog(NULL, filePath) == S_OK)
	{
		OPTIONS options;
		ThumbFishDocument *pDoc = (ThumbFishDocument*)m_pDocument;

		LPWSTR ext = ::PathFindExtensionW(filePath);
		sprintf_s(options.RenderOutputExtension, 6, "%ws", (ext[0] == '.') ? ext + 1 : ext);
		options.RenderImageWidth = options.RenderImageHeight = 300;

		// convert existing structure into bytes in specified format
		char* data = pConvertFunc(&pDoc->m_Buffer, &options);

		if(data != NULL)
		{
			HANDLE hFile; 
			DWORD dwBytesToWrite = (DWORD)options.OutBufferSize;
			DWORD dwBytesWritten = 0;
			BOOL bErrorFlag = FALSE;

			// create the file and then write data to it
			hFile = CreateFile(filePath, GENERIC_WRITE, 0, NULL, CREATE_NEW, FILE_ATTRIBUTE_NORMAL,	NULL);
			if (hFile != INVALID_HANDLE_VALUE)
			{
				bErrorFlag = WriteFile(hFile, data, dwBytesToWrite, &dwBytesWritten, NULL);
				if ((FALSE == bErrorFlag) || (dwBytesWritten != dwBytesToWrite))
				{
					::MessageBox(m_hWnd, _T("Unable to write data to the file"), _T("File Write Error"), MB_OK | MB_ICONERROR);
				}
			}
			else
			{
				::MessageBox(m_hWnd, _T("Unable to create file"), _T("File Write Error"), MB_OK | MB_ICONERROR);
			}

			CloseHandle(hFile);
		}
	}

	return 0;
}

LRESULT CPreviewCtrl::OnOptionsCopyStructureAs(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
	return S_OK;
}
