#pragma once

#include "resource.h"       // main symbols
#include <atlhost.h>

using namespace ATL;

class CQuickFixDlg : 
	public CAxDialogImpl<CQuickFixDlg>
{
private:
	PTSTR m_sourceFile;
	LPBUFFER m_sourceBuffer;
	LPBUFFER m_activeBuffer;

public:
	CQuickFixDlg(LPBUFFER buffer, PTSTR sourceFile)
	{
		m_sourceFile = sourceFile;
		m_activeBuffer = m_sourceBuffer = buffer;
	}

	~CQuickFixDlg() { }

	enum { IDD = IDD_QUICKFIXDLG };

BEGIN_MSG_MAP(CQuickFixDlg)
	MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
	COMMAND_HANDLER(IDOK, BN_CLICKED, OnApplyChanges)
	COMMAND_HANDLER(IDCANCEL, BN_CLICKED, OnCancel)
	COMMAND_RANGE_HANDLER(ID_STRUCTURE_AROMATIZE, ID_STRUCTURE_RESETTODEFAULT, OnQuickFix)
	CHAIN_MSG_MAP(CAxDialogImpl<CQuickFixDlg>)
	MESSAGE_HANDLER(WM_PAINT, OnPaint)
END_MSG_MAP()

	LRESULT OnInitDialog(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	LRESULT OnQuickFix(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
	LRESULT OnPaint(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	LRESULT OnApplyChanges(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
	LRESULT OnCancel(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
};


