// stdafx.cpp : source file that includes just the standard includes
// ThumbFish.pch will be the pre-compiled header
// stdafx.obj will contain the pre-compiled type information

#include "stdafx.h"

HINSTANCE dllHandle;
HINSTANCE ThisInstance;

DrawFuncType pDrawFunc;
GetPropertiesFuncType pGetPropsFunc;

PANTHEIOS_EXTERN_C const PAN_CHAR_T PANTHEIOS_FE_PROCESS_IDENTITY[] = PANTHEIOS_LITERAL_STRING("ThumbFish");

