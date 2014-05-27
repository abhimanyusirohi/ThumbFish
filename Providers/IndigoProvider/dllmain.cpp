// dllmain.cpp : Defines the entry point for the DLL application.
#include "stdafx.h"

HINSTANCE hInstance = NULL;

void InitPantheios()
{
	pantheios::pantheios_init();

	TCHAR logFile[MAX_PATH];
	if(Utils::GetSystemFolder(FOLDERID_LocalAppDataLow, logFile) == S_OK)
		_tcscat_s(logFile, MAX_PATH, _T("\\"));

	// in case of failure to get a log file path in AppDataLow, log file 
	// will be created in the current directory
	_tcscat_s(logFile, MAX_PATH, _T("ThumbFish.log"));
	pantheios_be_file_setFilePath(logFile, PANTHEIOS_BE_FILE_F_SHARE_ON_WINDOWS, 
		PANTHEIOS_BE_FILE_F_SHARE_ON_WINDOWS, PANTHEIOS_BEID_ALL);
}

BOOL APIENTRY DllMain( HMODULE hModule,
                       DWORD  ul_reason_for_call,
                       LPVOID lpReserved
					 )
{
	switch (ul_reason_for_call)
	{
	case DLL_PROCESS_ATTACH:
		hInstance = hModule;
		InitPantheios();
		break;

	case DLL_THREAD_ATTACH:
		break;

	case DLL_THREAD_DETACH:
		break;

	case DLL_PROCESS_DETACH:
		hInstance = NULL;
		pantheios::pantheios_uninit();
		break;
	}
	return TRUE;
}

