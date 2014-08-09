#include "stdafx.h"
#include "ContextMenuHandler.h"
#include <strsafe.h>
#include "AboutDlg.h"

typedef struct
{
	char* helpTextA;
	wchar_t* helpTextW;
} HELPTEXT;

const HELPTEXT c_HelpStrings[] = 
{
		{"Save selected structure to disk in different formats", L"Save selected structure to disk in different formats"},
		{"Copy structure in CDXML format", L"Copy structure in CDXML format"},
		{"Copy structure in CML format", L"Copy structure in CML format"},
		{"Copy structure in EMF format", L"Copy structure in EMF format"},
		{"Copy structure in InChi format", L"Copy structure in InChi format"},
		{"Copy structure in InChi Key format", L"Copy structure in InChi Key format"},
		{"Copy structure in MOLV2000 format", L"Copy structure in MOLV2000 format"},
		{"Copy structure in MOLV3000 format", L"Copy structure in MOLV3000 format"},
		{"Copy structure in RXNV2000 format", L"Copy structure in RXNV2000 format"},
		{"Copy structure in RXNV3000 format", L"Copy structure in RXNV3000 format"},
		{"Copy structure in SMILES format", L"Copy structure in SMILES format"},
		{"Copy selected structure to clipboard in different formats", L"Copy selected structure to clipboard in different formats"},
		{"Displays the online help page for ThumbFish", L"Displays the online help page for ThumbFish"},
		{"Displays the About dialog", L"Displays the About dialog"}
};

#pragma region IShellExtInit methods

// Initializes the context menu extension.
IFACEMETHODIMP CContextMenuHandler::Initialize(
    LPCITEMIDLIST pidlFolder, LPDATAOBJECT pDataObj, HKEY hProgID)
{
    HRESULT hr = E_INVALIDARG;
    if (NULL == pDataObj) return hr;

    FORMATETC fe = { CF_HDROP, NULL, DVASPECT_CONTENT, -1, TYMED_HGLOBAL };
    STGMEDIUM stm;

    // pDataObj contains the objects being acted upon. In this example, 
    // we get an HDROP handle for enumerating the selected files.
    if (SUCCEEDED(pDataObj->GetData(&fe, &stm)))
    {
        // Get an HDROP handle.
        HDROP hDrop = static_cast<HDROP>(GlobalLock(stm.hGlobal));
        if (hDrop != NULL)
        {
            // Determine how many files are involved in this operation.
            UINT nFiles = DragQueryFile(hDrop, 0xFFFFFFFF, NULL, 0);
            if (nFiles != 0)
            {
                // Enumerates the selected files and directories.
                for (UINT i = 0; i < nFiles; i++)
                {
					TCHAR* szFileName = new TCHAR[MAX_PATH];
                    if (DragQueryFile(hDrop, i, szFileName, MAX_PATH) != 0)
						m_Files.push_back(szFileName);
                }

                hr = S_OK;
            }

            GlobalUnlock(stm.hGlobal);
        }

        ReleaseStgMedium(&stm);
    }

	m_bMultiSelection = (m_Files.size() > 1);

    // If any value other than S_OK is returned from the method, the context 
    // menu is not displayed.
    return hr;
}

#pragma endregion

#pragma region Command Handlers

void CContextMenuHandler::OnThumbFishOnline()
{
	pantheios::log_NOTICE(_T("CContextMenuHandler::OnThumbFishOnline> Called"));

	Utils::ShellExecuteLink(c_OnlineLink);
}

void CContextMenuHandler::OnAboutThumbFish()
{
	pantheios::log_NOTICE(_T("CContextMenuHandler::OnAboutThumbFish> Called"));

	AboutDlg dlg;
	dlg.DoModal(NULL);
}

#pragma endregion

#pragma region IContextMenu methods

