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
		static void			Finalize();
	};
}

#ifdef __cpluscplus
}
#endif // __cpluscplus


#include "Macro.h"
#include "Assert.h"
#include "System.h"
#include "Math.h"
#include "Dump.h"
//#include "MemLeakDetect.h"

#ifdef _DEBUG
#include "Externals/Debugging/vld.h"	//Visual Leak Detector : Need "vld.lib"
#endif // _DEBUG

#include "Logger.h"

