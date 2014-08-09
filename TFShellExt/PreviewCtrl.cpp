#include "StdAfx.h"
#include "PreviewCtrl.h"
#include "AboutDlg.h"

#define ONE_KB	1024
#define FOUR_KB	4*ONE_KB

CPreviewCtrl::CPreviewCtrl() : m_previewDrawn(false), m_propsGenerated(false), 
	m_ttipWarning(NULL), m_staticWarning(NULL)
{
	pantheios::log_NOTICE(_T("CPreviewCtrl::CPreviewCtrl> Called."));
	m_pDocument = NULL;
}

void CPreviewCtrl::DoPaint(HDC hdc)
{
	pantheios::log_NOTICE(_T("CPreviewCtrl::DoPaint> Called"));
	
	OPTIONS options;
	ThumbFishDocument *pDoc = (ThumbFishDocument*)m_pDocument;

	if(pDoc)
	{
		if(pDrawFunc)
		{
			if(pDoc->m_Buffer.DataLength <= 0)
			{
				pantheios::log_NOTICE(_T("CPreviewCtrl::DoPaint> Data is ZERO length. No preview will be generated."));
				return;
			}

			HWND hPict = GetDlgItem(IDC_PICT);
			if(hPict != NULL)
			{
				RECT rect;

				// get client area of picture box
				::GetClientRect(hPict, &rect);

				// draw thumbnail in client area and get molecule properties
				m_previewDrawn = pDrawFunc(::GetDC(hPict), rect, &pDoc->m_Buffer, &options);

				if(m_previewDrawn)
					SetChemicalWarnings(&options);
				else
					pantheios::log_NOTICE(_T("CPreviewCtrl::DoPaint> Draw returned FALSE."));
			}
		}

		FillProperties(&pDoc->m_Buffer);
	}
	else
	{
		pantheios::log_ERROR(_T("CPreviewCtrl::DoPaint> Unable to draw Preview"), 
			_T("pDrawThumbnailFunc IsNULL="), pantheios::integer(pDrawFunc == NULL),
				_T("pDoc IsNULL="), pantheios::integer(pDoc == NULL));
	}
}

LRESULT CPreviewCtrl::OnInitDialog(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	RECT parentRect;
	INITCOMMONCONTROLSEX icex;

	m_previewDrawn = m_propsGenerated = false;

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

    // Ensure that the common control DLL is loaded. 
    icex.dwSize = sizeof(INITCOMMONCONTROLSEX);
    icex.dwICC  = ICC_BAR_CLASSES;	// loads tooltip controls
    InitCommonControlsEx(&icex);

	// create a simple static control to display warning icon inside preview window
	m_staticWarning = CreateWindowEx(WS_EX_TOPMOST | WS_EX_TRANSPARENT, _T("static"), _T(""), WS_CHILD | SS_ICON | SS_NOTIFY,
		10, 10, 60, 60, m_hWnd, reinterpret_cast<HMENU>(static_cast<INT_PTR>(IDC_CHEMWARNINGS)),
							_AtlBaseModule.m_hInst, 0);

	// set warning icon in the static control
	HICON hWarningIcon = (HICON)LoadImage(_AtlBaseModule.m_hInst, MAKEINTRESOURCE(IDI_WARN), IMAGE_ICON, 16, 16, NULL);
	if(hWarningIcon) SendMessage(m_staticWarning, STM_SETIMAGE, (WPARAM)IMAGE_ICON, (LPARAM)hWarningIcon);

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
	HWND targetWnd = (HWND) wParam;
	ThumbFishDocument *pDoc = (ThumbFishDocument*)m_pDocument;
	
	bool multiMolFile = Utils::IsMultiMolFile(pDoc->m_Buffer.FileName);
	ChemFormat format = CommonUtils::GetFormatFromFileName(pDoc->m_Buffer.FileName);

	HMENU hPopupMenu = LoadMenu(_AtlBaseModule.m_hInst, MAKEINTRESOURCE(IDR_PROPLISTMENU));
	hPopupMenu = GetSubMenu(hPopupMenu, 0);

	// set bitmaps for menu items
	HBITMAP hbmSave = LoadBitmap(_AtlBaseModule.m_hInst, MAKEINTRESOURCE(IDB_SAVE));
	SetMenuItemBitmaps(hPopupMenu, 0, MF_BYPOSITION, hbmSave, hbmSave);

	HBITMAP hbmCopy = LoadBitmap(_AtlBaseModule.m_hInst, MAKEINTRESOURCE(IDB_COPY));
	SetMenuItemBitmaps(hPopupMenu, 1, MF_BYPOSITION, hbmCopy, hbmCopy);

	if(pDoc != NULL)
	{
		::EnableMenuItem(hPopupMenu, ID_OPTIONS_SAVESTRUCTURE, MF_BYCOMMAND | ((m_previewDrawn && !multiMolFile) ? MF_ENABLED : MF_DISABLED));
		::EnableMenuItem(hPopupMenu, ID_OPTIONS_COPYPROPERTIES, MF_BYCOMMAND | (m_propsGenerated ? MF_ENABLED : MF_DISABLED));

		UINT startId = ID_COPYSTRUCTUREAS_CDXML;
		HMENU hCopyAsMenu = Utils::CreateCopyMenu(format, &startId);

		MENUITEMINFO miiCopyAs = { sizeof(MENUITEMINFO) };
		miiCopyAs.fMask = MIIM_SUBMENU | MIIM_STRING | MIIM_ID | MIIM_BITMAP | MIIM_STATE;
		miiCopyAs.wID = ID_COPYSTRUCTUREAS;
		miiCopyAs.hSubMenu = hCopyAsMenu;
		miiCopyAs.dwTypeData = _T("&Copy Structure As");
		miiCopyAs.hbmpItem = LoadBitmap(_AtlBaseModule.m_hInst, MAKEINTRESOURCE(IDB_COPY));
		miiCopyAs.fState = (!multiMolFile) ? MFS_ENABLED : MFS_DISABLED;
		InsertMenuItem (hPopupMenu, 2, TRUE, &miiCopyAs);
	}

	TrackPopupMenu(hPopupMenu, TPM_TOPALIGN | TPM_LEFTALIGN, GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam), 0, m_hWnd, NULL);

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

