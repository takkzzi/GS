//For Assertion
#pragma once

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

	#ifdef _WIN64

		#undef	ASSERT
		#undef	assert

	#include <Assert.h>
		#define ASSERT(Expr)	(void)( (!!(Expr)) || (_wassert(_CRT_WIDE(#Expr), _CRT_WIDE(__FILE__), __LINE__), 0) )

	#include <DbgHelp.h>

	#else

		#pragma warning( push )
		#pragma warning( disable : 4996 )
		#include "Externals/Bugslayer/BugslayerUtil.h"
		#pragma warning( pop )
		#undef new

		#undef	ASSERT
		#undef assert

		#define ASSERT(Expr)		SUPERASSERT(Expr)

	#endif

#else
		#define ASSERT(Expr)	(VOID(0))

#endif //#ifdef WIN32


