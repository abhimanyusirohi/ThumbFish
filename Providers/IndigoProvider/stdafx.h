// stdafx.h : include file for standard system include files,
// or project specific include files that are used frequently, but
// are changed infrequently
//

#pragma once

#include "targetver.h"

#define WIN32_LEAN_AND_MEAN             // Exclude rarely-used stuff from Windows headers
// Windows Header Files:
#include <windows.h>

#include <string>
#include <map>
#include <tchar.h>

#pragma comment(lib, "Shlwapi.lib")

#include "..\..\TFShellExt\Types.h"
#include <ObjIdl.h>
#include <Shlwapi.h>
#include "resource.h"

extern HINSTANCE hInstance;
