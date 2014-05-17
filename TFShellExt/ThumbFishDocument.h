// ThumbFishDocument.h : Declaration of the ThumbFishDocument class

#pragma once

#include <atlhandlerimpl.h>
#include "Types.h"

using namespace ATL;

class ThumbFishDocument : public CAtlDocumentImpl
{
public:
	BOOL		m_preLoad;
	BUFFER		m_Buffer;

public:
	ThumbFishDocument(void):m_preLoad(FALSE)
	{
		m_Buffer.pData = NULL;
		m_Buffer.DataLength = 0;
	}

	virtual ~ThumbFishDocument(void)
	{
		if(m_Buffer.DataLength > 0) 
		{
			// do not delete IStream pointer
			if(m_Buffer.pData != NULL) 
				delete[] m_Buffer.pData;
		}
	}

	virtual HRESULT LoadFromStream(IStream* pStream, DWORD grfMode);
	virtual void InitializeSearchContent();

	BOOL LoadStream(IStream* stream);

protected:
	void SetSearchContent(CString& value);
	virtual BOOL GetThumbnail(UINT, HBITMAP*, WTS_ALPHATYPE*);
	virtual void OnDrawThumbnail(HDC hDrawDC, LPRECT lprcBounds);
};
