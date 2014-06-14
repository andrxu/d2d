// stdafx.h : include file for standard system include files,
// or project specific include files that are used frequently, but
// are changed infrequently
//

#pragma once

#include "targetver.h"

#include <stdio.h>
#include <tchar.h>


// reference additional headers your program requires here

#include <wrl.h>
#include <d2d1.h>
#include <dwrite.h>
#include <comdef.h>
#include <crtdbg.h>
#include <wincodec.h>

#pragma comment(lib, "d2d1.lib")
#pragma comment(lib, "dwrite.lib") 
#pragma comment(lib, "windowscodecs.lib")

#ifndef CHECKHR
#define CHECKHR(expr) do {hr = (expr); _ASSERT(SUCCEEDED(hr)); if (FAILED(hr)) return(hr);} while(0)
#endif
