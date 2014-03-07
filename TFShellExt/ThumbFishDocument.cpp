// ThumbFishDocument.cpp : Implementation of the ThumbFishDocument class

#include "stdafx.h"
#include <propkey.h>
#include "ThumbFishDocument.h"

HRESULT ThumbFishDocument::LoadFromStream(IStream* pStream, DWORD grfMode)
{
	STATSTG stat;

	// get stream properties
	pStream->Stat(&stat, STATFLAG_DEFAULT);

#if _DEBUG
	//_msgLogger->LogMessage(_T("ThumbFishDocument::LoadFromStream"), _T("called"))
	//	<< _T("IStream> Name: ") << stat.pwcsName << _T(", Size: ") 
	//	<< MAKELONG(stat.cbSize.LowPart, stat.cbSize.HighPart) << std::endl;
#endif

	// initialize BUFFER
	m_Buffer.pData = pStream;
	m_Buffer.isStream = true;
	m_Buffer.DataLength = MAKELONG(stat.cbSize.LowPart, stat.cbSize.HighPart);
	_tcscpy_s(m_Buffer.FileName, MAX_PATH, OLE2W(stat.pwcsName));

	// load stream into BUFFER if preload is set
	if(m_preLoad)
		if(!Load(pStream, m_Buffer.DataLength))
			pantheios::log_ERROR(_T("ThumbFishDocument::LoadFromStream> Could not preload data."),
					m_Buffer.FileName, _T(", DataLength"), pantheios::integer(m_Buffer.DataLength));
	return S_OK;
}

void ThumbFishDocument::InitializeSearchContent()
{
	//CString value;
	//if(pGetPropsFunc != NULL)
	//{
	//	LPPROPS props = pGetPropsFunc(&m_Buffer, &_filterOptions);
	//	if(props != NULL)
	//	{
	//		value.AppendFormat(_T("n_%ls;name_%ls;"), props->MolName, props->MolName);
	//		value.AppendFormat(_T("fmt_%ls;format_%ls;"), props->Format, props->Format);
	//		value.AppendFormat(_T("f_%ls;formula_%ls;"), props->Formula, props->Formula);
	//		value.AppendFormat(_T("m_%f;mass_%f;"), props->MolWeight, props->MolWeight);
	//		value.AppendFormat(_T("s_%ls;smiles_%ls;"), props->SMILES, props->SMILES);
	//		value.AppendFormat(_T("i_%ls;inchi_%ls;"), props->InChI, props->InChI);
	//		value.AppendFormat(_T("ik_%ls;inchikey_%ls;"), props->InChIKey, props->InChIKey);
	//		value.AppendFormat(_T("na_%d;numatoms_%d;"), props->NumAtoms, props->NumAtoms);
	//		value.AppendFormat(_T("hbd_%d;hbdonor_%d;"), props->HBDonor, props->HBDonor);
	//		value.AppendFormat(_T("hba_%d;hbacceptor_%d;"), props->HBAcceptor, props->HBAcceptor);
	//	}
	//	else
	//	{
	//		LOG_WARNING1(_T("ThumbFishDocument::InitializeSearchContent"), _T("Unable to get properties."),
	//			m_Buffer.StreamName, m_Buffer.DataLength);
	//	}
	//}

	//SetSearchContent(value);
}

void ThumbFishDocument::SetSearchContent(CString& value)
{
	// Assigns search content to PKEY_Search_Contents key
	if (value.IsEmpty())
	{
		RemoveChunk(PKEY_Search_Contents.fmtid, PKEY_Search_Contents.pid);
	}
	else
	{
		CFilterChunkValueImpl *pChunk = NULL;
		ATLTRY(pChunk = new CFilterChunkValueImpl);
		if (pChunk != NULL)
		{
			pChunk->SetTextValue(PKEY_Search_Contents, value, CHUNK_TEXT);
			SetChunkValue(pChunk);
		}
	}
}

void ThumbFishDocument::OnDrawThumbnail(HDC hDrawDC, LPRECT lprcBounds)
{
	//if(pDrawThumbnailFunc != NULL)
	//	pDrawThumbnailFunc(hDrawDC, lprcBounds, &m_Buffer, &_thumbnailOptions, false);
}

BOOL ThumbFishDocument::Load(IStream* stream, ULONG size)
{
	// to avoid loading very large files
	if(size > MAX_READ) size = MAX_READ;

	// read IStream data into a char buffer
	m_Buffer.pData = NULL;	// we have buffered data
	m_Buffer.pData = new char[size];
	m_Buffer.isStream = false;
	if(stream->Read(m_Buffer.pData, size, &m_Buffer.DataLength) == S_OK)
		return TRUE;
	else
		return FALSE;
}

/// <summary>
/// This overload is needed to fix ATL's default monochrome bitmap
/// More Info: http://www.patthoyts.tk/blog/fixing-the-atl-ithumbnailprovider-implementation.html
/// </summary>
BOOL ThumbFishDocument::GetThumbnail(_In_ UINT cx, _Out_ HBITMAP* phbmp, _In_opt_ WTS_ALPHATYPE* /* pdwAlpha */)
{
    BOOL br = FALSE;
    HDC hdc = ::GetDC(NULL);
    HDC hDrawDC = CreateCompatibleDC(hdc);
    if (hDrawDC != NULL)
    {
        void *bits = 0;
        RECT rcBounds;
        SetRect(&rcBounds, 0, 0, cx, cx);

        BITMAPINFO bi = {0};
        bi.bmiHeader.biWidth = cx;
        bi.bmiHeader.biHeight = cx;
        bi.bmiHeader.biPlanes = 1;
        bi.bmiHeader.biBitCount = 32;
        bi.bmiHeader.biSizeImage = 0;
        bi.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
        bi.bmiHeader.biClrUsed = 0;
        bi.bmiHeader.biClrImportant = 0;

        HBITMAP hBmp = CreateDIBSection(hdc, &bi, DIB_RGB_COLORS, &bits, NULL, 0);
        if (hBmp != NULL)
        {
            HBITMAP hOldBitmap = (HBITMAP)SelectObject(hDrawDC, hBmp);
            OnDrawThumbnail(hDrawDC, &rcBounds);
            SelectObject(hDrawDC, hOldBitmap);
            *phbmp = hBmp;
            br = TRUE;
        }
        DeleteDC(hDrawDC);
    }
    ReleaseDC(NULL, hdc);
    return br;
}
