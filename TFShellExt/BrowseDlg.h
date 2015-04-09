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
	TCHAR m_srcFile[MAX_PATH];
	std::vector<LPMOLRECORD> m_mols;
	int m_recordWithMaxProps;
	std::map<int, WCHAR*> m_ColIndexHeaderTextMap;

public:
	CBrowseDlg(PTSTR sourceFile) : m_recordWithMaxProps(0), m_rowHeight(100)
	{
		_tcscpy_s(m_srcFile, MAX_PATH, sourceFile);
	}

	~CBrowseDlg()
	{
	}

	enum { IDD = IDD_BROWSEDLG };

BEGIN_MSG_MAP(CBrowseDlg)
	MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
	MESSAGE_HANDLER(WM_MEASUREITEM, OnMeasureItem)
	MESSAGE_HANDLER(WM_DRAWITEM, OnDrawItem)
	MESSAGE_HANDLER(WM_SIZE, OnSize)
	COMMAND_HANDLER(IDOK, BN_CLICKED, OnClickedOK)
	COMMAND_HANDLER(IDCANCEL, BN_CLICKED, OnClickedCancel)
	NOTIFY_HANDLER(IDC_MOLLIST, LVN_GETDISPINFO, OnListGetDispInfo)
	CHAIN_MSG_MAP(CAxDialogImpl<CBrowseDlg>)
END_MSG_MAP()

// Handler prototypes:
//  LRESULT MessageHandler(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
//  LRESULT CommandHandler(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);

	static DWORD WINAPI ThreadProc(LPVOID lpParameter);
	static bool OnRecord(LPVOID instance, BrowseEventArgs* e);

	LRESULT OnListGetDispInfo(int idCtrl, LPNMHDR pnmh, BOOL& bHandled);

	LRESULT OnInitDialog(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	LRESULT OnMeasureItem(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	LRESULT OnDrawItem(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	LRESULT OnSize(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	LRESULT OnClickedOK(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
	LRESULT OnClickedCancel(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
};


