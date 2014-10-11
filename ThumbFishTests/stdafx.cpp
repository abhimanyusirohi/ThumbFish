// stdafx.cpp : source file that includes just the standard includes
// ThumbFishTests.pch will be the pre-compiled header
// stdafx.obj will contain the pre-compiled type information

#include "stdafx.h"
#include <pantheios/frontend.h>

DrawFuncType pDrawFunc;
GetPropertiesFuncType pGetPropsFunc;
ConvertToFuncType pConvertFunc;
ExtractFuncType pExtractFunc;

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
	return 1;	// log all messages
}
