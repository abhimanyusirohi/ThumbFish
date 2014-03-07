// dllmain.cpp : Implementation of DllMain.

#include "stdafx.h"
#include "resource.h"
#include "ThumbFish_i.h"
#include "dllmain.h"
#include "xdlldata.h"

CThumbFishModule _AtlModule;

void InitInstance()
{
	TCHAR corePath[MAX_PATH];
	BOOL runTimeLinkSuccess = FALSE; 

	// make file path to provider dll in this module's directory
	GetModuleFileName(_AtlBaseModule.m_hInst, corePath, MAX_PATH);
	PathRemoveFileSpec(corePath);
	PathAppend(corePath, L"provider.dll");

	pantheios::log_INFORMATIONAL(_T("InitInstance: Loading Provider dll: "), corePath);

	// load the dll and keep the handle to it
	dllHandle = LoadLibrary(corePath);
	
	// If the handle is valid, try to get the function addresses
	if (dllHandle != NULL) 
	{
		// Get pointer to our functions using GetProcAddress:
		pDrawFunc = (DrawFuncType)GetProcAddress(dllHandle, "Draw");
		pGetPropsFunc = (GetPropertiesFuncType)GetProcAddress(dllHandle, "GetProperties");

		if(pDrawFunc == NULL)
			pantheios::log_WARNING(_T("InitInstance: GetProcAddress FAILED for Draw. GetLastError=")
									, pantheios::integer(GetLastError()));

		if(pGetPropsFunc == NULL)
			pantheios::log_WARNING(_T("InitInstance: GetProcAddress FAILED for GetProperties. GetLastError=")
									, pantheios::integer(GetLastError()));
	}
	else
	{
		pantheios::log_ERROR(_T("InitInstance: LoadLibrary FAILED"));
	}
}

void ExitInstance()
{
	// Free the library
	if(dllHandle != NULL)
		FreeLibrary(dllHandle);

	dllHandle = NULL;
}

// DLL Entry Point
extern "C" BOOL WINAPI DllMain(HINSTANCE hInstance, DWORD dwReason, LPVOID lpReserved)
{
	switch( dwReason ) 
	{
		case DLL_PROCESS_ATTACH:
			ThisInstance = hInstance;
			
			pantheios::pantheios_init();
			pantheios_be_file_setFilePath(_T("D:\\ThumbFish-Log.log"));

			pantheios::log_INFORMATIONAL(_T("DllMain -> DLL_PROCESS_ATTACH"));
			InitInstance();
			break;

		case DLL_THREAD_ATTACH:
			pantheios::log_INFORMATIONAL(_T("DllMain -> DLL_THREAD_ATTACH"));
			break;

		case DLL_THREAD_DETACH:
			pantheios::log_INFORMATIONAL(_T("DllMain -> DLL_THREAD_DETACH"));
			break;

		case DLL_PROCESS_DETACH:
			pantheios::log_INFORMATIONAL(_T("DllMain -> DLL_PROCESS_DETACH"));
			ExitInstance();
			pantheios::pantheios_uninit();
			break;
    }

#ifdef _MERGE_PROXYSTUB
	if (!PrxDllMain(hInstance, dwReason, lpReserved))
		return FALSE;
#endif
	hInstance;
	return _AtlModule.DllMain(dwReason, lpReserved); 
}
