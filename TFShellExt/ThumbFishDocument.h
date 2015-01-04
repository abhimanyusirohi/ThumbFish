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
	ThumbFishDocument(void):m_preLoad(FALSE) {}

	virtual ~ThumbFishDocument(void) {}

	virtual HRESULT LoadFromStream(IStream* pStream, DWORD grfMode);
	virtual void InitializeSearchContent();

	BOOL LoadStream(IStream* stream);

protected:
	void SetSearchContent(CString& value);
	virtual BOOL GetThumbnail(UINT, HBITMAP*, WTS_ALPHATYPE*);
	virtual void OnDrawThumbnail(HDC hDrawDC, LPRECT lprcBounds);
	int FindMolVersion(PCHAR data, size_t dataLength, int lineNum);
};
