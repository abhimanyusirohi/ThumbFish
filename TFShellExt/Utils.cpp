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
BOOL Utils::ShellExecuteLink(const TCHAR* link)
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
