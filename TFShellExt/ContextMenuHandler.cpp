#include "stdafx.h"
#include "ContextMenuHandler.h"
#include <strsafe.h>
#include "AboutDlg.h"

#pragma region IShellExtInit methods

// Initializes the context menu extension.
IFACEMETHODIMP CContextMenuHandler::Initialize(
    LPCITEMIDLIST pidlFolder, LPDATAOBJECT pDataObj, HKEY hProgID)
{
    HRESULT hr = E_INVALIDARG;
    
    if (NULL == pDataObj)
    {
        return hr;
    }

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
                // Get the name of the first file.
                if (0 != DragQueryFile(hDrop, 0, m_szFileName, MAX_PATH))
                {
                    hr = S_OK;
                }

                // [-or-]

                // Enumerates the selected files and directories.
                //wchar_t szFileName[MAX_PATH];
                //for (UINT i = 0; i < nFiles; i++)
                //{
                //    // Get the next filename.
                //    if (0 == DragQueryFile(hDrop, i, szFileName, MAX_PATH))
                //        continue;

                //    //...
                //}
                //hr = S_OK;
            }

            GlobalUnlock(stm.hGlobal);
        }

        ReleaseStgMedium(&stm);
    }

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

	// First, create and populate a submenu.
    HMENU hSubmenu = CreatePopupMenu();
	UINT id = idCmdFirst;

    InsertMenu(hSubmenu, 0, MF_BYPOSITION, id++, _T("&Online Help"));
	HBITMAP hbmHelp = LoadBitmap(_AtlBaseModule.m_hInst, MAKEINTRESOURCE(IDB_HELP));
	SetMenuItemBitmaps(hSubmenu, 0, MF_BYPOSITION, hbmHelp, hbmHelp);
	
    InsertMenu(hSubmenu, 1, MF_BYPOSITION, id++, _T("&About ThumbFish"));

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

    if (idCommand == 0)
    {
		char helpTextA[] = "Displays the online help page for ThumbFish";
		wchar_t helpTextW[] = L"Displays the online help page for ThumbFish";

        switch (uFlags)
        {
        case GCS_HELPTEXTA:
            hr = StringCchCopyNA(pszName, uMaxNameLen, helpTextA, lstrlenA(helpTextA));
            break;

        case GCS_HELPTEXTW:
            hr = StringCchCopyNW((LPWSTR)pszName, uMaxNameLen, helpTextW, lstrlenW((LPWSTR)helpTextW));
            break;

        default:
            hr = S_OK;
        }
    }
	else if(idCommand = 1)
	{
		char helpTextA[] = "Displays the About dialog";
		wchar_t helpTextW[] = L"Displays the About dialog";

        switch (uFlags)
        {
        case GCS_HELPTEXTA:
            hr = StringCchCopyNA(pszName, uMaxNameLen, helpTextA, lstrlenA(helpTextA));
            break;

        case GCS_HELPTEXTW:
            hr = StringCchCopyNW((LPWSTR)pszName, uMaxNameLen, helpTextW, lstrlenW((LPWSTR)helpTextW));
            break;

        default:
            hr = S_OK;
        }
	}

    // If the command (idCommand) is not supported by this context menu 
    // extension handler, return E_INVALIDARG.

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

    // If the command cannot be identified through the verb string, then 
    // check the identifier offset.
	if (LOWORD(lpcmi->lpVerb) == 0)
    {
        OnThumbFishOnline();
    }
	else if (LOWORD(lpcmi->lpVerb) == 1)
	{
		OnAboutThumbFish();
	}
    else
    {
        // If the verb is not recognized by the context menu handler, it 
        // must return E_FAIL to allow it to be passed on to the other 
        // context menu handlers that might implement that verb.
        return E_FAIL;
    }

    return S_OK;
}

#pragma endregion
