// AboutDlg.h : Declaration of the AboutDlg

#pragma once

#include "resource.h"       // main symbols

#include <atlhost.h>

using namespace ATL;

// AboutDlg

class AboutDlg : 
	public CAxDialogImpl<AboutDlg>
{
public:
	AboutDlg()
	{
	}

	~AboutDlg()
	{
	}

	enum { IDD = IDD_ABOUTDLG };

BEGIN_MSG_MAP(AboutDlg)
	MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
	COMMAND_HANDLER(IDOK, BN_CLICKED, OnClickedOK)
	CHAIN_MSG_MAP(CAxDialogImpl<AboutDlg>)
	NOTIFY_HANDLER(IDC_LINKONLINE, NM_CLICK, OnNMClickLinkonline)
END_MSG_MAP()

// Handler prototypes:
//  LRESULT MessageHandler(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
//  LRESULT CommandHandler(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
//  LRESULT NotifyHandler(int idCtrl, LPNMHDR pnmh, BOOL& bHandled);

	LRESULT OnInitDialog(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
	{
		// Load about text from resource. Static Text control has a limit of 256 characters
		// so we cannot just set the text in UI
		TCHAR buffer[1024];
		if(LoadString(_AtlBaseModule.m_hInst, IDS_ABOUTTEXT, buffer, 1024) > 0)
		{
			SetDlgItemText(IDC_ABOUTTEXT, buffer);
		}

		CAxDialogImpl<AboutDlg>::OnInitDialog(uMsg, wParam, lParam, bHandled);
		bHandled = TRUE;
		return 1;  // Let the system set the focus
	}

	LRESULT OnClickedOK(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
	{
		EndDialog(wID);
		return 0;
	}
	LRESULT OnNMClickLinkonline(int /*idCtrl*/, LPNMHDR pNMHDR, BOOL& /*bHandled*/);
};


