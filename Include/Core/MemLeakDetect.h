#pragma once

#if defined(_MSC_VER) && defined (_DEBUG)

#define _CRTDBG_MAP_ALLOC // 메모리 누수를 탐지하기 위해 선언 해주어야 한다.
#include <crtdbg.h>
#if !defined (_CONSOLE)
#include <cstdlib> // for Consol Application
#endif

#ifdef __cpluscplus
extern "C" {
#endif // __cpluscplus

namespace Core 
{
	class MemLeakDetect
	{
	private:
		MemLeakDetect ()
		{
			_CrtSetDbgFlag( _CRTDBG_ALLOC_MEM_DF | 
				_CRTDBG_LEAK_CHECK_DF);

			// Consol Application인 경우
	#if defined (_CONSOLE_OUTPUT)
			// Send all reports to STDOUT
			_CrtSetReportMode( _CRT_WARN,   
				_CRTDBG_MODE_FILE   );
			_CrtSetReportFile( _CRT_WARN,   
				_CRTDBG_FILE_STDOUT );
			_CrtSetReportMode( _CRT_ERROR,  
				_CRTDBG_MODE_FILE   );
			_CrtSetReportFile( _CRT_ERROR,  
				_CRTDBG_FILE_STDOUT );
			_CrtSetReportMode( _CRT_ASSERT, 
				_CRTDBG_MODE_FILE   );
			_CrtSetReportFile( _CRT_ASSERT, 
				_CRTDBG_FILE_STDOUT );


			/* new로 할당된 메모리에 누수가 있을 경우 소스상의 
			정확한 위치를 덤프해준다.
			*  ※ _AFXDLL을 사용할때는 자동으로 되지만 CONSOLE 
			모드에서 아래처럼
			* 재정의를 해주어야만 합니다.
			*    date: 2000-12-05
			*/
	#define DEBUG_NORMALBLOCK   new ( _NORMAL_BLOCK, __FILE__, __LINE__ )
	#ifdef new
	#undef new
	#endif
	#define new DEBUG_NORMALBLOCK

	#else

			// Send all reports to DEBUG window
			_CrtSetReportMode( _CRT_WARN,   
				_CRTDBG_MODE_DEBUG  );
			_CrtSetReportMode( _CRT_ERROR,  
				_CRTDBG_MODE_DEBUG  );
			_CrtSetReportMode( _CRT_ASSERT, 
				_CRTDBG_MODE_DEBUG  );

	#endif

	#ifdef malloc
	#undef malloc
	#endif
			/*
			* malloc으로 할당된 메모리에 누수가 있을 경우 위치
			를 덤프
			* CONSOLE 모드일 경우 crtdbg.h에 malloc이 정의되어 
			있지만,
			* _AXFDLL 모드일 경우에는 약간 다른방식으로 덤프 하
			게된다.
			* date: 2001-01-30
			*/
	#define malloc(s) (_malloc_dbg( s, _NORMAL_BLOCK, __FILE__, __LINE__ ))
		}

	public:
		static	void		Init()				{ static MemLeakDetect	mld; }
	};

}	//namespace Core

#ifdef __cpluscplus
}
#endif // __cpluscplus

#endif // if defined(_MSC_VER) && defined (_DEBUG)
