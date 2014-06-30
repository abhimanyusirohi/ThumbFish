#include "StdAfx.h"
#include "PreviewCtrl.h"
#include "AboutDlg.h"

#define ONE_KB	1024
#define FOUR_KB	4*ONE_KB

CPreviewCtrl::CPreviewCtrl() : m_previewDrawn(false), m_propsGenerated(false)
{
	pantheios::log_NOTICE(_T("CPreviewCtrl::CPreviewCtrl> Called."));
	m_pDocument = NULL;
}

void CPreviewCtrl::DoPaint(HDC hdc)
{
	pantheios::log_NOTICE(_T("CPreviewCtrl::DoPaint> Called"));
	
	OPTIONS options;
	ThumbFishDocument *pDoc = (ThumbFishDocument*)m_pDocument;

	if((pDrawFunc != NULL) && (pDoc != NULL))
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

			if(!m_previewDrawn)
				pantheios::log_NOTICE(_T("CPreviewCtrl::DoPaint> Draw returned FALSE."));

			// get handle of property ListView
			HWND hWndList = ::GetDlgItem(m_hWnd, IDC_PROPLIST);

			// avoid refilling properties
			if(ListView_GetItemCount(hWndList) == 0)
			{
				TCHAR** props = NULL;
				int propCount = pGetPropsFunc(&pDoc->m_Buffer, &props, &options, false);
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

		for(int id = ID_COPYSTRUCTUREAS_CDXML; id <= ID_COPYSTRUCTUREAS_MOLV2000; id++)
			::EnableMenuItem(hPopupMenu, id, MF_BYCOMMAND | ((m_previewDrawn && !multiMolFile) ? MF_ENABLED : MF_DISABLED));

		//TODO: enable this when Copy As InChiKey is fixed
		::EnableMenuItem(hPopupMenu, ID_COPYSTRUCTUREAS_INCHIKEY, MF_BYCOMMAND | MF_DISABLED);
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
	Utils::CopyToClipboard(data, strlen(data), CF_TEXT);

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
	ConvertFormats format;
	ThumbFishDocument *pDoc = (ThumbFishDocument*)m_pDocument;

	switch (wID)
	{
		case ID_COPYSTRUCTUREAS_MOLV2000: format = ConvertFormats::MolV2000; break;
		case ID_COPYSTRUCTUREAS_MOLV3000: format = ConvertFormats::MolV3000; break;
		case ID_COPYSTRUCTUREAS_EMF: format = ConvertFormats::EMF; break;
		case ID_COPYSTRUCTUREAS_CDXML: format = ConvertFormats::CDXML; break;
		case ID_COPYSTRUCTUREAS_SMILES: format = ConvertFormats::SMILES; break;
		case ID_COPYSTRUCTUREAS_INCHI: format = ConvertFormats::InChi; break;
		case ID_COPYSTRUCTUREAS_INCHIKEY: format = ConvertFormats::InChiKey; break;
	}
	
	char* data = Utils::ConvertStructure(&pDoc->m_Buffer, format, &options);
	if(data != NULL)
	{
		Utils::CopyToClipboard(data, options.OutBufferSize, 
			(format == ConvertFormats::EMF) ? CF_ENHMETAFILE : CF_TEXT);
	}
	else
	{
		pantheios::log_ERROR(_T("CPreviewCtrl::OnOptionsCopyStructureAs> Convert method FAILED. OutBufferSize= "),
			pantheios::integer(options.OutBufferSize));
	}

	return 0;
}
