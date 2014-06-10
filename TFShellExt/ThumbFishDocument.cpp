// ThumbFishDocument.cpp : Implementation of the ThumbFishDocument class

#include "stdafx.h"
#include <propkey.h>
#include "ThumbFishDocument.h"

#define MAX_CACHE_RECORD_COUNT	4

// assuming a normal record would be a maximum of 10KB
#define MAX_CACHE_SIZE	(10 * 1024) * MAX_CACHE_RECORD_COUNT

// maximum size of mol supported is 100KB
#define MAX_MOL_SIZE	100 * 1024

HRESULT ThumbFishDocument::LoadFromStream(IStream* pStream, DWORD grfMode)
{
	STATSTG stat;

	pantheios::log_NOTICE(_T("ThumbFishDocument::LoadFromStream> Called"));

	// get stream properties
	HRESULT hr = pStream->Stat(&stat, STATFLAG_DEFAULT);
	if(hr == S_OK)
	{
		// initialize BUFFER
		m_Buffer.DataLength = stat.cbSize.LowPart;
		_tcscpy_s(m_Buffer.FileName, MAX_PATH, OLE2W(stat.pwcsName));

		pantheios::log_NOTICE(_T("ThumbFishDocument::LoadFromStream> Name="), m_Buffer.FileName, 
		_T(", Size="), pantheios::integer(m_Buffer.DataLength));

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

		pantheios::log_NOTICE(_T("ThumbFishDocument::LoadFromStream> Loading Stream..."));

		// load stream into BUFFER only when it contains data
		if((m_Buffer.DataLength > 0) && !LoadStream(pStream))
			pantheios::log_ERROR(_T("ThumbFishDocument::LoadFromStream> Could not load data."),
				m_Buffer.FileName, _T(", DataLength"), pantheios::integer(m_Buffer.DataLength));
	}

	pantheios::log_NOTICE(_T("ThumbFishDocument::LoadFromStream> HRESULT="), pantheios::integer(hr));

	return S_OK;
}

void ThumbFishDocument::InitializeSearchContent()
{
	pantheios::log_NOTICE(_T("ThumbFishDocument::InitializeSearchContent> Called"));

	CString value;
	if(pGetPropsFunc != NULL)
	{
		OPTIONS options;
		TCHAR** props = NULL;
		int propCount = pGetPropsFunc(&m_Buffer, &props, &options, true);
		bool m_propsGenerated = ((props != NULL) && (propCount > 0));

		if(m_propsGenerated)
		{
			pantheios::log_NOTICE(_T("ThumbFishDocument::InitializeSearchContent> Properties generated="), 
				pantheios::integer(propCount), _T(", for: "), m_Buffer.FileName);

			// insert property values into list view control
			for(int i = 0; i < propCount; i++)
			{
				TCHAR* propName = props[2*i];
				TCHAR* propValue = props[2*i + 1];

				// "-" property names will not be included in search
				if(_tcsicmp(propName, _T("-")) != 0)
				{
					value.AppendFormat(_T("%ls_%ls;"), propName, propValue);
				}
			}

			delete[] props;
		}
		else
		{
			pantheios::log_WARNING(_T("ThumbFishDocument::InitializeSearchContent> Property generation failed."));
		}
	}
	else
	{
		pantheios::log_ERROR(_T("ThumbFishDocument::InitializeSearchContent> Property function is not available."));
	}

	SetSearchContent(value);
}

void ThumbFishDocument::SetSearchContent(CString& value)
{
	pantheios::log_NOTICE(_T("ThumbFishDocument::SetSearchContent> Called"));

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
	pantheios::log_NOTICE(_T("ThumbFishDocument::OnDrawThumbnail> Called"));

	if(pDrawFunc != NULL)
	{
		OPTIONS options;
		options.IsThumbnail = true;
		if(!pDrawFunc(hDrawDC, *lprcBounds, &m_Buffer, &options))
		{
			pantheios::log_NOTICE(_T("ThumbFishDocument::OnDrawThumbnail> Draw returned FALSE. Thumbnail NOT drawn."),
				_T("File="), m_Buffer.FileName);
		}
	}
	else
	{
		pantheios::log_ERROR(_T("ThumbFishDocument::OnDrawThumbnail> Draw function not available."));
	}
}

