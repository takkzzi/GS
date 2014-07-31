//For Assertion
#pragma once

#ifdef WIN32

/*
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
*/

#ifdef ASSERT
	#undef		ASSERT
#endif // ASSERT

#ifdef USE_ASSERT

	

	#ifdef ASSERT_DUMP		//For Server Build
		#define ASSERT(Expr)		ASSERT_DUMP(Expr)		//TODO:	 Full-Dumping
	#else
		#pragma warning( push )
		#pragma warning( disable : 4996 )
		#include "Externals/Debugging/BugslayerUtil.h"
		#undef new
		#define ASSERT(Expr)		SUPERASSERT(Expr)
		#pragma warning( pop )
	#endif

#else


	
#endif //#ifdef WIN32