// The Shell calls IContextMenu::QueryContextMenu to allow the context menu handler to add 
// its menu items to the menu. It passes in the HMENU handle in the hmenu parameter. 
// The indexMenu parameter is set to the index to be used for the first menu item that is to be added.
IFACEMETHODIMP CContextMenuHandler::QueryContextMenu(
    HMENU hMenu, UINT indexMenu, UINT idCmdFirst, UINT idCmdLast, UINT uFlags)
{
    // If uFlags include CMF_DEFAULTONLY then we should not do anything
    if (CMF_DEFAULTONLY & uFlags)
    {
        return MAKE_HRESULT(SEVERITY_SUCCESS, 0, USHORT(0));
    }

	bool multiMolFile = false;
	ChemFormat format = fmtUnknown;
	if(m_Files.size() == 1) 
	{
		multiMolFile = Utils::IsMultiMolFile(m_Files[0]);
		format = CommonUtils::GetFormatFromFileName(m_Files[0]);
	}

	// First, create and populate a submenu.
    HMENU hSubmenu = CreatePopupMenu();
	UINT id = idCmdFirst;

	// -- Save Structure
	InsertMenu(hSubmenu, 0, MF_BYPOSITION | ((!m_bMultiSelection && !multiMolFile) ? MF_ENABLED : MF_DISABLED), 
		id++, _T("&Save Structure..."));
	HBITMAP hbmSave = LoadBitmap(_AtlBaseModule.m_hInst, MAKEINTRESOURCE(IDB_SAVE));
	SetMenuItemBitmaps(hSubmenu, 0, MF_BYPOSITION, hbmSave, hbmSave);

	// -- Copy Structure As
	#pragma region Create CopyAs SubMenu

	HMENU hCopyAsMenu = Utils::CreateCopyMenu(format, &id);

	MENUITEMINFO miiCopyAs = { sizeof(MENUITEMINFO) };
	miiCopyAs.fMask = MIIM_SUBMENU | MIIM_STRING | MIIM_ID | MIIM_BITMAP | MIIM_STATE;
	miiCopyAs.wID = id++;
    miiCopyAs.hSubMenu = hCopyAsMenu;
    miiCopyAs.dwTypeData = _T("&Copy Structure As");
	miiCopyAs.hbmpItem = LoadBitmap(_AtlBaseModule.m_hInst, MAKEINTRESOURCE(IDB_COPY));
	miiCopyAs.fState = (!m_bMultiSelection && !multiMolFile) ? MFS_ENABLED : MFS_DISABLED;
    InsertMenuItem (hSubmenu, 1, TRUE, &miiCopyAs);

	#pragma endregion

	// separator
	InsertMenu(hSubmenu, 2, MF_BYPOSITION | MF_SEPARATOR, 0, NULL);

	// -- Online Help
    InsertMenu(hSubmenu, 3, MF_BYPOSITION, id++, _T("&Online Help"));
	HBITMAP hbmHelp = LoadBitmap(_AtlBaseModule.m_hInst, MAKEINTRESOURCE(IDB_HELP));
	SetMenuItemBitmaps(hSubmenu, 2, MF_BYPOSITION, hbmHelp, hbmHelp);
	
	// -- About ThumbFish
    InsertMenu(hSubmenu, 4, MF_BYPOSITION, id++, _T("&About ThumbFish"));

	// Insert the submenu into the ctx menu provided by Explorer.
	MENUITEMINFO mii = { sizeof(MENUITEMINFO) };

    mii.fMask = MIIM_SUBMENU | MIIM_STRING | MIIM_ID | MIIM_BITMAP;
	mii.wID = id++;
    mii.hSubMenu = hSubmenu;
    mii.dwTypeData = _T("&ThumbFish");
	mii.hbmpItem = LoadBitmap(_AtlBaseModule.m_hInst, MAKEINTRESOURCE(IDB_THUMBFISH));

    InsertMenuItem (hMenu, indexMenu, TRUE, &mii);

    // Return an HRESULT value with the severity set to SEVERITY_SUCCESS. 
    // Set the code value to the offset of the largest command identifier 
    // that was assigned, plus one (1)
	return MAKE_HRESULT(SEVERITY_SUCCESS, FACILITY_NULL, id - idCmdFirst);
}

