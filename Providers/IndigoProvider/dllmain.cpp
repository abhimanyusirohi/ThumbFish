// dllmain.cpp : Defines the entry point for the DLL application.
#include "stdafx.h"

//Logger* _logger = NULL;

BOOL APIENTRY DllMain( HMODULE hModule,
                       DWORD  ul_reason_for_call,
                       LPVOID lpReserved
					 )
{
	switch (ul_reason_for_call)
	{
	case DLL_PROCESS_ATTACH:
		//if(_logger == NULL) _logger = new Logger(_T("IndigoProvider"), _T("IndigoProvider"));
		break;

	case DLL_THREAD_ATTACH:
		break;

	case DLL_THREAD_DETACH:
		break;

	case DLL_PROCESS_DETACH:
		//if(_logger != NULL) delete _logger;
		break;
	}
	return TRUE;
}

