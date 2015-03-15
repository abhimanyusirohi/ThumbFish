// GenerateDlg.h : Declaration of the CGenerateDlg

#pragma once

#include "resource.h"       // main symbols

#include <atlhost.h>

using namespace ATL;

// CGenerateDlg

class CGenerateDlg : 
	public CAxDialogImpl<CGenerateDlg>
{
public:
	CGenerateDlg()
	{
	}

	~CGenerateDlg()
	{
	}

	enum { IDD = IDD_GENERATEDLG };

BEGIN_MSG_MAP(CGenerateDlg)
	MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
	COMMAND_HANDLER(IDOK, BN_CLICKED, OnSaveAs)
	COMMAND_HANDLER(IDCANCEL, BN_CLICKED, OnClickedCancel)
	CHAIN_MSG_MAP(CAxDialogImpl<CGenerateDlg>)
	MESSAGE_HANDLER(WM_PAINT, OnPaint)
	COMMAND_HANDLER(IDC_EDT_GENERATE_INPUT, EN_CHANGE, OnInputChange)
	COMMAND_HANDLER(IDC_CBO_INPUT_FORMAT, CBN_SELCHANGE, OnFormatSelChange)
END_MSG_MAP()

	LRESULT OnInitDialog(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	LRESULT OnSaveAs(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
	LRESULT OnClickedCancel(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
	LRESULT OnInputChange(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT OnFormatSelChange(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT OnPaint(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);

	void InitBuffer(LPBUFFER buffer);
};


