// BrowseDlg.cpp : Implementation of CBrowseDlg

#include "stdafx.h"
#include "BrowseDlg.h"

#include <sstream>

LRESULT CBrowseDlg::OnInitDialog(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	CAxDialogImpl<CBrowseDlg>::OnInitDialog(uMsg, wParam, lParam, bHandled);
		
	ListView_SetExtendedListViewStyleEx(GetDlgItem(IDC_MOLLIST), LVS_EX_GRIDLINES | LVS_EX_FULLROWSELECT, 
		LVS_EX_GRIDLINES | LVS_EX_FULLROWSELECT);

	// spawn a new thread and start reading the source file
	LPBROWSEPARAMS param = new BROWSEPARAMS();
	sprintf_s(param->SourceFile, MAX_PATH, "%ls", m_srcFile);
	param->SourceFormat = CommonUtils::GetFormatFromFileName(m_srcFile);
	param->caller = this;
	param->callback = (BrowseCallback)(&CBrowseDlg::OnRecord);
	CloseHandle(CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)&CBrowseDlg::ThreadProc, param, 0, NULL));

	bHandled = TRUE;
	return 1;  // Let the system set the focus
}

LRESULT CBrowseDlg::OnClickedOK(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
	EndDialog(wID);
	return 0;
}

LRESULT CBrowseDlg::OnClickedCancel(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
	EndDialog(wID);
	return 0;
}

DWORD WINAPI CBrowseDlg::ThreadProc(LPVOID lpParameter)
{
	OPTIONS options;
	BrowseParams* bParams = (BrowseParams*)lpParameter;

	COMMANDPARAMS params(cmdBrowse, NULL, bParams);
	std::auto_ptr<OUTBUFFER> outBuffer(pExecuteFunc(&params, &options));

	// finally create colums and update the total record count for list
	CBrowseDlg* pDlg = (CBrowseDlg*)bParams->caller;
	HWND hWndListView = pDlg->GetDlgItem(IDC_MOLLIST);

	// add colums using the record with max number of properties/columns
	int numColumns = pDlg->m_mols[pDlg->m_recordWithMaxProps]->Properties.size();

	int iCol = 0;
	LVCOLUMN lvc;
    lvc.iSubItem = iCol;
    lvc.cx = 150;
	lvc.fmt = LVCFMT_CENTER;
	lvc.mask = LVCF_FMT | LVCF_WIDTH | LVCF_SUBITEM;
	//pDlg->m_ColIndexHeaderTextMap.insert(std::make_pair<int, WCHAR*>(iCol, it->first));
    ListView_InsertColumn(hWndListView, iCol++, &lvc);

	MAP_WSWS map = pDlg->m_mols[pDlg->m_recordWithMaxProps]->Properties;
	for (MAP_WSWS::iterator it = map.begin(); it != map.end(); ++it, iCol++)
    {
		LVCOLUMN lvc;
        lvc.iSubItem = iCol;
        lvc.cx = 100;
		lvc.fmt = LVCFMT_LEFT;
		lvc.mask = LVCF_FMT | LVCF_WIDTH | LVCF_TEXT | LVCF_SUBITEM;
        lvc.pszText = it->first;
		
		pDlg->m_ColIndexHeaderTextMap.insert(std::make_pair(iCol, it->first));

        // Insert the columns into the list view.
        if (ListView_InsertColumn(hWndListView, iCol, &lvc) == -1)
            return FALSE;
    }
    
	// set record count
	ListView_SetItemCountEx(hWndListView, pDlg->m_mols.size(), LVSICF_NOINVALIDATEALL | LVSICF_NOSCROLL);

	return 0;
}

// called by provider for each record read from the file
bool CBrowseDlg::OnRecord(LPVOID instance, BrowseEventArgs* e)
{
	CBrowseDlg* pDlg = (CBrowseDlg*)instance;
	_ASSERT(pDlg != NULL);

	//TODO: Update progress
	
	// add to local storage
	pDlg->m_mols.push_back(new MOLRECORD(*e));

	// compare last item property count with saved item one and updated saved
	int lastIndex = pDlg->m_mols.size() - 1;
	if(pDlg->m_mols[lastIndex]->Properties.size() > pDlg->m_mols[pDlg->m_recordWithMaxProps]->Properties.size())
		pDlg->m_recordWithMaxProps = lastIndex;

	//TODO: Test this
	// periodically update the virtual list view count after every 100 records
	if((pDlg->m_mols.size() % 100) == 0)
		ListView_SetItemCountEx(pDlg->GetDlgItem(IDC_MOLLIST), pDlg->m_mols.size(), LVSICF_NOINVALIDATEALL | LVSICF_NOSCROLL);

	return true;
}

