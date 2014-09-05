//For Assertion
#pragma once

#if defined WIN32 | _WIN64


#define NOGDICAPMASKS
//#define NOMENUS
#define NORASTEROPS
#define NOATOM
#define NODRAWTEXT
#define NOKERNEL
#define NOMEMMGR
#define NOMETAFILE
#define NOMINMAX
#define NOOPENFILE
#define NOSCROLL
#define NOSERVICE
#define NOSOUND
#define NOCOMM
#define NOKANJI
#define NOHELP
#define NOPROFILER
#define NODEFERWINDOWPOS
#define NOMCX
#define NOCRYPT

// Define these for MFC projects
#define NOTAPE
#define NOIMAGE
#define NOPROXYSTUB
#define NORPC


#ifdef USE_ASSERT

	#pragma warning( push )
	#pragma warning( disable : 4996 )
	#include "Externals/Debugging/BugslayerUtil.h"
	#pragma warning( pop )
	//#undef new

#ifdef ASSERT
	#undef	ASSERT
#endif // ASSERT

	#define ASSERT(Expr)		SUPERASSERT(Expr)
	
#else
	#define			ASSERT(Expr)	
#endif
	
#endif //#ifdef WIN32


