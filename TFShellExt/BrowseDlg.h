// BrowseDlg.h : Declaration of the CBrowseDlg

#pragma once

#include "resource.h"       // main symbols
#include <atlhost.h>
#include "BrowseParam.h"
#include <vector>

using namespace ATL;

class CBrowseDlg : 
	public CAxDialogImpl<CBrowseDlg>
{
private:
	typedef BrowseEventArgs MOLRECORD, *LPMOLRECORD;

	int m_rowHeight;
	bool m_cancelLoading;
	HANDLE m_threadHandle;
	bool m_structureColAdded;
	bool m_columnCountChanged;
	size_t m_totalBytesLoaded;

	TCHAR m_srcFile[MAX_PATH];
	std::vector<LPMOLRECORD> m_mols;
	int m_recordWithMaxProps;
	std::map<int, WCHAR*> m_ColIndexHeaderTextMap;

public:
	CBrowseDlg(PTSTR sourceFile) : m_recordWithMaxProps(0), m_rowHeight(100), m_columnCountChanged(true), 
		m_structureColAdded(false), m_totalBytesLoaded(0), m_cancelLoading(false), m_threadHandle(NULL)
	{
		_tcscpy_s(m_srcFile, MAX_PATH, sourceFile);
	}

	~CBrowseDlg()
	{
		if (m_threadHandle != NULL)
			CloseHandle(m_threadHandle);
	}

	enum { IDD = IDD_BROWSEDLG };

BEGIN_MSG_MAP(CBrowseDlg)
	MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
	MESSAGE_HANDLER(WM_MEASUREITEM, OnMeasureItem)
	MESSAGE_HANDLER(WM_DRAWITEM, OnDrawItem)
	MESSAGE_HANDLER(WM_SIZE, OnSize)
	COMMAND_HANDLER(IDCANCEL, BN_CLICKED, OnClickedCancel)
	COMMAND_HANDLER(IDC_BROWSE_CANCELLOAD, BN_CLICKED, OnCancelLoading)
	NOTIFY_HANDLER(IDC_MOLLIST, LVN_GETDISPINFO, OnListGetDispInfo)
	CHAIN_MSG_MAP(CAxDialogImpl<CBrowseDlg>)
END_MSG_MAP()

	static DWORD WINAPI ThreadProc(LPVOID lpParameter);
	static bool OnRecord(LPVOID instance, BrowseEventArgs* e);

	LRESULT OnListGetDispInfo(int idCtrl, LPNMHDR pnmh, BOOL& bHandled);

	LRESULT OnInitDialog(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	LRESULT OnMeasureItem(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	LRESULT OnDrawItem(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	LRESULT OnSize(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	LRESULT OnClickedCancel(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
	LRESULT OnCancelLoading(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);

	void ResizeControls();
};


