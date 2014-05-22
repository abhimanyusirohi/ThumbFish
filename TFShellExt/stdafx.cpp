// stdafx.cpp : source file that includes just the standard includes
// ThumbFish.pch will be the pre-compiled header
// stdafx.obj will contain the pre-compiled type information

#include "stdafx.h"

HINSTANCE dllHandle;
HINSTANCE ThisInstance;

DrawFuncType pDrawFunc;
GetPropertiesFuncType pGetPropsFunc;
ConvertToFuncType pConvertFunc;

PANTHEIOS_EXTERN_C const PAN_CHAR_T PANTHEIOS_FE_PROCESS_IDENTITY[] = PANTHEIOS_LITERAL_STRING("ThumbFish");

//// define NOLOG in final release so that only WARNING and above messages are logged
//// for other releases such as beta, all informational messages will be logged
//#if NOLOG
//PANTHEIOS_CALL(int) pantheios_fe_isSeverityLogged(void* token, int severity, int backEndId)
//{
//	return severity <= pantheios::warning;
//}
//#endif
