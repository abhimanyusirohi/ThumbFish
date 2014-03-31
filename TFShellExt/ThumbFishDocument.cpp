// ThumbFishDocument.cpp : Implementation of the ThumbFishDocument class

#include "stdafx.h"
#include <propkey.h>
#include "ThumbFishDocument.h"

const int MAX_CACHE_RECORDS = 4;
// assuming a normal record would be a maximum of 10KB
#define MAX_READ	(10 * 1024) * MAX_CACHE_RECORDS

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

	// set extension enum
	TCHAR* ext = ::PathFindExtension(m_Buffer.FileName);
	
	if(TEQUAL(ext, ".mol")) m_Buffer.FileExtension = extMOL;
	else if(TEQUAL(ext, ".mol")) m_Buffer.FileExtension = extMOL;
	else if(TEQUAL(ext, ".rxn")) m_Buffer.FileExtension = extRXN;
	else if(TEQUAL(ext, ".smi")) m_Buffer.FileExtension = extSMI;
	else if(TEQUAL(ext, ".smiles")) m_Buffer.FileExtension = extSMILES;
	else if(TEQUAL(ext, ".smarts")) m_Buffer.FileExtension = extSMARTS;
	else if(TEQUAL(ext, ".sdf")) m_Buffer.FileExtension = extSDF;
	else if(TEQUAL(ext, ".rdf")) m_Buffer.FileExtension = extRDF;
	else if(TEQUAL(ext, ".cml")) m_Buffer.FileExtension = extCML;
	else m_Buffer.FileExtension = extUnknown;

	// load stream into BUFFER if preload is set
	if(m_preLoad)
		if(!LoadStream())
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
	if(pDrawFunc != NULL)
	{
		OPTIONS options;
		pDrawFunc(hDrawDC, lprcBounds, &m_Buffer, &options);
	}
}

BOOL ThumbFishDocument::LoadStream()
{
	char recordDelimiter[20];
	memset(recordDelimiter, 0, 20);
	
	IStream* stream = (IStream*)m_Buffer.pData;

	m_Buffer.pData = NULL;	// set we have buffered data
	m_Buffer.isStream = false;

	if(m_Buffer.FileExtension == extSDF)
	{
		strcpy_s(recordDelimiter, 20, "$$$$");
	}
	else if(m_Buffer.FileExtension == extRDF)
	{
		strcpy_s(recordDelimiter, 20, "$RFMT");
	}
	else if(m_Buffer.FileExtension == extCML)
	{
		strcpy_s(recordDelimiter, 20, "</molecule>");
	}
	else
	{
		// for single molecule files such as MOL, read the whole data
		m_Buffer.pData = new char[m_Buffer.DataLength];
		return (stream->Read(m_Buffer.pData, m_Buffer.DataLength, &m_Buffer.DataLength) == S_OK);
	}

	/* the following code runs for multimol files */

	char readBuffer;
	ULONG readCount = 0;
	char largeTempBuffer[MAX_READ];
	int delimiterLen = strlen(recordDelimiter);
	int matchIndex = 0, totalRead = 0, recordCount = 0;

	// read stream char by char, identify the record delimiters and cache 
	// MAX_CACHE_RECORDS number of records
	while(stream->Read(&readBuffer, 1, &readCount) == S_OK)
	{
		largeTempBuffer[totalRead++] = readBuffer;
		if(totalRead >= MAX_READ) break;	// very big records, normally not possible but just in case

		// identify record delimiter in text
		if(readBuffer == recordDelimiter[matchIndex++])
		{
			if(matchIndex == delimiterLen) recordCount++;	// whole string matches
			if(recordCount == MAX_CACHE_RECORDS) break;		// desired number of records read
		}
		else
		{
			matchIndex = 0;
		}
	}

	m_Buffer.pData = new char[totalRead];
	m_Buffer.DataLength = totalRead;
	memcpy(m_Buffer.pData, largeTempBuffer, totalRead);
	return TRUE;
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
