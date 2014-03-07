#pragma once
#include "stdafx.h"
#include "resource.h"       // main symbols

#define AFX_PREVIEW_STANDALONE
#include <atlhandler.h>
#include <atlhandlerimpl.h>
#include "ThumbFishDocument.h"
#include "atldlgpreviewctrlimpl.h"

class CPreviewCtrl : public CAtlDlgPreviewCtrlImpl
{
protected:	
	virtual void DoPaint(HDC hdc);
	virtual void SetRect(const RECT* prc, BOOL bRedraw);
	LRESULT OnInitDialog(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	LRESULT OnContextMenu(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);

private:
	void InsertLVItem(HWND hWndList, int index, TCHAR* item, int subitem);
	void InsertLVItem(HWND hWndList, int index, TCHAR* item, double subitem);
	void InsertLVItem(HWND hWndList, int index, TCHAR* item, TCHAR* subitem);

	void AutoSizeControls(RECT& parentRect);
	bool CopyToClipboard(const TCHAR* text);

public:
	BEGIN_MSG_MAP(CPreviewCtrl)
		// commands on ListView context menu
		COMMAND_ID_HANDLER(ID_OPTIONS_COPY, OnOptionsCopy)
		COMMAND_ID_HANDLER(ID_OPTIONS_COPYALL, OnOptionsCopyAll)
		COMMAND_ID_HANDLER(ID_OPTIONS_ABOUT, OnOptionsAbout)

		MESSAGE_HANDLER(WM_CONTEXTMENU, OnContextMenu)
		MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)

		// chains the above msg map with base class
		CHAIN_MSG_MAP(CAtlDlgPreviewCtrlImpl)
	END_MSG_MAP()

	LRESULT OnOptionsCopy(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT OnOptionsCopyAll(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT OnOptionsAbout(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
};