void CPreviewCtrl::FillProperties(LPBUFFER buffer)
{
	OPTIONS options;

	// get handle of property ListView
	HWND hWndList = ::GetDlgItem(m_hWnd, IDC_PROPLIST);
	if(ListView_GetItemCount(hWndList) == 0)
	{
		TCHAR** props = NULL;
		int propCount = pGetPropsFunc(buffer, &props, &options, false);
		m_propsGenerated = ((props != NULL) && (propCount > 0));

		if(m_propsGenerated)
		{
			pantheios::log_NOTICE(_T("CPreviewCtrl::DoPaint> Properties generated="), pantheios::integer(propCount));

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

			pantheios::log_NOTICE(_T("CPreviewCtrl::DoPaint> No properties generated."));
		}
	}
}

void CPreviewCtrl::SetChemicalWarnings(LPOPTIONS options)
{
	bool hasWarnings = false;
	wchar_t largeText[ONE_KB];

	// corresponds to bad valence warning
	if(options->OutWarning1)
	{
		swprintf(largeText, ONE_KB, _T("Warning: %hs"), options->OutWarning1);
		hasWarnings = true;
	}

	// corresponds to AmbiguousH warning
	if(options->OutWarning2)
	{
		swprintf(largeText, ONE_KB, _T("%s\r\nWarning: %hs"), options->OutWarning2);
		hasWarnings = true;
	}

	// warning icon on preview window is ONLY shown when there are warnings
	::ShowWindow(m_staticWarning, hasWarnings ? SW_SHOW : SW_HIDE);

	// if we have warnings && the tooltip control is NOT already created
	if(hasWarnings && !m_ttipWarning)
		CreateWarningControls(largeText);
}

LRESULT CPreviewCtrl::OnOptionsCopyAll(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
	HWND hWndList = ::GetDlgItem(m_hWnd, IDC_PROPLIST);

	wchar_t largeText[FOUR_KB];
	memset(largeText, 0, FOUR_KB);

	int count = ListView_GetItemCount(hWndList);
	for(int index = 0; index < count; index++)
	{
		wchar_t propText[255], valueText[ONE_KB];

		// get both column values
		ListView_GetItemText(hWndList, index, 0, propText, 255);
		ListView_GetItemText(hWndList, index, 1, valueText, ONE_KB);

		// append "prop:value" to the large array
		_tcscat_s(largeText, FOUR_KB, propText);
		_tcscat_s(largeText, FOUR_KB, _T(": "));
		_tcscat_s(largeText, FOUR_KB, valueText);
		_tcscat_s(largeText, FOUR_KB, _T("\r\n"));
	}

	USES_CONVERSION;

	// convert to ASCII and copy text to clipboard
	LPSTR data = W2A(largeText);
	Utils::CopyToClipboard(data, (int)strlen(data), CF_TEXT);

	bHandled = TRUE;
	return 0;
}