LRESULT CBrowseDlg::OnListGetDispInfo(int idCtrl, LPNMHDR pnmh, BOOL& bHandled)
{	
	NMLVDISPINFO* plvdi = (NMLVDISPINFO*)pnmh;

	if(m_mols.size() > 0)
	{
		if (plvdi->item.mask & LVIF_TEXT)
		{
			MAP_WSWS propMap = m_mols[plvdi->item.iItem]->Properties;
			int numColums = propMap.size();

			// find the header text for this subitem
			std::map<int, WCHAR*>::iterator itHeader = m_ColIndexHeaderTextMap.find(plvdi->item.iSubItem);
			if(itHeader != m_ColIndexHeaderTextMap.end())
			{
				// find the item with property name same as the header text
				MAP_WSWS::iterator it = propMap.find(itHeader->second);
				if(it != propMap.end())
					plvdi->item.pszText = it->second;
			}
		}
	}

	return TRUE;
}

LRESULT CBrowseDlg::OnDrawItem(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	LPDRAWITEMSTRUCT di = (LPDRAWITEMSTRUCT)lParam;
	if(di->CtlID == IDC_MOLLIST)
	{
		OPTIONS options;

		// Save these value to restore them when done drawing.
		COLORREF crOldTextColor = GetTextColor(di->hDC);
		COLORREF crOldBkColor = GetBkColor(di->hDC);

		// If this item is selected, set the background color  
		// and the text color to appropriate values. Also, erase 
		// rect by filling it with the background color. 
		if ((di->itemAction | ODA_SELECT) && (di->itemState & ODS_SELECTED))
		{
			SetTextColor(di->hDC, ::GetSysColor(COLOR_HIGHLIGHTTEXT));
			SetBkColor(di->hDC, ::GetSysColor(COLOR_HIGHLIGHT));
			FillRect(di->hDC, &di->rcItem, ::GetSysColorBrush(COLOR_HIGHLIGHT));
			options.RenderBackgroundColor = ::GetSysColor(COLOR_HIGHLIGHT);
		}
		else
		{
			FillRect(di->hDC, &di->rcItem, ::GetSysColorBrush(crOldBkColor));
			options.RenderBackgroundColor = crOldBkColor;
		}

		// restore original
		SetTextColor(di->hDC, crOldTextColor);
		SetBkColor(di->hDC, crOldBkColor);

		RECT rect;
		HWND lv = di->hwndItem;

		// Draw molecule in first column
		BUFFER buffer(false);
		buffer.pData = m_mols[di->itemID]->MolData->pData;
		buffer.DataLength = m_mols[di->itemID]->MolData->DataLength;
		buffer.DataFormat = m_mols[di->itemID]->DataFormat;

		ListView_GetItemRect(lv, di->itemID, &rect, LVIR_LABEL);
		
		DRAWPARAMS drawParams(di->hDC, rect);
		COMMANDPARAMS params((int)cmdDraw, &buffer, &drawParams);
		options.IsThumbnail = false;
		//options.RenderMarginX = options.RenderMarginY = 10;
		options.HDC_offset_X = rect.left;
		options.HDC_offset_Y = rect.top;
		std::auto_ptr<OUTBUFFER> outBuffer(pExecuteFunc(&params, &options));

		// Draw other columns
		WCHAR text[QUARTER_KB];
		for(int colIndex = 0; colIndex < m_ColIndexHeaderTextMap.size(); colIndex++)
		{
			text[0] = NULL;

			// skip molecule column and get text for others
			ListView_GetItemText(lv, di->itemID, colIndex + 1, text, QUARTER_KB);
			if(text[0] != NULL)
			{
				ListView_GetSubItemRect(lv, di->itemID, colIndex + 1, LVIR_LABEL, &rect);

				InflateRect(&rect, -2, -2);
				DrawText(di->hDC, text, -1, &rect, DT_WORDBREAK);
			}			
		}

		bHandled = TRUE;
		return TRUE;
	}

	return TRUE;
}

LRESULT CBrowseDlg::OnMeasureItem(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	LPMEASUREITEMSTRUCT mi = (LPMEASUREITEMSTRUCT)lParam;
	if (mi->CtlID == IDC_MOLLIST)
	{
		mi->itemHeight = m_rowHeight;
	}

	return 1;
}
