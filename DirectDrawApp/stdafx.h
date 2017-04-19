// stdafx.h : include file for standard system include files,
// or project specific include files that are used frequently, but
// are changed infrequently
//

#pragma once

#include "targetver.h"

// Windows Header Files:
#undef WIN32_LEAN_AND_MEAN
#include <windows.h>

#define VELIYA_DEBUG

// C RunTime Header Files
#include <stdlib.h>
#include <malloc.h>
#include <memory.h>
#include <tchar.h>
#include <strsafe.h>
#include "DrawEngine.h"
#include "PrivateMacro.h"

typedef icu::UnicodeString UniString;