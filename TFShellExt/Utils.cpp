#include "StdAfx.h"
#include "Utils.h"

Utils::Utils(void)
{
}

Utils::~Utils(void)
{
}

// This code is taken from an MSDN Sample
// http://msdn.microsoft.com/en-us/library/windows/desktop/bb776913(v=vs.85).aspx
// Opens up Save File Dialog. The PrevHost (Preview Handler executable) runs in Low integrity and can 
// only write inside 'C:\Users\{USER}\AppData\LocalLow' folder (FOLDERID_LocalAppDataLow).
HRESULT Utils::DoFileSaveDialog(HWND owner, PWSTR filePath)
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
                        hr = pfd->SetDefaultExtension(L"png");
                        if (SUCCEEDED(hr))
                        {
                            // Show the dialog
                            hr = pfd->Show(owner);
                            if (SUCCEEDED(hr))
                            {
                                // Obtain the result once the user clicks 
                                // the 'Save' button.
                                // The result is an IShellItem object.
                                IShellItem *psiResult;
                                hr = pfd->GetResult(&psiResult);
                                if (SUCCEEDED(hr))
                                {
                                    // We are just going to print out the 
                                    // name of the file for sample sake.
                                    PWSTR pszFilePath = NULL;
                                    hr = psiResult->GetDisplayName(SIGDN_FILESYSPATH, 
                                                        &pszFilePath);
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

	return ShellExecuteEx(&shInfo);
}

void Utils::DoSaveStructure(HWND parentWnd, LPBUFFER buffer, LPOPTIONS options)
{
	WCHAR filePath[MAX_PATH];

	// TODO: for some reason, setting owner to our preview control handle suppresses the dialog
	// passing NULL as owner handle has the side effect of showing the Save dialog in taskbar
	if(Utils::DoFileSaveDialog(NULL, filePath) == S_OK)
	{
		LPWSTR ext = ::PathFindExtensionW(filePath);
		sprintf_s(options->RenderOutputExtension, EXTLEN, "%ws", (ext[0] == '.') ? ext + 1 : ext);

		// convert existing structure into bytes in specified format
		char* data = pConvertFunc(buffer, options);

		if(data != NULL)
		{
			HANDLE hFile; 
			DWORD dwBytesToWrite = (DWORD)options->OutBufferSize;
			DWORD dwBytesWritten = 0;
			BOOL bErrorFlag = FALSE;

			// create the file and then write data to it
			hFile = CreateFile(filePath, GENERIC_WRITE, 0, NULL, CREATE_NEW, FILE_ATTRIBUTE_NORMAL,	NULL);
			if (hFile != INVALID_HANDLE_VALUE)
			{
				bErrorFlag = WriteFile(hFile, data, dwBytesToWrite, &dwBytesWritten, NULL);
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
			pantheios::log_ERROR(_T("Utils::DoSaveStructure> No data returned from Convert method. OutBufferSize= "), 
				pantheios::integer(options->OutBufferSize));
		}
	}
}

bool Utils::CopyToClipboard(const char* data, int dataLength, int format)
{
	HANDLE handle = NULL;
	if(format == CF_ENHMETAFILE)
	{
		handle = SetEnhMetaFileBits(dataLength, (BYTE*)data);
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

char* Utils::ConvertStructure(LPBUFFER buffer, ConvertFormats convertTo, LPOPTIONS options)
{
	switch (convertTo)
	{
		case ConvertFormats::SMILES:		// CopyAs SMILES
			strcpy_s(options->RenderOutputExtension, EXTLEN, "smi");
			break;

		case ConvertFormats::InChi:			// CopyAs InChi
			strcpy_s(options->RenderOutputExtension, EXTLEN, "inchi");
			break;

		case ConvertFormats::InChiKey:			// CopyAs InChiKey
			strcpy_s(options->RenderOutputExtension, EXTLEN, "inchik");
			break;

		case ConvertFormats::MolV2000:			// CopyAs MOLV2000
		case ConvertFormats::MolV3000:			// CopyAs MOLV3000
			options->MOLSavingMode = (convertTo == ConvertFormats::MolV2000) ? 1 : 2;
			strcpy_s(options->RenderOutputExtension, EXTLEN, "mol");
			break;

		case ConvertFormats::CDXML:				// CopyAs CDXML
			strcpy_s(options->RenderOutputExtension, EXTLEN, "cdxml");
			break;

		case ConvertFormats::EMF:				// CopyAs EMF
			strcpy_s(options->RenderOutputExtension, EXTLEN, "emf");
			break;

		default:
			return NULL;
	}
	
	// convert existing structure into bytes in specified format
	// default width and height values are used for now
	return pConvertFunc(buffer, options);
}

Extension Utils::GetExtension(TCHAR* fileName)
{
	// set extension enum
	TCHAR* ext = ::PathFindExtension(fileName);
	
	if(TEQUAL(ext, ".mol")) return extMOL;
	else if(TEQUAL(ext, ".mol")) return extMOL;
	else if(TEQUAL(ext, ".rxn")) return extRXN;
	else if(TEQUAL(ext, ".smi")) return extSMI;
	else if(TEQUAL(ext, ".smiles")) return extSMILES;
	else if(TEQUAL(ext, ".smarts")) return extSMARTS;
	else if(TEQUAL(ext, ".sdf")) return extSDF;
	else if(TEQUAL(ext, ".rdf")) return extRDF;
	else if(TEQUAL(ext, ".cml")) return extCML;
	else return extUnknown;
}

bool Utils::IsMultiMolFile(TCHAR* fileName)
{
	Extension ext = Utils::GetExtension(fileName);
	return ((ext == extSDF) || (ext == extRDF) || (ext == extCML));
}
