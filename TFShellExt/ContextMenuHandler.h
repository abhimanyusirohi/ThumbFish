#pragma once

#include "resource.h"       // main symbols
#include "ThumbFish_i.h"

class ATL_NO_VTABLE CContextMenuHandler :
    public CComObjectRootEx<CComSingleThreadModel>,
    public CComCoClass<CContextMenuHandler, &CLSID_ContextMenuHandler>,
    public IShellExtInit, 
    public IContextMenu
{
public:
    CContextMenuHandler()
    {
		pantheios::log_NOTICE(_T("CContextMenuHandler::CContextMenuHandler> ctor Called"));
    }

    DECLARE_REGISTRY_RESOURCEID(IDR_CONTEXTMENU_HANDLER)

    DECLARE_NOT_AGGREGATABLE(CContextMenuHandler)

    BEGIN_COM_MAP(CContextMenuHandler)
        COM_INTERFACE_ENTRY(IShellExtInit)
        COM_INTERFACE_ENTRY(IContextMenu)
    END_COM_MAP()

    DECLARE_PROTECT_FINAL_CONSTRUCT()

    HRESULT FinalConstruct()
    {
        return S_OK;
    }

    void FinalRelease()
    {
    }

public:

    // IShellExtInit
    IFACEMETHODIMP Initialize(LPCITEMIDLIST, LPDATAOBJECT, HKEY);

    // IContextMenu
    IFACEMETHODIMP GetCommandString(UINT_PTR, UINT, UINT*, LPSTR, UINT);
    IFACEMETHODIMP InvokeCommand(LPCMINVOKECOMMANDINFO);
    IFACEMETHODIMP QueryContextMenu(HMENU, UINT, UINT, UINT, UINT);

protected:

    // The name of the first selected file
    TCHAR m_szFileName[MAX_PATH];

    // The function that handles the "Sample" verb
    void OnSample(HWND hWnd);
	void OnThumbFishOnline();
	void OnAboutThumbFish();
};

OBJECT_ENTRY_AUTO(__uuidof(ContextMenuHandler), CContextMenuHandler)
