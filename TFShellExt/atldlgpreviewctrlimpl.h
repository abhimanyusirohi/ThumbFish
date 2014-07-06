// Code duplicated from ATL library's AtlPreviewCtrlImpl.h
// to implement a dialog based preview control. The original
// one implements CWindow based control that requires painting.

#pragma once

#include <atldef.h>

//#if !defined(_ATL_USE_WINAPI_FAMILY_DESKTOP_APP)
//#error This file is not compatible with the current WINAPI_FAMILY
//#endif


#include <atlwin.h>
#include <atlhandler.h>

#pragma once

#ifndef __cplusplus
#error ATL requires C++ compilation (use a .cpp suffix)
#endif

#pragma pack(push,_ATL_PACKING)
namespace ATL
{
/// <summary>
/// This class is an ATL implementation of a window that is placed on a host window provided by the Shell
/// for Rich Preview. </summary>
class CAtlDlgPreviewCtrlImpl :
	public CDialogImpl<CAtlDlgPreviewCtrlImpl>,
	public IPreviewCtrl
{
public:
   enum { IDD = IDD_PREVIEWDIALOG };

public:
	BEGIN_MSG_MAP(CAtlDlgPreviewCtrlImpl)
		MESSAGE_HANDLER(WM_PAINT, OnPaint)
	END_MSG_MAP()

	/// <summary>
	/// Constructs a preview control object.
	/// </summary>
	CAtlDlgPreviewCtrlImpl(void) : m_clrText(0), m_clrBack(RGB(255, 255, 255)), m_plf(NULL)
	{
	}

	/// <summary>
	/// Destructs a preview control object.</summary>
	virtual ~CAtlDlgPreviewCtrlImpl(void)
	{
	}
	/// <summary>Handles WM_PAINT message.</summary>
	/// <param name="nMsg">Set to WM_PAINT.</param>
	/// <param name="wParam">This parameter is not used. </param>
	/// <param name="lParam">This parameter is not used. </param>
	/// <param name="bHandled">When this function returns it contains TRUE.</param>
	/// <returns> Always returns 0.</returns>
	LRESULT OnPaint(
		_In_ UINT nMsg,
		_In_ WPARAM wParam,
		_In_ LPARAM lParam,
		_Out_ BOOL& bHandled)
	{
		UNREFERENCED_PARAMETER(nMsg);
		UNREFERENCED_PARAMETER(wParam);
		UNREFERENCED_PARAMETER(lParam);

		PAINTSTRUCT ps;
		BeginPaint(&ps);
		DoPaint(ps.hdc);
		EndPaint(&ps);

		bHandled = TRUE;

		return 0;
	}

	/// <summary>
	/// Called by a Rich Preview handler to create the Windows window. </summary>
	/// <param name="hWndParent"> A handle to the host window supplied by the Shell for Rich Preview. </param>
	/// <param name="prc"> Specifies initial size and position of the window. </param>
	/// <returns>TRUE if creation succeeded; otherwise FALSE.</returns>
	virtual BOOL Create(
		_In_ HWND hWndParent,
		_In_ const RECT* prc)
	{
		CDialogImpl<CAtlDlgPreviewCtrlImpl>::Create(hWndParent, const_cast<RECT&>(*prc), NULL);
		ShowWindow(SW_SHOW);
		return TRUE;
	}
	/// <summary>
	/// Called by a Rich Preview handler when it needs to destroy this control.</summary>
	virtual void Destroy()
	{
		CDialogImpl<CAtlDlgPreviewCtrlImpl>::DestroyWindow();
	}
	/// <summary>
	/// Sets a new parent for this control. </summary>
	/// <param name="hWndParent">A handle to the new parent window.</param>
	virtual void SetHost(_In_ HWND hWndParent)
	{
		CDialogImpl<CAtlDlgPreviewCtrlImpl>::SetParent(hWndParent);
	}
	/// <summary>
	/// Sets input focus to this control. </summary>
	virtual void Focus()
	{
		CDialogImpl<CAtlDlgPreviewCtrlImpl>::SetFocus();
	}
	/// <summary>
	/// Tells this control to redraw. </summary>
	virtual void Redraw()
	{
		CDialogImpl<CAtlDlgPreviewCtrlImpl>::RedrawWindow();
	}
	/// <summary>
	/// Sets a new bounding rectangle for this control. </summary>
	/// <remarks> Usually new bounding rectangle is set when the host control is resized.</remarks>
	/// <param name="prc">Specifies the new size and position of preview control.</param>
	/// <param name="bRedraw">Specifies whether the control should be redrawn.</param>
	virtual void SetRect(
		_In_ const RECT* prc,
		_In_ BOOL bRedraw)
	{
		DWORD dwFlags = SWP_NOZORDER;
		if (!bRedraw)
		{
			dwFlags |= SWP_NOREDRAW;
		}
		SetWindowPos(NULL, prc, dwFlags);
	}
	/// <summary>
	/// Called by a Rich Preview handler when it needs to set visuals of rich preview content.</summary>
	/// <param name="clrBack"> Background color of preview window. </param>
	/// <param name="clrText"> Text color of preview window. </param>
	/// <param name="plf"> Font used to display texts in preview window. </param>
	virtual void SetPreviewVisuals(
		_In_ COLORREF clrBack,
		_In_ COLORREF clrText,
		_In_ const LOGFONTW *plf)
	{
		m_clrText = clrText;
		m_clrBack = clrBack;
		m_plf = plf;
	}

protected:
	/// <summary> Called by the framework to render the preview.</summary>
	/// <param name="hdc">A handle to a device context for painting.</par
	virtual void DoPaint(_In_ HDC hdc)
	{
		CString str = _T("Override CAtlPreviewCtrlImpl::DoPaint in a derived class for painting.");
		TextOut(hdc, 0, 0, str, str.GetLength());
	}

protected:
	/// <summary> Text color of preview window. </summary>
	COLORREF m_clrText;
	/// <summary> Background color of preview window. </summary>
	COLORREF m_clrBack;
	/// <summary> Font used to display texts in preview window. </summary>
	const LOGFONTW* m_plf;
};
} //namespace ATL
#pragma pack(pop)