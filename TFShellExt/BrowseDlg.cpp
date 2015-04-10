// BrowseDlg.cpp : Implementation of CBrowseDlg

#include "stdafx.h"
#include "BrowseDlg.h"
#include <sys/stat.h>

#include <sstream>

LRESULT CBrowseDlg::OnInitDialog(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	CAxDialogImpl<CBrowseDlg>::OnInitDialog(uMsg, wParam, lParam, bHandled);

	ResizeControls();
	ListView_SetExtendedListViewStyleEx(GetDlgItem(IDC_MOLLIST), LVS_EX_GRIDLINES | LVS_EX_FULLROWSELECT,
		LVS_EX_GRIDLINES | LVS_EX_FULLROWSELECT);

	LPBROWSEPARAMS param = new BROWSEPARAMS();
	sprintf_s(param->SourceFile, MAX_PATH, "%ls", m_srcFile);
	param->SourceFormat = CommonUtils::GetFormatFromFileName(m_srcFile);
	param->caller = this;
	param->callback = (BrowseCallback)(&CBrowseDlg::OnRecord);

	// set title with filename
	wchar_t title[MAX_PATH];
	swprintf_s(title, MAX_PATH, L"Browse [%s]", PathFindFileName(m_srcFile));
	SetWindowText(title);

	// spawn a new thread and start reading the source file
	m_threadHandle = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)&CBrowseDlg::ThreadProc, param, 0, NULL);

	bHandled = TRUE;
	return 1;  // Let the system set the focus
}

LRESULT CBrowseDlg::OnClickedCancel(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
	// check if the thread is running and notify user
	bool threadRunning = (WaitForSingleObject(m_threadHandle, 0) != WAIT_OBJECT_0);

	if (threadRunning)
		MessageBox(L"Cancel molecule loading operation before closing this dialog.",
			L"ThumbFish", MB_ICONWARNING | MB_OK);
	else
		EndDialog(wID);

	return 0;
}

LRESULT CBrowseDlg::OnCancelLoading(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
	m_cancelLoading = true;
	return 0;
}

DWORD WINAPI CBrowseDlg::ThreadProc(LPVOID lpParameter)
{
	OPTIONS options;
	BrowseParams* bParams = (BrowseParams*)lpParameter;
	CBrowseDlg* pDlg = (CBrowseDlg*)bParams->caller;

	wchar_t title[MAX_PATH];
	pDlg->GetWindowText(title, MAX_PATH);	// save original title

	// set file size as max range for progress bar
	struct _stat64i32 st;
	bool fileExists = (_wstat(pDlg->m_srcFile, &st) == 0);
	if (!fileExists) return 0;
	long fileSize = st.st_size;

	// init and show progress controls
	HWND hWndPB = pDlg->GetDlgItem(IDC_LOADPROGRESS);
	::ShowWindow(hWndPB, SW_SHOW);
	SendMessage(hWndPB, (UINT)PBM_SETRANGE32, (WPARAM)0, (LPARAM)fileSize); // progress range
	::ShowWindow(pDlg->GetDlgItem(IDC_BROWSE_CANCELLOAD), SW_SHOW);

	COMMANDPARAMS params(cmdBrowse, NULL, bParams);
	std::auto_ptr<OUTBUFFER> outBuffer(pExecuteFunc(&params, &options));

	// set title back to original (with mol count)
	wchar_t titleNew[MAX_PATH];
	swprintf_s(titleNew, MAX_PATH, L"%s (%d)", title, pDlg->m_mols.size());
	pDlg->SetWindowText(titleNew);

	// hide progress and cancel button
	::ShowWindow(hWndPB, SW_HIDE);
	::ShowWindow(pDlg->GetDlgItem(IDC_BROWSE_CANCELLOAD), SW_HIDE);

	return 0;
}

