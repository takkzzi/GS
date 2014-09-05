#pragma once

#if defined(_MSC_VER) && defined (_DEBUG)

#define _CRTDBG_MAP_ALLOC // �޸� ������ Ž���ϱ� ���� ���� ���־�� �Ѵ�.
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

			// Consol Application�� ���
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


			/* new�� �Ҵ�� �޸𸮿� ������ ���� ��� �ҽ����� 
			��Ȯ�� ��ġ�� �������ش�.
			*  �� _AFXDLL�� ����Ҷ��� �ڵ����� ������ CONSOLE 
			��忡�� �Ʒ�ó��
			* �����Ǹ� ���־�߸� �մϴ�.
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
			* malloc���� �Ҵ�� �޸𸮿� ������ ���� ��� ��ġ
			�� ����
			* CONSOLE ����� ��� crtdbg.h�� malloc�� ���ǵǾ� 
			������,
			* _AXFDLL ����� ��쿡�� �ణ �ٸ�������� ���� ��
			�Եȴ�.
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
