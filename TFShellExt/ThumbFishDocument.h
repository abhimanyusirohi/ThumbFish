// ThumbFishDocument.h : Declaration of the ThumbFishDocument class

#pragma once

#include <atlhandlerimpl.h>
#include "Types.h"

using namespace ATL;

#define MAX_READ	20*1024	// max 20KB read

class ThumbFishDocument : public CAtlDocumentImpl
{
public:
	BOOL		m_preLoad;
	BUFFER		m_Buffer;
	HANDLE		m_hEventLog;

public:
	ThumbFishDocument(void):m_preLoad(FALSE)
	{
		m_Buffer.pData = NULL;
		m_Buffer.DataLength = 0;
		m_Buffer.isStream = false;

		m_hEventLog = NULL;
	}

	ThumbFishDocument(HANDLE eventlog) : m_preLoad(FALSE)
	{
		m_Buffer.pData = NULL;
		m_Buffer.DataLength = 0;
		m_Buffer.isStream = false;

		m_hEventLog = eventlog;
	}

	virtual ~ThumbFishDocument(void)
	{
		if(m_Buffer.DataLength > 0) 
		{
			// do not delete IStream pointer
			if(m_Buffer.pData != NULL && !m_Buffer.isStream) 
				delete m_Buffer.pData;
		}
	}

	virtual HRESULT LoadFromStream(IStream* pStream, DWORD grfMode);
	virtual void InitializeSearchContent();

	BOOL Load(IStream* stream, ULONG size);

protected:
	void SetSearchContent(CString& value);
	virtual BOOL GetThumbnail(UINT, HBITMAP*, WTS_ALPHATYPE*);
	virtual void OnDrawThumbnail(HDC hDrawDC, LPRECT lprcBounds);
};