// called by provider for each record read from the file
bool CBrowseDlg::OnRecord(LPVOID instance, BrowseEventArgs* e)
{
	CBrowseDlg* pDlg = (CBrowseDlg*)instance;
	_ASSERT(pDlg != NULL);

	// add to local storage
	pDlg->m_mols.push_back(new MOLRECORD(*e));

	wchar_t title[MAX_PATH];
	swprintf_s(title, MAX_PATH, L"%d Molecules Loaded", pDlg->m_mols.size());
	pDlg->SetWindowText(title);

	HWND hWndPB = pDlg->GetDlgItem(IDC_LOADPROGRESS);
	pDlg->m_totalBytesLoaded += e->MolData->DataLength;
	SendMessage(hWndPB, (UINT)PBM_SETPOS, (WPARAM)pDlg->m_totalBytesLoaded, (LPARAM)0);

	// compare last item property count with saved item one and updated saved
	size_t lastIndex = pDlg->m_mols.size() - 1;
	if (pDlg->m_mols[lastIndex]->Properties.size() > pDlg->m_mols[pDlg->m_recordWithMaxProps]->Properties.size())
	{
		pDlg->m_columnCountChanged = true;
		pDlg->m_recordWithMaxProps = (int)lastIndex;
	}

	// periodically update the virtual list view count after every 100 records
	if (((pDlg->m_mols.size() % 100) == 0) || pDlg->m_cancelLoading)
	{
		HWND hWndListView = pDlg->GetDlgItem(IDC_MOLLIST);

		// create new columns if there are new columns
		if (pDlg->m_columnCountChanged)
		{
			MAP_WSWS map = pDlg->m_mols[pDlg->m_recordWithMaxProps]->Properties;
			int iCol = (int)pDlg->m_ColIndexHeaderTextMap.size();

			// create structure column as first column only once
			if (!pDlg->m_structureColAdded)
			{
				LVCOLUMN lvc;
				lvc.iSubItem = iCol;
				lvc.cx = 150;
				lvc.fmt = LVCFMT_CENTER;
				lvc.mask = LVCF_FMT | LVCF_WIDTH | LVCF_SUBITEM;
				ListView_InsertColumn(hWndListView, iCol++, &lvc);
				pDlg->m_structureColAdded = true;
			}

			// add colums using the record with max number of properties/columns
			for (MAP_WSWS::iterator it = map.begin(); it != map.end(); ++it)
			{
				bool columnExists = false;

				// go through the existing columns and check if the column 
				// with this name has already been created
				for (std::map<int, WCHAR*>::iterator it2 = pDlg->m_ColIndexHeaderTextMap.begin();
					it2 != pDlg->m_ColIndexHeaderTextMap.end(); it2++)
				{
					if (it2->second == it->first)
					{
						columnExists = true;
						break;
					}
				}

				if (!columnExists)
				{
					LVCOLUMN lvc;
					lvc.iSubItem = iCol;
					lvc.cx = 100;
					lvc.fmt = LVCFMT_LEFT;
					lvc.mask = LVCF_FMT | LVCF_WIDTH | LVCF_TEXT | LVCF_SUBITEM;
					lvc.pszText = it->first;

					pDlg->m_ColIndexHeaderTextMap.insert(std::make_pair(iCol, it->first));

					// Insert the columns into the list view.
					if (ListView_InsertColumn(hWndListView, iCol++, &lvc) == -1)
						return FALSE;
				}
			}

			// autosize the last column to fit
			ListView_SetColumnWidth(hWndListView, pDlg->m_ColIndexHeaderTextMap.size(), LVSCW_AUTOSIZE_USEHEADER);
			pDlg->m_columnCountChanged = false;
		}

		// set record count
		ListView_SetItemCountEx(hWndListView, pDlg->m_mols.size(), LVSICF_NOINVALIDATEALL | LVSICF_NOSCROLL);
	}

	return !pDlg->m_cancelLoading;
}

LRESULT CBrowseDlg::OnListGetDispInfo(int idCtrl, LPNMHDR pnmh, BOOL& bHandled)
{	
	NMLVDISPINFO* plvdi = (NMLVDISPINFO*)pnmh;

	if(m_mols.size() > 0)
	{
		if (plvdi->item.mask & LVIF_TEXT)
		{
			MAP_WSWS propMap = m_mols[plvdi->item.iItem]->Properties;
			size_t numColums = propMap.size();

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
		options.RenderMarginX = options.RenderMarginY = 5;
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

LRESULT CBrowseDlg::OnSize(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	ResizeControls();

	// size the last column to fill
	ListView_SetColumnWidth(GetDlgItem(IDC_MOLLIST), m_ColIndexHeaderTextMap.size(), LVSCW_AUTOSIZE_USEHEADER);

	return 0;
}

void CBrowseDlg::ResizeControls()
{
	RECT formRect;
	GetClientRect(&formRect);

	HWND hWndList = GetDlgItem(IDC_MOLLIST);
	::SetWindowPos(hWndList, NULL, 10, 40, (formRect.right - formRect.left) - 20,
		(formRect.bottom - formRect.top) - 50, SWP_NOZORDER);
}