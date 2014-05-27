#include "StdAfx.h"
#include "Utils.h"


Utils::Utils(void)
{
}


Utils::~Utils(void)
{
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
