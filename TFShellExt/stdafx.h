// stdafx.h : include file for standard system include files,
// or project specific include files that are used frequently,
// but are changed infrequently

#pragma once

#ifndef STRICT
#define STRICT
#endif

#include "targetver.h"

#ifdef _MANAGED
#error File type handlers cannot be built as managed assemblies.  Set the Common Language Runtime options to no CLR support in project properties.
#endif

#ifndef _UNICODE
#error File type handlers must be built Unicode.  Set the Character Set option to Unicode in project properties.
#endif

#define SHARED_HANDLERS

#define ATL_NO_ASSERT_ON_DESTROY_NONEXISTENT_WINDOW

#define SOURCE _T("ThumbFish")
#define MODULE _T("ThumbFish.dll")

#include <atlbase.h>
#include <atlcom.h>
#include <atlctl.h>
#include <atlconv.h>
#include "comdef.h"

//#include "..\Logger\Logger.h"
#include <pantheios\pantheios.hpp>
#include <pantheios\inserters\integer.hpp>
#include <pantheios\backends\bec.file.h>

// implicit linking
#include <pantheios\implicit_link\util.h>
#include <pantheios\implicit_link\core.h>
#include <pantheios\implicit_link\be.file.h>
#include <pantheios\implicit_link\fe.Simple.h>

#include <string>
#include <map>
#include "Types.h"
#include <ShlObj.h>
#include "Utils.h"

extern HINSTANCE dllHandle;
extern HINSTANCE ThisInstance;

extern DrawFuncType pDrawFunc;
extern GetPropertiesFuncType pGetPropsFunc;
extern ConvertToFuncType pConvertFunc;
