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
#include <atlbase.h>

#include <pantheios\pantheios.hpp>
#include <pantheios\inserters\integer.hpp>
#include <pantheios\inserters\pointer.hpp>
#include <pantheios\backends\bec.file.h>

// implicit linking
#include <pantheios\implicit_link\util.h>
#include <pantheios\implicit_link\core.h>
#include <pantheios\implicit_link\be.file.h>

#pragma comment(lib, "Shlwapi.lib")

#include "..\..\TFShellExt\Types.h"
#include <ObjIdl.h>
#include <Shlwapi.h>
#include "resource.h"

// ShellChangeNotify
#include <MsiQuery.h>
#include <ShlObj.h>
#include <ShellAPI.h>
#include "Utils.h"

extern HINSTANCE hInstance;
