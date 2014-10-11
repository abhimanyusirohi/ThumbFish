// stdafx.h : include file for standard system include files,
// or project specific include files that are used frequently, but
// are changed infrequently
//

#pragma once

#include "targetver.h"

#include <stdio.h>
#include <tchar.h>


#define _ATL_CSTRING_EXPLICIT_CONSTRUCTORS      // some CString constructors will be explicit

#include <atlbase.h>
#include <atlstr.h>
#include <atlctl.h>
#include <atlconv.h>
#include "comdef.h"

#include <pantheios\pantheios.hpp>
#include <pantheios\inserters\integer.hpp>
#include <pantheios\backends\bec.file.h>

// implicit linking
#include <pantheios\implicit_link\util.h>
#include <pantheios\implicit_link\core.h>
#include <pantheios\implicit_link\be.file.h>

#include <string>
#include <map>
#include "Types.h"
#include <ShlObj.h>
#include "Utils.h"
