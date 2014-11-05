#include "StdAfx.h"
#include "Utils.h"

Utils::Utils(void)
{
}

Utils::~Utils(void)
{
}

HRESULT Utils::DoFolderDialog(HWND owner, LPCTSTR title, LPTSTR startFolder, LPTSTR folderPath)
{
    BROWSEINFO bi = { 0 };
    bi.lpszTitle  = title;
    bi.ulFlags    = BIF_RETURNONLYFSDIRS | BIF_NEWDIALOGSTYLE;
    bi.lpfn       = BrowseFolderCallback;
    bi.lParam     = (LPARAM) startFolder;
	bi.hwndOwner  = owner;
	
    LPITEMIDLIST pidl = SHBrowseForFolder ( &bi );

    if ( pidl != 0 )
    {
        // get the name of the folder and put it in path
        SHGetPathFromIDList (pidl, folderPath);

        // free memory used
        IMalloc * imalloc = 0;
        if (SUCCEEDED(SHGetMalloc(&imalloc)))
        {
            imalloc->Free(pidl);
            imalloc->Release();
        }

		return S_OK;
	}
	else
	{
		return S_FALSE;
	}
}

// This code is taken from an MSDN Sample
// http://msdn.microsoft.com/en-us/library/windows/desktop/bb776913(v=vs.85).aspx
// Opens up Save File Dialog. The PrevHost (Preview Handler executable) runs in Low integrity and can 
// only write inside 'C:\Users\{USER}\AppData\LocalLow' folder (FOLDERID_LocalAppDataLow).
HRESULT Utils::DoFileSaveDialog(HWND owner, PWSTR filePath, UINT* filterIndex)
{
    // CoCreate the File Open Dialog object.
    IFileDialog *pfd = NULL;
    HRESULT hr = CoCreateInstance(CLSID_FileSaveDialog, 
                      NULL, 
                      CLSCTX_INPROC_SERVER, 
                      IID_PPV_ARGS(&pfd));
    if (SUCCEEDED(hr))
    {
        // Set the options on the dialog.
        DWORD dwFlags;

        // Before setting, always get the options first in order 
        // not to override existing options.
        hr = pfd->GetOptions(&dwFlags);
        if (SUCCEEDED(hr))
        {
            // In this case, get shell items only for file system items.
            hr = pfd->SetOptions(dwFlags | FOS_OVERWRITEPROMPT | FOS_FORCEFILESYSTEM | FOS_STRICTFILETYPES);
            if (SUCCEEDED(hr))
            {
				CComPtr<IShellItem> psiFolder;
				LPWSTR wszPath = NULL;
 
				// set the initial folder to the AppData\LocalLow because that's where we have write permissions
				hr = SHGetKnownFolderPath (FOLDERID_LocalAppDataLow, KF_FLAG_CREATE, NULL, &wszPath ); 
				if (SUCCEEDED(hr))
				{
					hr = SHCreateItemFromParsingName (wszPath, NULL, IID_PPV_ARGS(&psiFolder));
					if (SUCCEEDED(hr))
						pfd->SetFolder (psiFolder);
					CoTaskMemFree ( wszPath );
				}

                // Set the file types to display only. 
                // Notice that this is a 1-based array.
                hr = pfd->SetFileTypes(ARRAYSIZE(c_SaveTypes), c_SaveTypes);
                if (SUCCEEDED(hr))
                {
                    // Set the selected file type index to first in the list.
                    hr = pfd->SetFileTypeIndex(1);
                    if (SUCCEEDED(hr))
                    {
                        // Set the default extension to be ".png" file.
						//TODO: Will go in options
                        hr = pfd->SetDefaultExtension(L"png");
                        if (SUCCEEDED(hr))
                        {
                            // Show the dialog
                            hr = pfd->Show(owner);
                            if (SUCCEEDED(hr))
                            {
								// ONE based index
								pfd->GetFileTypeIndex(filterIndex);

                                // Obtain the result once the user clicks 
                                // the 'Save' button.
                                // The result is an IShellItem object.
                                IShellItem *psiResult;
                                hr = pfd->GetResult(&psiResult);
                                if (SUCCEEDED(hr))
                                {
                                    PWSTR pszFilePath = NULL;
                                    hr = psiResult->GetDisplayName(SIGDN_FILESYSPATH, &pszFilePath);
                                    if (SUCCEEDED(hr))
                                    {
										_tcscpy_s(filePath, MAX_PATH, pszFilePath);
                                        CoTaskMemFree(pszFilePath);
                                    }
                                    psiResult->Release();
                                }
                            }
                        }
                    }
                }
            }
        }
        pfd->Release();
    }

	pantheios::log_NOTICE(_T("Utils::DoFileSaveDialog> HRESULT="), pantheios::integer(hr));

    return hr;
}

