// GenerateDlg.cpp : Implementation of CGenerateDlg

#include "stdafx.h"
#include "GenerateDlg.h"
#include "windowsx.h"

LRESULT CGenerateDlg::OnInitDialog(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	CAxDialogImpl<CGenerateDlg>::OnInitDialog(uMsg, wParam, lParam, bHandled);
	bHandled = TRUE;

	// limit to the maximum we allocate
	Edit_LimitText(GetDlgItem(IDC_EDT_GENERATE_INPUT), ONE_KB);
	ComboBox_SetCurSel(GetDlgItem(IDC_CBO_INPUT_FORMAT), 0);

	return 1;  // Let the system set the focus
}

LRESULT CGenerateDlg::OnSaveAs(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
	BUFFER buffer;
	OPTIONS options;

	InitBuffer(&buffer);
	Utils::DoSaveStructure(m_hWnd, &buffer, &options);

	return 0;
}

LRESULT CGenerateDlg::OnClickedCancel(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
	EndDialog(wID);
	return 0;
}

LRESULT CGenerateDlg::OnInputChange(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
	Invalidate();
	return 0;
}

LRESULT CGenerateDlg::OnFormatSelChange(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
	OnInputChange(wNotifyCode, wID, hWndCtl, bHandled);
	return 0;
}

LRESULT CGenerateDlg::OnPaint(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	UNREFERENCED_PARAMETER(uMsg);
	UNREFERENCED_PARAMETER(wParam);
	UNREFERENCED_PARAMETER(lParam);

	PAINTSTRUCT ps;
	BeginPaint(&ps);

	if(::Edit_GetTextLength(GetDlgItem(IDC_EDT_GENERATE_INPUT)) > 0)
	{
		// get picture box handle and draw the active buffer data in it
		HWND hPict = GetDlgItem(IDC_PICT_GENERATE_STRUCTURE);
		if(hPict != NULL)
		{
			RECT rect;
			BUFFER buffer;
			OPTIONS options;
			
			InitBuffer(&buffer);

			// get client area of picture box
			::GetClientRect(hPict, &rect);

			DRAWPARAMS drawParams(::GetDC(hPict), rect);
			COMMANDPARAMS params((int)cmdDraw, &buffer, &drawParams);				
			std::auto_ptr<OUTBUFFER> outBuffer(pExecuteFunc(&params, &options));
		}
	}

	EndPaint(&ps);
	bHandled = TRUE;
	return 0;
}

void CGenerateDlg::InitBuffer(LPBUFFER buffer)
{
	buffer->DataFormat = (ComboBox_GetCurSel(GetDlgItem(IDC_CBO_INPUT_FORMAT)) == 0) ? fmtSMILES : fmtINCHI;
	buffer->pData = new char[ONE_KB];
	GetDlgItemTextA(m_hWnd, IDC_EDT_GENERATE_INPUT, (LPSTR)buffer->pData, ONE_KB);
	buffer->DataLength = strlen((LPSTR)buffer->pData) + 1;

	// auto append CRLF for SMILES data if we have space
	if((buffer->DataFormat == fmtSMILES) && (buffer->DataLength < ONE_KB))
	{
		PCHAR pch = (PCHAR)buffer->pData;
		pch[buffer->DataLength - 1] = LF;
		pch[buffer->DataLength] = NULL;
		buffer->DataLength += 1;
	}
}
