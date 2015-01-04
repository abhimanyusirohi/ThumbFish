// stdafx.cpp : source file that includes just the standard includes
// ThumbFish.pch will be the pre-compiled header
// stdafx.obj will contain the pre-compiled type information

#include "stdafx.h"
#include <pantheios/frontend.h>

HINSTANCE dllHandle;
HINSTANCE ThisInstance;

ExecuteFuncType pExecuteFunc;

// Custom Pantheios FrontEnd implementation to filter messages
PANTHEIOS_CALL(int) pantheios_fe_init(void* /* reserved */, void** ptoken)
{
	*ptoken = NULL;
	return 0;
}

PANTHEIOS_CALL(void) pantheios_fe_uninit(void* /* token */) {}
PANTHEIOS_CALL(PAN_CHAR_T const*) pantheios_fe_getProcessIdentity(void* /* token */) { return _T("ThumbFish"); }

PANTHEIOS_CALL(int) pantheios_fe_isSeverityLogged(void* /* token */, int severity, int /* backEndId */)
{
	// define DETAILEDLOG in final release so that only WARNING and above messages are logged
	// for other releases such as beta, all informational messages should be logged
	#if DETAILEDLOG
		return 1;	// log all messages
	#else
		return severity <= pantheios::warning;	// log warnings and above only
	#endif
}
