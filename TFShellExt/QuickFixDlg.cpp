// QuickFixDlg.cpp : Implementation of CQuickFixDlg

#include "stdafx.h"
#include "QuickFixDlg.h"

LRESULT CQuickFixDlg::OnInitDialog(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	CAxDialogImpl<CQuickFixDlg>::OnInitDialog(uMsg, wParam, lParam, bHandled);
	bHandled = TRUE;
	return 1;  // Let the system set the focus
}

LRESULT CQuickFixDlg::OnQuickFix(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
	OPTIONS options;
	Command cmd = cmdAromatize;

	if(wID == ID_STRUCTURE_VALIDATE)
	{
		TCHAR msg[ONE_KB];

		COMMANDPARAMS params(cmdValidate, m_activeBuffer, NULL);
		std::auto_ptr<OUTBUFFER> outBuffer(pExecuteFunc(&params, &options));
		if(outBuffer->DataLength > 0)
			swprintf(msg, ONE_KB, _T("%hs"), (const char*) outBuffer->pData);
		else
			swprintf(msg, ONE_KB, _T("No problems found."));

		MessageBox(msg, _T("Validate"), MB_ICONINFORMATION);
		return 0;
	}
	else if(wID == ID_STRUCTURE_RESETTODEFAULT)
	{
		// release the existing buffer if it is not the original structure buffer
		if(m_activeBuffer != m_sourceBuffer) DeleteAndNull(m_activeBuffer);
		m_activeBuffer = m_sourceBuffer;
	}
	else
	{
		switch(wID)
		{
		case ID_STRUCTURE_AROMATIZE:
			cmd = cmdAromatize;
			break;
		case ID_STRUCTURE_DEAROMATIZE:
			cmd = cmdDearomatize;
			break;
		case ID_STRUCTURE_CLEANUP:
			cmd = cmdCleanup;
			break;
		case ID_STRUCTURE_NORMALIZE:
			cmd = cmdNormalize;
			break;
		case ID_STRUCTURE_FOLDHYDROGENS:
			cmd = cmdFoldHydrogens;
			break;
		case ID_STRUCTURE_UNFOLDHYDROGENS:
			cmd = cmdUnfoldHydrogens;
			break;
		}

		COMMANDPARAMS params(cmd, m_activeBuffer, NULL);
		std::auto_ptr<OUTBUFFER> outBuffer(pExecuteFunc(&params, &options));
			
		if((outBuffer.get() != NULL) && (outBuffer->DataLength > 0))
		{
			// release the existing buffer if it is not the original structure buffer
			if(m_activeBuffer != m_sourceBuffer) DeleteAndNull(m_activeBuffer);

			// init activeBuffer with output data
			m_activeBuffer = new BUFFER();

			// Indigo does not support saving as SMARTS so SMARTS format is converted to MOL V3000
			// during Perform operation so we need to set the correct format here
			m_activeBuffer->DataFormat = (m_sourceBuffer->DataFormat == fmtSMARTS) ? fmtMOLV3 : m_sourceBuffer->DataFormat;
			m_activeBuffer->pData = outBuffer->pData;
			m_activeBuffer->DataLength = outBuffer->DataLength;
		}
		else
		{
			MessageBox(_T("There was some problem applying the fix."), _T("Structure QuickFix"), 0);
		}
	}

	// refresh windows to display changed structure
	Invalidate();
	UpdateWindow();

	return 0;
}

LRESULT CQuickFixDlg::OnPaint(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	UNREFERENCED_PARAMETER(uMsg);
	UNREFERENCED_PARAMETER(wParam);
	UNREFERENCED_PARAMETER(lParam);

	PAINTSTRUCT ps;
	BeginPaint(&ps);

	// get picture box handle and draw the active buffer data in it
	HWND hPict = GetDlgItem(IDC_QFPICT);
	if(hPict != NULL)
	{
		RECT rect;
		OPTIONS options;

		// get client area of picture box
		::GetClientRect(hPict, &rect);

		DRAWPARAMS drawParams(::GetDC(hPict), rect);
		COMMANDPARAMS params((int)cmdDraw, m_activeBuffer, &drawParams);				
		std::auto_ptr<OUTBUFFER> outBuffer(pExecuteFunc(&params, &options));
	}

	EndPaint(&ps);
	bHandled = TRUE;
	return 0;
}

LRESULT CQuickFixDlg::OnApplyChanges(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
	HANDLE hFile; 
	DWORD dwBytesWritten = 0;
	BOOL bErrorFlag = FALSE;
	PCHAR buffer = (PCHAR)m_activeBuffer->pData;
	DWORD dwBytesToWrite = (DWORD)m_activeBuffer->DataLength;

	if(MessageBox(_T("Are you sure you want to overwrite the existing file with current changes? This action cannot be undone."), 
		_T("Structure QuickFix"), MB_YESNO | MB_ICONQUESTION) == IDNO)
		return 0;

	//INFO: SMARTS file will be saved as MOLV3

	// truncate the existing file and write changed data
	hFile = CreateFile(m_sourceFile, GENERIC_WRITE, 0, NULL, TRUNCATE_EXISTING, FILE_ATTRIBUTE_NORMAL,	NULL);
	if (hFile != INVALID_HANDLE_VALUE)
	{
		bErrorFlag = WriteFile(hFile, buffer, dwBytesToWrite, &dwBytesWritten, NULL);
		if ((FALSE == bErrorFlag) || (dwBytesWritten != dwBytesToWrite))
		{
			pantheios::log_ERROR(_T("Problem writing to file="), m_sourceFile, 
				_T(", GetLastError="), pantheios::integer(GetLastError()));
			MessageBox(_T("There was an error trying to save changes."), _T("File Write Error"), MB_OK | MB_ICONERROR);
			return S_FALSE;
		}
		CloseHandle(hFile);
	}
	else
	{
		pantheios::log_ERROR(_T("Problem opening file="), m_sourceFile, _T(", GetLastError="), 
			pantheios::integer(GetLastError()));
		MessageBox(_T("There was a problem opening the existing file. Please check the file and try again."), 
			_T("File Write Error"), MB_OK | MB_ICONERROR);
		return S_FALSE;
	}

	// update the original buffer
	m_sourceBuffer = m_activeBuffer;

	MessageBox(_T("Changes applied successfully."), _T("Structure QuickFix"), MB_OK | MB_ICONINFORMATION);

	return S_OK;
}

LRESULT CQuickFixDlg::OnCancel(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
	EndDialog(wID);
	return 0;
}
