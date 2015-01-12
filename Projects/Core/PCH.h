// PCH.h : include file for standard system include files,
// or project specific include files that are used frequently, but
// are changed infrequently
//

#pragma once

#include "targetver.h"

#define		WIN32_LEAN_AND_MEAN             // Exclude rarely-used stuff from Windows headers
#ifndef _CRT_SECURE_NO_WARNINGS
	#define		_CRT_SECURE_NO_WARNINGS
#endif // !_CRT_SECURE_NO_WARNINGS



// TODO: reference additional headers your program requires here
#include <windows.h>

/*
#include <fstream>
#include <cassert>
#include <stddef.h>  // For ptrdiff_t
*/
#include <stdlib.h>
#include <tchar.h>


#include "Src/Assert.h"
#include "Src/Macro.h"
