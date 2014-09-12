// ExtractDlg.h : Declaration of the CExtractDlg

#pragma once

#include "stdafx.h"
#include "resource.h"       // main symbols

#include <atlhost.h>
#include "Utils.h"

using namespace ATL;

class CExtractDlg : 
	public CAxDialogImpl<CExtractDlg>
{
public:
	int				m_currentGroup;
	ExtractParams	m_params;
	HANDLE			m_threadHandle;

	static bool		m_userCancelled;

public:
	CExtractDlg(PTSTR sourceFile) : m_currentGroup(-1), m_threadHandle(NULL)
	{
		m_params.callback = (ProgressCallback)(&CExtractDlg::OnProgressChanged);
		m_params.sourceFormat = CommonUtils::GetFormatFromFileName(sourceFile);
		_tcscpy_s(m_params.sourceFile, MAX_PATH, sourceFile);

		// set the default output format based on source file format
		if(m_params.sourceFormat == fmtSDF) m_params.exportFormat = fmtMOLV2;
		else if(m_params.sourceFormat == fmtRDF) m_params.exportFormat = fmtRXNV2;
		else if(m_params.sourceFormat == fmtCML) m_params.exportFormat = fmtCML;
		else if(m_params.sourceFormat == fmtSMILES) m_params.exportFormat = fmtSMILES;

		// set output folder to source file folder
		_tcscpy_s(m_params.folderPath, MAX_PATH, sourceFile);
		PathRemoveFileSpec(m_params.folderPath);
	}

	~CExtractDlg()
	{
	}

	enum { IDD = IDD_EXTRACTDLG };

BEGIN_MSG_MAP(CExtractDlg)
	MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
	COMMAND_HANDLER(IDOK, BN_CLICKED, OnClickedOK)
	COMMAND_HANDLER(IDCANCEL, BN_CLICKED, OnClickedCancel)
	CHAIN_MSG_MAP(CAxDialogImpl<CExtractDlg>)
	NOTIFY_HANDLER(IDC_LINKMAIN, NM_CLICK, OnNMClickLinkMolCount)
	COMMAND_HANDLER(IDC_BTN_UPDATEOPTIONS, BN_CLICKED, OnUpdateOptionsClicked)
	COMMAND_RANGE_HANDLER(ID_EXTRACTFORMAT_MOL, ID_EXTRACTFORMAT_MOLV3000, OnChangeDataFormat);
	COMMAND_RANGE_HANDLER(ID_OVERWRITE_OVERWRITTEN, ID_OVERWRITE_SKIPPED, OnChangeOverwrite);
END_MSG_MAP()

	LRESULT OnInitDialog(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
	{
		CAxDialogImpl<CExtractDlg>::OnInitDialog(uMsg, wParam, lParam, bHandled);
		bHandled = TRUE;

		m_params.caller = this;

		// update syslink control text with current values
		UpdateLinkText();
		
		// set check mark on update button
		SendDlgItemMessage(IDC_BTN_UPDATEOPTIONS, BM_SETIMAGE, IMAGE_ICON, 
			(LPARAM)LoadImage(_AtlBaseModule.m_hInst, MAKEINTRESOURCE(IDI_TICK), IMAGE_ICON, 24, 24, NULL));

		// set group caption
		TCHAR grpCaption[MAX_PATH];
		_snwprintf_s(grpCaption, MAX_PATH, _T("[%s]"), PathFindFileName(m_params.sourceFile));
		SetDlgItemText(IDC_GRP_MAIN, grpCaption);

		SetDlgItemInt(IDC_EDTEXTRACTCOUNT, 50);	// set default extract count value

		SetDlgItemText(IDC_EDT_FILEFORMAT, m_params.fileFormat);
		CheckDlgButton(m_params.extractMolCount == -1 ? IDC_RDO_EXTRACTALL : IDC_RDO_EXTRACTSOME, 1);

		ResizeDialog(false);

		return 1;  // Let the system set the focus
	}

	LRESULT OnClickedOK(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
	{
		TCHAR ext[10];
		if(CommonUtils::GetFormatExtension(m_params.exportFormat, ext, 10))
		{
			// set folder path to full export file template containing %d that will 
			// be replaced during extract process
			_snwprintf_s(m_params.folderPath, MAX_PATH, _T("%s\\%s.%s"), m_params.folderPath, 
				m_params.fileFormat, ext);

			// create a worker thread that will run the Extract operation
			m_threadHandle = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)&CExtractDlg::ThreadProc, &m_params, 0, NULL);

			if(m_threadHandle == NULL)
				pantheios::log_ERROR(_T("CExtractDlg::OnClickedOK> Unable to create worker thread."));
			else
				::EnableWindow(::GetDlgItem(m_hWnd, IDOK), FALSE);	// disable Extract button
		}

		return 0;
	}

	LRESULT OnClickedCancel(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
	{
		// check if thread is still running
		DWORD result = WaitForSingleObject(m_threadHandle, 0);
		bool threadRunning = (result != WAIT_OBJECT_0);

		if(m_threadHandle != NULL)
		{
			if(threadRunning)
			{
				m_userCancelled = true;
				SetDlgItemText(IDCANCEL, _T("Exit"));
			}
			else
			{
				CloseHandle(m_threadHandle);
				m_threadHandle = NULL;
				EndDialog(wID);
			}
		}
		else
		{
			EndDialog(wID);
		}

		return 0;
	}

	void UpdateLinkText();
	void DisplayGroup(int groupId);
	static bool OnProgressChanged(LPVOID sender, CallbackEventArgs* e);
	static DWORD WINAPI ThreadProc(LPVOID lpParameter);

	// resizes the dialog to display additional options
	void ResizeDialog(bool expand)
	{
		RECT rect;
		GetWindowRect(&rect);
		SetWindowPos(NULL, -1, -1, rect.right - rect.left, expand ? 245 : 170, SWP_NOMOVE | SWP_NOACTIVATE | SWP_NOZORDER);
	}

	LRESULT OnNMClickLinkMolCount(int /*idCtrl*/, LPNMHDR pNMHDR, BOOL& /*bHandled*/);
	LRESULT OnUpdateOptionsClicked(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT OnChangeDataFormat(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT OnChangeOverwrite(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
};

