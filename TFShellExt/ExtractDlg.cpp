// ExtractDlg.cpp : Implementation of CExtractDlg

#include "stdafx.h"
#include "ExtractDlg.h"

bool CExtractDlg::m_userCancelled = false;

LRESULT CExtractDlg::OnNMClickLinkMolCount(int idCtrl, LPNMHDR pNMHDR, BOOL& bHandled)
{
	PNMLINK link = (PNMLINK) pNMHDR;
	switch(link->item.iLink)
	{
	case 0:		// Extract Count
		DisplayGroup(1);
		break;

	case 1:		// Select Output Directory
		if(!SUCCEEDED(Utils::DoFolderDialog(m_hWnd, _T("Extract Molecules to Folder..."), 
			m_params.folderPath, m_params.folderPath)))
		{
			//TODO: log
		}
		break;

	case 3:		// File Format
		DisplayGroup(2);
		break;

	case 2:
	case 4:
		POINT point;
		if(GetCursorPos(&point))
		{
			HMENU hMenu = LoadMenu(_AtlBaseModule.m_hInst, MAKEINTRESOURCE(IDR_EXTRACTMENU));
			if(hMenu)
			{
				int subMenuId = (link->item.iLink == 2) ? 0 : 1;
				HMENU hSubMenu = GetSubMenu(hMenu, subMenuId);

				//WORKAROUND: To be removed when provider issues with RXN -> CML, CDXML are fixed
				// disable CML,CDXML if source is reaction to avoid crash due to provider issue
				if((m_params.exportFormat == fmtRXNV2) || (m_params.exportFormat == fmtRXNV3))
				{
					EnableMenuItem(hSubMenu, ID_EXTRACTFORMAT_CML, MF_DISABLED);
					EnableMenuItem(hSubMenu, ID_EXTRACTFORMAT_CDXML, MF_DISABLED);
				}

				TrackPopupMenuEx(hSubMenu, TPM_LEFTALIGN, point.x, point.y, m_hWnd, NULL);
				DestroyMenu(hMenu);
			}
		}
		break;
	}

	UpdateLinkText();

	return 0;
}

LRESULT CExtractDlg::OnChangeDataFormat(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
	switch(wID)
	{
		case ID_EXTRACTFORMAT_MOL: m_params.exportFormat = fmtMOLV2; break;
		case ID_EXTRACTFORMAT_MOLV3000: m_params.exportFormat = fmtMOLV3; break;
		case ID_EXTRACTFORMAT_RXN: m_params.exportFormat = fmtRXNV2; break;
		case ID_EXTRACTFORMAT_RXNV3000: m_params.exportFormat = fmtRXNV3; break;
		case ID_EXTRACTFORMAT_CML: m_params.exportFormat = fmtCML; break;
		case ID_EXTRACTFORMAT_EMF: m_params.exportFormat = fmtEMF; break;
		case ID_EXTRACTFORMAT_CDXML: m_params.exportFormat = fmtCDXML; break;
	}

	UpdateLinkText();
	return 0;
}

LRESULT CExtractDlg::OnChangeOverwrite(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
	m_params.overwriteFiles = (wID == ID_OVERWRITE_OVERWRITTEN) ? true : false;
	UpdateLinkText();
	return 0;
}

LRESULT CExtractDlg::OnUpdateOptionsClicked(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
	if(m_currentGroup > 0)
	{
		if(m_currentGroup == 1)			// Extract Molecule Count
		{
			if(IsDlgButtonChecked(IDC_RDO_EXTRACTALL) == BST_CHECKED)
				m_params.extractMolCount = -1;
			else
			{
				int val = GetDlgItemInt(IDC_EDTEXTRACTCOUNT, NULL, false);

				if(val <= 0)
				{
					MessageBox(_T("Value must be greater than 0."), _T("Validation Failed"), MB_OK | MB_ICONWARNING);
					return 0;
				}

				m_params.extractMolCount = val;
			}

		}
		else if(m_currentGroup == 2)	// File Format
		{
			TCHAR format[50];
			GetDlgItemText(IDC_EDT_FILEFORMAT, format, 50);

			size_t start = 0;
			int countPercentD = 0;
			tstring strFormat(format);

			// count the number of %d in format string
			while ((start = strFormat.find(_T("%d"), start)) != std::string::npos) {
				++countPercentD; start += 2;
			}

			// count the number of % sign in format string
			size_t countPercents = std::count(strFormat.begin(), strFormat.end(), _T('%'));

			// make sure we have exactly ONE %d in the format
			if((countPercentD != 1) || (countPercents > 1))
			{
				MessageBox(_T("Format is not correct. Format string must contain exactly one %d which will be replaced by a number during extract operation."),
					_T("Validation Failed"), MB_OK | MB_ICONWARNING);
				return 0;
			}

			// looks ok, copy to main variable
			_tcscpy_s(m_params.fileFormat, 50, format);
		}

		RECT rect;
		GetWindowRect(&rect);
		SetWindowPos(NULL, -1, -1, rect.right - rect.left, 170, SWP_NOMOVE | SWP_NOACTIVATE | SWP_NOZORDER);

		m_currentGroup = -1; // no active group
		UpdateLinkText();
	}

	return 0;
}

