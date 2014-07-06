#pragma once
#include "stdafx.h"
#include "resource.h"       // main symbols

#define AFX_PREVIEW_STANDALONE
#include <atlhandler.h>
#include <atlhandlerimpl.h>
#include "ThumbFishDocument.h"
#include "atldlgpreviewctrlimpl.h"
#include "Utils.h"

class CPreviewCtrl : public CAtlDlgPreviewCtrlImpl
{
private:
	BOOL	m_previewDrawn;
	BOOL	m_propsGenerated;	// TRUE when there is at least 1 property
	HWND	m_ttipWarning;
	HWND	m_staticWarning;

protected:
	virtual void DoPaint(HDC hdc);
	virtual void SetRect(const RECT* prc, BOOL bRedraw);
	LRESULT OnInitDialog(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	LRESULT OnContextMenu(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	LRESULT OnCtlColor(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);

private:
	void InsertLVItem(HWND hWndList, int index, TCHAR* item, int subitem);
	void InsertLVItem(HWND hWndList, int index, TCHAR* item, double subitem);
	void InsertLVItem(HWND hWndList, int index, TCHAR* item, TCHAR* subitem);

	void AutoSizeControls(RECT& parentRect);
	void CreateWarningControls(PTSTR pszText);
	void FillProperties(LPBUFFER buffer);
	void SetChemicalWarnings(LPOPTIONS options);

public:
	CPreviewCtrl();

	BEGIN_MSG_MAP(CPreviewCtrl)
		// commands on ListView context menu
		COMMAND_ID_HANDLER(ID_OPTIONS_COPYPROPERTIES, OnOptionsCopyAll)
		COMMAND_ID_HANDLER(ID_OPTIONS_ABOUT, OnOptionsAbout)

		MESSAGE_HANDLER(WM_CONTEXTMENU, OnContextMenu)
		MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
		MESSAGE_HANDLER(WM_CTLCOLORSTATIC, OnCtlColor)

		// chains the above msg map with base class
		CHAIN_MSG_MAP(CAtlDlgPreviewCtrlImpl)
		COMMAND_ID_HANDLER(ID_OPTIONS_SAVESTRUCTURE, OnOptionsSaveStructure)

		COMMAND_RANGE_HANDLER(ID_COPYSTRUCTUREAS_CDXML, ID_COPYSTRUCTUREAS_MOLV2000, OnOptionsCopyStructureAs);

	END_MSG_MAP()

	LRESULT OnOptionsCopyAll(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT OnOptionsAbout(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT OnOptionsSaveStructure(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT OnOptionsCopyStructureAs(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
};
