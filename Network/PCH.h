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
#include <vector>
#include <list>
#include <map>
#include <tchar.h>

#include <WinSock2.h>
#include <mswsock.h>

#include "Src/Defines.h"
#include "Core/Core.h"
