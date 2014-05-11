// AboutDlg.cpp : Implementation of AboutDlg

#include "stdafx.h"
#include "AboutDlg.h"


// AboutDlg


LRESULT AboutDlg::OnNMClickLinkonline(int /*idCtrl*/, LPNMHDR pNMHDR, BOOL& /*bHandled*/)
{
	SHELLEXECUTEINFO shInfo;
	shInfo.cbSize = sizeof(SHELLEXECUTEINFO);
    shInfo.fMask = NULL;
    shInfo.hwnd = NULL;
    shInfo.lpVerb = NULL;
    shInfo.lpFile = _T("http://abhimanyusirohi.github.io/ThumbFish/");
    shInfo.lpParameters = NULL;
    shInfo.lpDirectory = NULL;
    shInfo.nShow = SW_SHOW;
    shInfo.hInstApp = NULL;

	ShellExecuteEx(&shInfo);

	return 0;
}
