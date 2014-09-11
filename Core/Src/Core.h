//Representative Header For External Using

#pragma once

//#define WIN32_LEAN_AND_MEAN      // Exclude rarely-used stuff from Windows headers
//#include <windows.h>

#ifdef __cpluscplus
extern "C" {
#endif // __cpluscplus

namespace Core 
{
	class CoreSystem
	{
	public:
		static void			Init();
		static void			Shutdown();
	};
}

#ifdef __cpluscplus
}
#endif // __cpluscplus

using namespace Core;

#ifndef WIN32_LEAN_AND_MEAN             
	#define	WIN32_LEAN_AND_MEAN		// Exclude rarely-used stuff from Windows headers
#endif

#ifndef _CRT_SECURE_NO_WARNINGS
	#define		_CRT_SECURE_NO_WARNINGS
#endif // !_CRT_SECURE_NO_WARNINGS

#include <windows.h>


#include "MemLeakDetect.h"
#include "Macro.h"
#include "Assert.h"
#include "System.h"
#include "Math.h"
#include "Dump.h"
#include "Logger.h"
#include "Thread.h"



#ifdef _DEBUG
#include "Externals/Debugging/vld.h"	//Visual Leak Detector : Need "vld.lib"
#endif // _DEBUG