LRESULT CPreviewCtrl::OnOptionsAbout(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	AboutDlg dlg;
	dlg.DoModal(NULL);

	return 0;
}

LRESULT CPreviewCtrl::OnOptionsSaveStructure(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	ThumbFishDocument *pDoc = (ThumbFishDocument*)m_pDocument;
	if(pDoc)
	{
		OPTIONS options;
		options.RenderImageWidth = options.RenderImageHeight = 300;
		Utils::DoSaveStructure(m_hWnd, &pDoc->m_Buffer, &options);
	}
	else
		pantheios::log_WARNING(_T("CPreviewCtrl::OnOptionsSaveStructure> m_pDocument is NULL."));

	return 0;
}

LRESULT CPreviewCtrl::OnOptionsCopyStructureAs(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
	OPTIONS options;
	int cbFormat = CF_TEXT;
	ChemFormat format = fmtUnknown;
	ThumbFishDocument *pDoc = (ThumbFishDocument*)m_pDocument;

	switch (wID)
	{
		case ID_COPYSTRUCTUREAS_MOLV2000: format = fmtMOLV2; break;
		case ID_COPYSTRUCTUREAS_MOLV3000: format = fmtMOLV3; break;
		case ID_COPYSTRUCTUREAS_RXNV2000: format = fmtRXNV2; break;
		case ID_COPYSTRUCTUREAS_RXNV3000: format = fmtRXNV3; break;
		case ID_COPYSTRUCTUREAS_EMF: format = fmtEMF; break;
		case ID_COPYSTRUCTUREAS_CDXML: format = fmtCDXML; break;
		case ID_COPYSTRUCTUREAS_SMILES: format = fmtSMILES; break;
		case ID_COPYSTRUCTUREAS_INCHI: format = fmtINCHI; break;
		case ID_COPYSTRUCTUREAS_INCHIKEY: format = fmtINCHIKEY; break;
		case ID_COPYSTRUCTUREAS_CML: format = fmtCML; break;
	}
	
	Utils::ConvertAndCopy(&pDoc->m_Buffer, format, &options);

	return 0;
}

LRESULT CPreviewCtrl::OnCtlColor(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	HBRUSH hbr = NULL;

	// paints the background of static warning control WHITE so it appears 
	// transparent on the WHITE picture control that we have
	if (::GetWindowLong((HWND)lParam, GWL_EXSTYLE) & WS_EX_TRANSPARENT)
	{
		HDC hDC = (HDC)wParam;
		SetBkMode(hDC, TRANSPARENT);
		hbr = (HBRUSH)GetStockObject(WHITE_BRUSH);
	}

	return (LRESULT)hbr;
}

void CPreviewCtrl::CreateWarningControls(PTSTR pszText)
{
    // create balloon tooltip for static warning control
    m_ttipWarning = CreateWindowEx(0, TOOLTIPS_CLASS, NULL, WS_POPUP | TTS_ALWAYSTIP | TTS_BALLOON,
                              CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, m_hWnd, NULL, _AtlBaseModule.m_hInst, NULL);
    
	if(m_ttipWarning)
	{
		// associate the tooltip with the static control
		TOOLINFO toolInfo = { 0 };
		toolInfo.cbSize = sizeof(toolInfo);
		toolInfo.hwnd = m_hWnd;
		toolInfo.uFlags = TTF_IDISHWND | TTF_SUBCLASS | TTF_CENTERTIP;
		toolInfo.uId = (UINT_PTR)m_staticWarning;
		toolInfo.lpszText = pszText;

		// set tooltip options
		SendMessage(m_ttipWarning, TTM_ADDTOOL, 0, (LPARAM)&toolInfo);	// add tooltip to static control
		SendMessage(m_ttipWarning, TTM_SETMAXTIPWIDTH, 0, 400);
		SendMessage(m_ttipWarning, TTM_SETDELAYTIME, TTDT_AUTOPOP, 10000);	// set showtime to 10s
		SendMessage(m_ttipWarning, TTM_SETTITLE, TTI_WARNING, (LPARAM)_T("Chemical Warnings"));
	}
	else
	{
		pantheios::log_ERROR(_T("CPreviewCtrl::CreateWarningControls> Tooltip creation FAILED."));
	}
}