// Sets the outPath variable with path to the specified system folder
// outPath must be an array of MAX_PATH size
HRESULT Utils::GetSystemFolder(const KNOWNFOLDERID folderID, TCHAR* outPath)
{
	CComPtr<IShellItem> psiFolder;
	LPWSTR wszPath = NULL;
 
	HRESULT hr = SHGetKnownFolderPath (folderID, KF_FLAG_CREATE, NULL, &wszPath); 
	if (SUCCEEDED(hr))
	{
		hr = SHCreateItemFromParsingName(wszPath, NULL, IID_PPV_ARGS(&psiFolder));
		if (SUCCEEDED(hr))
		{
			_tcscpy_s(outPath, MAX_PATH, wszPath);
		}
		CoTaskMemFree (wszPath);
	}

	pantheios::log_NOTICE(_T("Utils::GetSystemFolder> HRESULT="), pantheios::integer(hr));

	return hr;
}

// ShellExecuteLink
// Opens the specified link in default browser using the ShellExecuteEx API
bool Utils::ShellExecuteLink(const TCHAR* link)
{
	pantheios::log_NOTICE(_T("Utils::ShellExecuteLink> Called. LINK="), link);

	SHELLEXECUTEINFO shInfo;
	shInfo.cbSize = sizeof(SHELLEXECUTEINFO);
    shInfo.fMask = NULL;
    shInfo.hwnd = NULL;
    shInfo.lpVerb = NULL;
    shInfo.lpFile = link;
    shInfo.lpParameters = NULL;
    shInfo.lpDirectory = NULL;
    shInfo.nShow = SW_SHOW;
    shInfo.hInstApp = NULL;

	return (ShellExecuteEx(&shInfo) == TRUE) ? true : false;
}

void Utils::DoSaveStructure(HWND parentWnd, LPBUFFER buffer, LPOPTIONS options)
{
	UINT filterIndex;
	WCHAR filePath[MAX_PATH];
	LPOUTBUFFER oBuffer = NULL;

	// TODO: for some reason, setting owner to our preview control handle suppresses the dialog
	// passing NULL as owner handle has the side effect of showing the Save dialog in taskbar
	if(Utils::DoFileSaveDialog(NULL, filePath, &filterIndex) == S_OK)
	{
		ChemFormat format = CommonUtils::GetFormatFromFileName(filePath);

		// both mol/rxn v2/v3 have the same extension so we need to identify V3 filters
		if(filterIndex == ONEBASED_MOLV3FILTERINDEX)
			format = fmtMOLV3;
		else if(filterIndex == ONEBASED_RXNV3FILTERINDEX)
			format = fmtRXNV3;

		// convert existing structure into bytes in specified format
		oBuffer = pConvertFunc(buffer, format, options);

		if(oBuffer != NULL)
		{
			HANDLE hFile; 
			DWORD dwBytesWritten = 0;
			BOOL bErrorFlag = FALSE;
			PCHAR buffer = oBuffer->pData;
			DWORD dwBytesToWrite = (DWORD)oBuffer->DataLength;

			// write a more human readable form of MDLCT containing line breaks
			if(format == fmtMDLCT) 
			{
				buffer = Utils::PrettyMDLCT(buffer, dwBytesToWrite);
				dwBytesToWrite = strlen(buffer) + 1;
			}

			// create the file and then write data to it
			hFile = CreateFile(filePath, GENERIC_WRITE, 0, NULL, CREATE_NEW, FILE_ATTRIBUTE_NORMAL,	NULL);
			if (hFile != INVALID_HANDLE_VALUE)
			{
				bErrorFlag = WriteFile(hFile, buffer, dwBytesToWrite, &dwBytesWritten, NULL);
				if ((FALSE == bErrorFlag) || (dwBytesWritten != dwBytesToWrite))
				{
					::MessageBox(parentWnd, _T("Unable to write data to the file"), _T("File Write Error"), MB_OK | MB_ICONERROR);
				}
			}
			else
			{
				::MessageBox(parentWnd, _T("Unable to create file"), _T("File Write Error"), MB_OK | MB_ICONERROR);
			}

			CloseHandle(hFile);
		}
		else
		{
			pantheios::log_ERROR(_T("Utils::DoSaveStructure> No data returned from Convert method. \
				This could be due to a failure or the conversion is not currently supported."));

			// display message to the user that the operation failed
			::MessageBox(NULL, _T("The selected file could not be saved in requested format."), 
				_T("Save Failed"), MB_OK | MB_ICONERROR);
		}
	}

	if(oBuffer) delete oBuffer;
}

