#include "PCH.h"
#include "Dump.h"

#ifndef USE_ASSERT
	#include <DbgHelp.h>	//Assert.h�� #include "Externals/Bugslayer/BugslayerUtil.h" �� �ߺ� ����
#endif // !USE_ASSERT	

using namespace Core;

#ifndef    _WIN64		
typedef BOOL (WINAPI *MINIDUMPWRITEDUMP)( // Callback �Լ��� ����
	HANDLE hProcess, 
	DWORD dwPid, 
	HANDLE hFile, 
	MINIDUMP_TYPE DumpType,
	CONST PMINIDUMP_EXCEPTION_INFORMATION ExceptionParam,
	CONST PMINIDUMP_USER_STREAM_INFORMATION UserStreamParam,
	CONST PMINIDUMP_CALLBACK_INFORMATION CallbackParam);

LONG WINAPI UnHandledExceptionFilter(struct _EXCEPTION_POINTERS *exceptionInfo)
{
	HMODULE	DllHandle		= NULL;

	// Windows 2000 �������� ���� DBGHELP�� �����ؼ� ������ �־�� �Ѵ�.
	DllHandle				= LoadLibrary(_T("DBGHELP.DLL"));

	if (DllHandle)
	{
		MINIDUMPWRITEDUMP Dump = (MINIDUMPWRITEDUMP) GetProcAddress(DllHandle, "MiniDumpWriteDump");

		if (Dump)
		{
			TCHAR		DumpPath[MAX_PATH] = {0,};
			SYSTEMTIME	SystemTime;

			GetLocalTime(&SystemTime);

			_sntprintf(DumpPath, MAX_PATH, _T("%d-%d-%d %d_%d_%d.dmp"), 
				SystemTime.wYear,
				SystemTime.wMonth,
				SystemTime.wDay,
				SystemTime.wHour,
				SystemTime.wMinute,
				SystemTime.wSecond);
			
			HANDLE FileHandle = CreateFile(
				DumpPath, 
				GENERIC_WRITE, 
				FILE_SHARE_WRITE, 
				NULL, CREATE_ALWAYS, 
				FILE_ATTRIBUTE_NORMAL, 
				NULL);

			if (FileHandle != INVALID_HANDLE_VALUE)
			{
				_MINIDUMP_EXCEPTION_INFORMATION MiniDumpExceptionInfo;
				
				MiniDumpExceptionInfo.ThreadId			= GetCurrentThreadId();
				MiniDumpExceptionInfo.ExceptionPointers	= exceptionInfo;
				MiniDumpExceptionInfo.ClientPointers	= NULL;

				BOOL Success = Dump(
					GetCurrentProcess(), 
					GetCurrentProcessId(), 
					FileHandle, 
					MiniDumpNormal, 
					&MiniDumpExceptionInfo, 
					NULL, 
					NULL);

				if (Success)
				{
					CloseHandle(FileHandle);

					return EXCEPTION_EXECUTE_HANDLER;
				}
			}

			CloseHandle(FileHandle);
		}
	}

	return EXCEPTION_CONTINUE_SEARCH;
}
#else
LONG WINAPI UnHandledExceptionFilter(struct _EXCEPTION_POINTERS *exceptionInfo)
{
	return 0;
}
#endif

LPTOP_LEVEL_EXCEPTION_FILTER PreviousExceptionFilter = NULL;

void Dump::Init()
{
	SetErrorMode(SEM_FAILCRITICALERRORS);
	PreviousExceptionFilter = SetUnhandledExceptionFilter(UnHandledExceptionFilter);
}

void Dump::Shutdown(VOID)
{
	SetUnhandledExceptionFilter(PreviousExceptionFilter);
}