void CExtractDlg::DisplayGroup(int groupId)
{
	ResizeDialog(true);

	m_currentGroup = groupId;
	bool molCountGroup = (groupId == 1);

	SetDlgItemText(IDC_EXTRAOPTIONS, molCountGroup
		? _T("Extract Count Options")
		: _T("File Format Options"));

	::ShowWindow(GetDlgItem(IDC_RDO_EXTRACTALL), molCountGroup ? SW_SHOW : SW_HIDE);
	::ShowWindow(GetDlgItem(IDC_RDO_EXTRACTSOME), molCountGroup ? SW_SHOW : SW_HIDE);
	::ShowWindow(GetDlgItem(IDC_EDTEXTRACTCOUNT), molCountGroup ? SW_SHOW : SW_HIDE);
	::ShowWindow(GetDlgItem(IDC_STATIC_EXTRACTMOL), molCountGroup ? SW_SHOW : SW_HIDE);
	::ShowWindow(GetDlgItem(IDC_LBL_FILEFORMAT), !molCountGroup ? SW_SHOW : SW_HIDE);
	::ShowWindow(GetDlgItem(IDC_EDT_FILEFORMAT), !molCountGroup ? SW_SHOW : SW_HIDE);

	// set focus to the textboxes
	if(molCountGroup)
	{
		if(IsDlgButtonChecked(IDC_RDO_EXTRACTSOME) == BST_CHECKED)
		{
			::SetFocus(GetDlgItem(IDC_EDTEXTRACTCOUNT));
		}
	}
	else
	{
		::SetFocus(GetDlgItem(IDC_EDT_FILEFORMAT));
	}
}

void CExtractDlg::UpdateLinkText()
{
	TCHAR formatStatement[512];
	TCHAR finalStatement[1024];

	if(LoadString(_AtlBaseModule.m_hInst, IDS_EXTRACT_STATEMENT, formatStatement, 512) > 0)
	{
		TCHAR strMolCount[10];
		m_params.GetStrMolCount(strMolCount, 10);

		TCHAR dataFormat[10];
		CommonUtils::GetFormatString(m_params.exportFormat, dataFormat, 10);

		TCHAR overwrite[15];
		m_params.GetStrOverwriteFiles(overwrite, 15);

		_snwprintf_s(finalStatement, 1024, formatStatement, 
			strMolCount, m_params.folderPath, dataFormat, m_params.fileFormat, overwrite);

		SetDlgItemText(IDC_LINKMAIN, finalStatement);
	}
}

bool CExtractDlg::OnProgressChanged(LPVOID sender, CallbackEventArgs* e)
{
	HWND hWndDlg = ((CExtractDlg*)sender)->m_hWnd;

	switch(e->type)
	{
		case progressWorking:
			TCHAR msg[50];
			_sntprintf_s(msg, 50, _T("Processed %d"), e->processed);
			::SetDlgItemText(hWndDlg, IDC_EXTRACTMSG, msg);
			break;

		case progressDone:
			// clear the progress message
			::SetDlgItemText(hWndDlg, IDC_EXTRACTMSG, _T(""));

			// display summary when done
			if(::MessageBox(hWndDlg, e->message, 
				m_userCancelled ? _T("Extraction Cancelled") : _T("Extraction Completed"), 
				MB_YESNO | MB_ICONINFORMATION) == IDYES)
			{
				TCHAR folder[MAX_PATH];

				// copy to a local variable and remove file part from it
				// params::folderPath contains "[folderPath]\[file%d].[ext]"
				if(_tcscpy_s(folder, MAX_PATH, ((CExtractDlg*)sender)->m_params.folderPath) == 0)
				{
					::PathRemoveFileSpec(folder);
					int errCode = (int)ShellExecute(NULL, _T("open"), folder, NULL, NULL, SW_SHOWDEFAULT);
					if(errCode <= 32)
						pantheios::log_ERROR(_T("CExtractDlg::OnProgressChanged> ShellExecute failed. Error Code="), 
										pantheios::integer(errCode));
				}
				else
				{
					pantheios::log_ERROR(_T("CExtractDlg::OnProgressChanged> _tcscpy_s failed. folderPath="),
						((CExtractDlg*)sender)->m_params.folderPath);
				}
			}
			break;
	}

	return m_userCancelled;	
}

DWORD WINAPI CExtractDlg::ThreadProc(LPVOID lpParameter)
{
	OPTIONS options;
	ExtractParams* exParams = (ExtractParams*)lpParameter;

	COMMANDPARAMS params(cmdExtract, NULL, exParams);
	std::auto_ptr<OUTBUFFER> outBuffer(pExecuteFunc(&params, &options));

	return 0;
}