bool Utils::CopyToClipboard(const char* data, size_t dataLength, int format)
{
	HANDLE handle = NULL;
	if(format == CF_ENHMETAFILE)
	{
		handle = SetEnhMetaFileBits((UINT)dataLength, (BYTE*)data);
	}
	else
	{
		// Allocate string and copy to global memory
		handle = GlobalAlloc(GMEM_MOVEABLE | GMEM_DDESHARE, dataLength);
		LPVOID dst = GlobalLock(handle);
		memcpy(dst, data, dataLength);
		GlobalUnlock(handle);
	}

	if(handle != NULL)
	{
		if (::OpenClipboard(NULL))
		{
			EmptyClipboard();
			if (SetClipboardData(format, handle))
			{
				CloseClipboard();
			}
			else
			{
				pantheios::log_ERROR(_T("Utils::CopyToClipboard> SetClipboardData FAILED. GetLastError="),
							pantheios::integer(GetLastError()));
				return false;
			}
		}
		else
		{
			pantheios::log_ERROR(_T("Utils::CopyToClipboard> OpenClipboard FAILED. GetLastError="),
						pantheios::integer(GetLastError()));
			return false;
		}
	}
	else
	{
		pantheios::log_ERROR(_T("Utils::CopyToClipboard> Handle is NULL. GetLastError="), 
			pantheios::integer(GetLastError()));
		return false;
	}

	if((format == CF_ENHMETAFILE) && (handle != NULL))
		DeleteEnhMetaFile((HENHMETAFILE)handle);

	return true;
}

void Utils::ConvertAndCopy(LPBUFFER buffer, ChemFormat convertTo, LPOPTIONS options)
{
	LPOUTBUFFER oBuffer = pConvertFunc(buffer, convertTo, options);
	if(oBuffer != NULL)
	{
		Utils::CopyToClipboard(oBuffer->pData, oBuffer->DataLength, 
			(convertTo == fmtEMF) ? CF_ENHMETAFILE : CF_TEXT);
		delete oBuffer;
	}
	else
	{
		pantheios::log_ERROR(_T("Utils::ConvertAndCopy> Convert method FAILED. OutBuffer is NULL. Conversion From="),
			pantheios::integer(buffer->DataFormat), _T(", To="), pantheios::integer(convertTo));

		// display message to the user that the operation failed
		::MessageBox(NULL, _T("The selected file could not be copied in requested format."), 
			_T("Copy Failed"), MB_OK | MB_ICONERROR);
	}
}

bool Utils::IsMultiMolFile(TCHAR* fileName)
{
	return CommonUtils::IsMultiMolFormat(CommonUtils::GetFormatFromFileName(fileName));
}

