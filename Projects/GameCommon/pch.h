//
// pch.h
// Header for standard system include files.
//

#pragma once

#include "targetver.h"

#define WIN32_LEAN_AND_MEAN             // Exclude rarely-used stuff from Windows headers
#ifndef _CRT_SECURE_NO_WARNINGS
	#define		_CRT_SECURE_NO_WARNINGS
#endif // !_CRT_SECURE_NO_WARNINGS

// Windows Header Files:
#include <windows.h>

//#include <stdlib.h>
//#include <tchar.h>

#include "Core\Core.h"
#include "Network\Network.h"