BOOL ThumbFishDocument::LoadStream(IStream* stream)
{
	pantheios::log_NOTICE(_T("ThumbFishDocument::LoadStream> Called"));

	char recordDelimiter[20];
	memset(recordDelimiter, 0, 20);
	
	m_Buffer.pData = NULL;

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
		if(m_Buffer.DataLength < MAX_MOL_SIZE)
		{
			m_Buffer.pData = new char[m_Buffer.DataLength];

			HRESULT hr = stream->Read(m_Buffer.pData, m_Buffer.DataLength, (ULONG*)&m_Buffer.DataLength);

			if(hr == S_OK)
			{
				// set data version (V2000, V3000) info for MOL and RXN files
				if(m_Buffer.FileExtension == extMOL)
					m_Buffer.DataVersion = FindMolVersion(m_Buffer.pData, m_Buffer.DataLength, 4);
				else if(m_Buffer.FileExtension == extRXN)
					m_Buffer.DataVersion = FindMolVersion(m_Buffer.pData, m_Buffer.DataLength, 1);
			}

			return hr;
		}
		else
		{
			pantheios::log_WARNING(_T("ThumbFishDocument::LoadStream> File too large and cannot be read. Size= "),
				pantheios::integer(m_Buffer.DataLength));

			m_Buffer.DataLength = -1;	// file too large
			return false;
		}
	}

	/*	
		the following code runs for multimol files only and caches a number of records 
		because we cannot cache all the records.
	*/

	pantheios::log_NOTICE(_T("ThumbFishDocument::LoadStream> Reading multimol file..."));

	char readBuffer;
	ULONG readCount = 0;
	bool firstMolVersionChecked = false;
	char largeTempBuffer[MAX_CACHE_SIZE];
	size_t delimiterLen = strlen(recordDelimiter);
	int matchIndex = 0, totalReadBytes = 0, recordCount = 0, recordsReadBytes = 0;

	// read stream char by char, identify the record delimiters and cache 
	// MAX_CACHE_RECORDS number of records
	while(stream->Read(&readBuffer, 1, &readCount) == S_OK)
	{
		largeTempBuffer[totalReadBytes++] = readBuffer;
		if(totalReadBytes >= MAX_CACHE_SIZE) break;	// very big records, normally not possible but just in case

		// identify record delimiter in text
		if(readBuffer == recordDelimiter[matchIndex++])
		{
			if(matchIndex == delimiterLen) // whole delimiter string matches
			{
				if(!firstMolVersionChecked)
				{
					if(m_Buffer.FileExtension == extSDF)
					{
						m_Buffer.DataVersion = FindMolVersion(largeTempBuffer, (totalReadBytes - recordsReadBytes), 4);
						firstMolVersionChecked = true;
					}
					else if((m_Buffer.FileExtension == extRDF) && (recordCount == 1))	// RDF has start tag unlike SDF ending tag $$$$
					{
						m_Buffer.DataVersion = FindMolVersion(largeTempBuffer, (totalReadBytes - recordsReadBytes), 4);
						firstMolVersionChecked = true;
					}
				}

				recordsReadBytes = totalReadBytes;	// count of bytes upto which we got records
				recordCount++;	// RDF will have one less than MAX_CACHE_RECORD_COUNT coz it has starting tag
			}

			if(recordCount == MAX_CACHE_RECORD_COUNT) break;	// desired number of records read
		}
		else
		{
			matchIndex = 0;
		}
	}

	if(recordCount > 0)
	{
		// approximate the total number of records
		if((m_Buffer.FileExtension == extSDF) || (m_Buffer.FileExtension == extCML))
			m_Buffer.TotalRecords = (m_Buffer.DataLength / (recordsReadBytes / 4));
		else if(m_Buffer.FileExtension == extRDF)
			m_Buffer.TotalRecords = (m_Buffer.DataLength / (recordsReadBytes / 3));

		m_Buffer.pData = new char[recordsReadBytes];
		m_Buffer.DataLength = recordsReadBytes;
		memcpy(m_Buffer.pData, largeTempBuffer, recordsReadBytes);
		return TRUE;
	}
	else
	{
		m_Buffer.DataLength = 0;	// no records read
	}

	pantheios::log_NOTICE(_T("ThumbFishDocument::LoadStream> Records Read="), pantheios::integer(recordCount), 
		_T(", Bytes Read="), pantheios::integer(recordsReadBytes),
		_T(", Approx Records="), pantheios::integer(m_Buffer.TotalRecords));

	return FALSE;
}

int ThumbFishDocument::FindMolVersion(char* data, int dataLength, int lineNum)
{
	int line = 0;
	for(int index = 0; index < dataLength; index++)
	{
		if(data[index] == '\n')
		{
			line++;

			// when we are at the end of desired line, check if the last few 
			// chars corresponds to some version number
			if(line == lineNum)
			{
				// if the LF char is preceded by a CR then we have to shift our compare index
				int shiftIndex = (data[index - 1] == '\r') ? 6 : 5;

				// compare the last 5/6 characters to get the version number
				// For V3000 format, the text 'V3000' will always be there. This is not true
				// for V2000 which could be missing in a RXN file
				if (_strnicmp(&(data[index - shiftIndex]), "V3000", 5) == 0) return 2;
				else return 1;	// default to V2000
			}
		}
	}
}

/// <summary>
/// This overload is needed to fix ATL's default monochrome bitmap
/// More Info: http://www.patthoyts.tk/blog/fixing-the-atl-ithumbnailprovider-implementation.html
/// </summary>
BOOL ThumbFishDocument::GetThumbnail(_In_ UINT cx, _Out_ HBITMAP* phbmp, _In_opt_ WTS_ALPHATYPE* /* pdwAlpha */)
{
	pantheios::log_NOTICE(_T("ThumbFishDocument::GetThumbnail> Called"));

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