HMENU Utils::CreateCopyMenu(ChemFormat srcFormat, UINT* idStart)
{
	HMENU hCopyAsMenu = CreatePopupMenu();

	bool isMol = (srcFormat == fmtMOLV2) || (srcFormat == fmtMOLV3);
	bool isRxn = (srcFormat == fmtRXNV2) || (srcFormat == fmtRXNV3);
	bool isMolRxn = (srcFormat == fmtCML) || (srcFormat == fmtSMILES) || (srcFormat == fmtSMARTS);
	bool isAllExceptSDFRDF = isMol || isRxn ||isMolRxn;

	// reaction cannot be saved as CDXML due to an issue with Indigo
	// https://github.com/ggasoftware/indigo/issues/9
	InsertMenu(hCopyAsMenu, 0, MF_BYPOSITION | ((isAllExceptSDFRDF && !isRxn) ? MF_ENABLED : MF_DISABLED), (*idStart)++, _T("CDXML"));

	// reaction cannot be saved as CML (https://github.com/ggasoftware/indigo/issues/10)
	InsertMenu(hCopyAsMenu, 1, MF_BYPOSITION | (isAllExceptSDFRDF && !isRxn ? MF_ENABLED : MF_DISABLED), (*idStart)++, _T("CML"));
	InsertMenu(hCopyAsMenu, 2, MF_BYPOSITION | (isAllExceptSDFRDF ? MF_ENABLED : MF_DISABLED), (*idStart)++, _T("EMF"));
	InsertMenu(hCopyAsMenu, 3, MF_BYPOSITION | ((isMol || isMolRxn) ? MF_ENABLED : MF_DISABLED), (*idStart)++, _T("INCHI"));

	// cannot convert to INCHIKEY due to an issue with provider
	// https://github.com/ggasoftware/indigo/issues/11
	InsertMenu(hCopyAsMenu, 4, MF_BYPOSITION | MF_DISABLED, (*idStart)++, _T("INCHI KEY"));
	InsertMenu(hCopyAsMenu, 6, MF_BYPOSITION | ((isMol || isMolRxn) ? MF_ENABLED : MF_DISABLED), (*idStart)++, _T("MOL V2000"));
	InsertMenu(hCopyAsMenu, 7, MF_BYPOSITION | ((isMol || isMolRxn) ? MF_ENABLED : MF_DISABLED), (*idStart)++, _T("MOL V3000"));
	InsertMenu(hCopyAsMenu, 8, MF_BYPOSITION | ((isRxn || isMolRxn) ? MF_ENABLED : MF_DISABLED), (*idStart)++, _T("RXN V2000"));
	InsertMenu(hCopyAsMenu, 9, MF_BYPOSITION | ((isRxn || isMolRxn) ? MF_ENABLED : MF_DISABLED), (*idStart)++, _T("RXN V3000"));
    InsertMenu(hCopyAsMenu, 10, MF_BYPOSITION | (isAllExceptSDFRDF ? MF_ENABLED : MF_DISABLED), (*idStart)++, _T("SMILES"));

	return hCopyAsMenu;
}

int CALLBACK Utils::BrowseFolderCallback(HWND hwnd, UINT uMsg, LPARAM lParam, LPARAM lpData)
{
    if (uMsg == BFFM_INITIALIZED) {
        LPCTSTR path = reinterpret_cast<LPCTSTR>(lpData);
        ::SendMessage(hwnd, BFFM_SETSELECTION, true, (LPARAM) path);
    }
    return 0;
}

/**
	PrettyMDLCT
	Converts the real MDLCT format to a more human-readable one by adding CRLF
	for line breaks and removing the DATALEN+DATA format that could include NULLs

	Returns a null terminated string
*/
YOURS PCHAR	Utils::PrettyMDLCT(PCHAR mdlct, int dataLen)
{
	assert(mdlct != NULL);
	assert(dataLen != 0);

	// allocate memory for new data
	int outlen = dataLen + 1 + 50;
	PCHAR out = new char[outlen];
	memset(out, 0, outlen);

	int srcIndex = 1, destIndex = 0;
	for(int i = 0; i < dataLen; i++)
	{
		int len = (int)mdlct[i];
		memcpy_s(out + destIndex, dataLen, mdlct + srcIndex, len);
		out[destIndex + len] = CR;
		out[destIndex + len + 1] = LF;
		i += len;

		destIndex += len + 2;	// extra 1 for LF
		srcIndex += len + 1;
	}

	return out;
}
