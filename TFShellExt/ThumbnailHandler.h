// ThumbnailHandler.h : Declaration of the thumbnail handler

#pragma once
#include "resource.h"       // main symbols

#define AFX_PREVIEW_STANDALONE
#include <atlhandler.h>
#include <atlhandlerimpl.h>
#include "ThumbFishDocument.h"

#include "ThumbFish_i.h"

using namespace ATL;

// CThumbnailHandler

class ATL_NO_VTABLE CThumbnailHandler :
	public CComObjectRootEx<CComSingleThreadModel>,
	public CComCoClass<CThumbnailHandler, &CLSID_Thumbnail>,
	public CThumbnailProviderImpl
{
public:
	CThumbnailHandler()
	{
		//LOG_INFO(_T("CThumbnailHandler"), _T("ctor called."));
	}

DECLARE_REGISTRY_RESOURCEID(IDR_THUMBNAIL_HANDLER)
DECLARE_NOT_AGGREGATABLE(CThumbnailHandler)

BEGIN_COM_MAP(CThumbnailHandler)
	COM_INTERFACE_ENTRY(IInitializeWithStream)
	COM_INTERFACE_ENTRY(IThumbnailProvider)
END_COM_MAP()

	DECLARE_PROTECT_FINAL_CONSTRUCT()

	HRESULT FinalConstruct()
	{
		return S_OK;
	}

	void FinalRelease()
	{
		CThumbnailProviderImpl::FinalRelease();
	}

protected:
	virtual HRESULT GetBitmap(UINT cx, HBITMAP *phbmp, WTS_ALPHATYPE *pdwAlpha)
	{
		if (m_pDocument == NULL)
		{
			return E_NOTIMPL;
		}

		// Implement OnDrawThumbnail in IDocument derived class
		static int const nDocDimensions = 256;
		if (!m_pDocument->GetThumbnail(cx, phbmp, pdwAlpha))
		{
			return E_FAIL;
		}

		return S_OK;
	}

	virtual IDocument* CreateDocument()
	{
		ThumbFishDocument *pDocument = NULL;
		ATLTRY(pDocument = new ThumbFishDocument());
		return pDocument;
	}
};

OBJECT_ENTRY_AUTO(__uuidof(Thumbnail), CThumbnailHandler)
