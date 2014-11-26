//Representative Header For External Using

#pragma once

//#define WIN32_LEAN_AND_MEAN      // Exclude rarely-used stuff from Windows headers
//#include <windows.h>

namespace Core 
{
	static class CoreSystem
	{
	public:
		static void			Init(const TCHAR* logDirName);
		static void			Shutdown();

	private:
		static bool			msInit;
	};
}


using namespace Core;

#ifndef WIN32_LEAN_AND_MEAN             
	#define	WIN32_LEAN_AND_MEAN		// Exclude rarely-used stuff from Windows headers
#endif

#ifndef _CRT_SECURE_NO_WARNINGS
	#define		_CRT_SECURE_NO_WARNINGS
#endif // !_CRT_SECURE_NO_WARNINGS

#include <windows.h>

#ifdef _DEBUG
#include "Externals/VisualLeakDetector/vld.h"	//Visual Leak Detector : Need "vld.lib"
#endif // _DEBUG

#include <tchar.h>
#include <vector>
#include <list>
#include <map>

#include "MemLeakDetect.h"
#include "Macro.h"
#include "Assert.h"
#include "System.h"
#include "Math.h"
#include "Dump.h"
#include "Logger.h"
#include "Thread.h"
#include "MemPoolObject.h"




