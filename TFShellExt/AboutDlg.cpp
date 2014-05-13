// AboutDlg.cpp : Implementation of AboutDlg

#include "stdafx.h"
#include "AboutDlg.h"
#include "Utils.h"

// AboutDlg


LRESULT AboutDlg::OnNMClickLinkonline(int /*idCtrl*/, LPNMHDR pNMHDR, BOOL& /*bHandled*/)
{
	Utils::ShellExecuteLink(c_OnlineLink);
	return 0;
}
