// PreviewHandler.h : Declaration of the preview handler

#pragma once
#include "stdafx.h"
#include "resource.h"       // main symbols

#include <atlhandler.h>
#include <atlhandlerimpl.h>
#include "ThumbFishDocument.h"
#include "PreviewCtrl.h"

#include "ThumbFish_i.h"

using namespace ATL;

class ATL_NO_VTABLE CPreviewHandler :
	public CComObjectRootEx<CComSingleThreadModel>,
	public CComCoClass<CPreviewHandler, &CLSID_Preview>,
	public CPreviewHandlerImpl <CPreviewHandler>
{
public:
	CPreviewHandler()
	{
		m_pDocument = NULL;
		pantheios::log_NOTICE(_T("CPreviewHandler::CPreviewHandler> ctor Called"));
	}

DECLARE_REGISTRY_RESOURCEID(IDR_PREVIEW_HANDLER)
DECLARE_NOT_AGGREGATABLE(CPreviewHandler)

BEGIN_COM_MAP(CPreviewHandler)
	COM_INTERFACE_ENTRY(IObjectWithSite)
	COM_INTERFACE_ENTRY(IOleWindow)
	COM_INTERFACE_ENTRY(IInitializeWithStream)
	COM_INTERFACE_ENTRY(IPreviewHandler)
	COM_INTERFACE_ENTRY(IPreviewHandlerVisuals)
END_COM_MAP()

	DECLARE_PROTECT_FINAL_CONSTRUCT()

	HRESULT FinalConstruct()
	{
		return S_OK;
	}

	void FinalRelease()
	{
		CPreviewHandlerImpl<CPreviewHandler>::FinalRelease();
	}

protected:
	virtual IPreviewCtrl* CreatePreviewControl()
	{
		pantheios::log_NOTICE(_T("CPreviewHandler::CreatePreviewControl> Called"));

		// create the preview control
		CPreviewCtrl *pPreviewCtrl = NULL;
		ATLTRY(pPreviewCtrl = new CPreviewCtrl());
		return pPreviewCtrl;
	}

	virtual IDocument* CreateDocument()
	{
		pantheios::log_NOTICE(_T("CPreviewHandler::CreateDocument> Called"));

		// create document
		ThumbFishDocument *pDocument = NULL;
		ATLTRY(pDocument = new ThumbFishDocument());

		// preload in case of Preview because the IStream pointer is released
		// before window is repainted, so we dont get a valid IStream pointer
		// in CPreviewCtrl::DoPaint()
		pDocument->m_preLoad = TRUE;

		return pDocument;
	}

};

OBJECT_ENTRY_AUTO(__uuidof(Preview), CPreviewHandler)