// If a user highlights one of the items added by a context menu handler, the handler's 
// IContextMenu::GetCommandString method is called to request a Help text string that will 
// be displayed on the Windows Explorer status bar. This method can also be called to 
// request the verb string that is assigned to a command. Either ANSI or Unicode verb 
// strings can be requested.
IFACEMETHODIMP CContextMenuHandler::GetCommandString(
    UINT_PTR idCommand, UINT uFlags, LPUINT lpReserved, LPSTR pszName, 
    UINT uMaxNameLen)
{
    HRESULT hr = E_INVALIDARG;

	int nHelpStrings = (sizeof(c_HelpStrings)/sizeof(HELPTEXT));
	if(idCommand >= nHelpStrings) return hr;

    switch (uFlags)
    {
		case GCS_HELPTEXTA:
			hr = StringCchCopyNA(pszName, uMaxNameLen, c_HelpStrings[idCommand].helpTextA, lstrlenA(c_HelpStrings[idCommand].helpTextA));
			break;

		case GCS_HELPTEXTW:
			hr = StringCchCopyNW((LPWSTR)pszName, uMaxNameLen, c_HelpStrings[idCommand].helpTextW, lstrlenW((LPWSTR)c_HelpStrings[idCommand].helpTextW));
			break;

		default:
			hr = S_OK;
	}

    return hr;
}

// This method is called when a user clicks a menu item to tell the handler to run the 
// associated command. The lpcmi parameter points to a structure that contains the needed information.
IFACEMETHODIMP CContextMenuHandler::InvokeCommand(LPCMINVOKECOMMANDINFO lpcmi)
{
    BOOL fEx = FALSE;
    BOOL fUnicode = FALSE;

    // Determines which structure is being passed in, CMINVOKECOMMANDINFO or 
    // CMINVOKECOMMANDINFOEX based on the cbSize member of lpcmi. Although 
    // the lpcmi parameter is declared in Shlobj.h as a CMINVOKECOMMANDINFO 
    // structure, in practice it often points to a CMINVOKECOMMANDINFOEX 
    // structure. This struct is an extended version of CMINVOKECOMMANDINFO 
    // and has additional members that allow Unicode strings to be passed.
    if (lpcmi->cbSize == sizeof(CMINVOKECOMMANDINFOEX))
    {
        fEx = TRUE;
        if((lpcmi->fMask & CMIC_MASK_UNICODE))
        {
            fUnicode = TRUE;
        }
    }

	int commandID = LOWORD(lpcmi->lpVerb);
	if(commandID == 12)			// Open ThumbFish Online webpage
	{
		OnThumbFishOnline();
	}
	else if(commandID == 13)	// About ThumbFish dialog
	{
		OnAboutThumbFish();
	}
	else
	{
		ThumbFishDocument doc;
		IStream* dataStream = NULL;
		if(SUCCEEDED(SHCreateStreamOnFileEx(m_Files[0], STGM_READ | STGM_SHARE_DENY_NONE, FILE_ATTRIBUTE_NORMAL, FALSE, NULL, &dataStream)))
		{
			if(SUCCEEDED(doc.LoadFromStream(dataStream, STGM_READ | STGM_SHARE_DENY_NONE)) && (doc.m_Buffer.DataLength > 0))
			{
				OPTIONS options;
				ChemFormat format = fmtUnknown;

				switch(commandID)
				{
				case 0:	// Save Structure
					Utils::DoSaveStructure(NULL, &doc.m_Buffer, &options);
					break;

				case 1:
					if(format == fmtUnknown) format = fmtCDXML;
				case 2:
					if(format == fmtUnknown) format = fmtCML;
				case 3:
					if(format == fmtUnknown) format = fmtEMF;
				case 4:
					if(format == fmtUnknown) format = fmtINCHI;
				case 5:
					if(format == fmtUnknown) format = fmtINCHIKEY;
				case 6:
					if(format == fmtUnknown) format = fmtMOLV2;
				case 7:
					if(format == fmtUnknown) format = fmtMOLV3;
				case 8:
					if(format == fmtUnknown) format = fmtRXNV2;
				case 9:
					if(format == fmtUnknown) format = fmtRXNV3;
				case 10:
					if(format == fmtUnknown) format = fmtSMILES;

					if(format != fmtUnknown)
					{
						// convert the structure in document buffer to required format
						Utils::ConvertAndCopy(&doc.m_Buffer, format, &options);
					}
					else
					{
						pantheios::log_ERROR(_T("CContextMenuHandler::InvokeCommand> Called for Unknown format."));
					}
					break;

				default:
					return E_FAIL;
				}
			}
		}
		else
		{
			pantheios::log_ERROR(_T("CContextMenuHandler::InvokeCommand> SHCreateStreamOnFileEx() method FAILED. File= "),
				m_Files[0]);
		}
	}

    return S_OK;
}

#pragma endregion
