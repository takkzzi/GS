#include "PCH.h"
#include "Dump.h"
#include "System.h"


#ifndef USE_ASSERT
	#include <DbgHelp.h>	//Assert.h의 #include "Externals/Bugslayer/BugslayerUtil.h" 와 중복 방지
#endif // !USE_ASSERT	

using namespace Core;


MINIDUMP_TYPE	g_dumpType = MiniDumpWithFullMemory;
LPTOP_LEVEL_EXCEPTION_FILTER g_previousExceptionFilter = NULL;


static void AcquireExceptionReason(DWORD dwExceptionCode, OUT TCHAR* pszStr, size_t strSize)
{
	struct ExceptionNames
	{
		DWORD		dwExceptionCode;
		TCHAR*		pszExceptionName;
	};

	ExceptionNames sExceptionMap[] =
	{
		{0x40010005, _T("Control-C로 인한 오류가 발생하였습니다.")},
		{0x40010008, _T("Control-Break로 인한 오류가 발생하였습니다.")},
		{0x80000002, _T("정렬되지 않은 데이타에 접근하였습니다.")},
		{0x80000003, _T("Breakpoint가 걸렸습니다.")},
		{0xc0000005, _T("잘못된 참조가 발생하였습니다")},
		{0xc0000006, _T("page를 Load할 수 없어 진행 할 수 없습니다.")},
		{0xc0000017, _T("Memory할당에 실패 하였습니다.")},
		{0xc000001d, _T("잘못된 명령어를 실행하였습니다.")},
		{0xc0000025, _T("오류로 인해 더이상 진행 시킬수 없습니다.")},
		{0xc0000026, _T("잘못된 기능에 접근 하였습니다.")},
		{0xc000008c, _T("배열 범위를 초과 하였습니다.")},
		{0xc000008d, _T("Float정규화가 되지않는 작은값 입니다.")},
		{0xc000008e, _T("Float를 0으로 나누었습니다.")},
		{0xc000008f, _T("Float소수를 정확하게 나타낼수 없습니다.")},
		{0xc0000090, _T("Float에서 알수없는 오류가 발생하였습니다.")},
		{0xc0000091, _T("Float Overflow가 발생하였습니다.")},
		{0xc0000092, _T("Float Stack값을 넘었거나 값이 너무 작습니다.")},
		{0xc0000093, _T("Float 값이 너무 작습니다.")},
		{0xc0000094, _T("Integer를 0으로 나누었습니다.")},
		{0xc0000095, _T("Integer Overflow가 발생하였습니다.")},
		{0xc0000096, _T("해당 명령어를 실행 할 수 없습니다.")},
		{0xc00000fD, _T("Stack Overflow가 발생하였습니다.")},
		{0xc0000142, _T("DLL을 초기화 할 수 없습니다.")},
		{0xe06d7363, _T("Microsoft C++에서 오류가 발생하였습니다.")},
	};

	size_t nStringLen = strSize / sizeof( TCHAR );
	int nCount = sizeof( sExceptionMap ) / sizeof( sExceptionMap[0] );
	for( int i = 0; i < nCount; ++i )
	{
		if( dwExceptionCode == sExceptionMap[i].dwExceptionCode )
		{
			::_tcsncpy( pszStr, sExceptionMap[i].pszExceptionName, nStringLen );
			pszStr[nStringLen - 1] = NULL;
			return;
		}
	}

	::_tcsncpy( pszStr, _T("정확한 오류 원인을 찾을 수 없습니다."), nStringLen );
	pszStr[nStringLen - 1] = NULL;
}

typedef BOOL (WINAPI *MINIDUMPWRITEDUMP)( // Callback 함수의 원형
	HANDLE hProcess, 
	DWORD dwPid, 
	HANDLE hFile, 
	MINIDUMP_TYPE DumpType,
	CONST PMINIDUMP_EXCEPTION_INFORMATION ExceptionParam,
	CONST PMINIDUMP_USER_STREAM_INFORMATION UserStreamParam,
	CONST PMINIDUMP_CALLBACK_INFORMATION CallbackParam);

static LONG DoSaveMiniDump( _EXCEPTION_POINTERS* pException, const TCHAR* fileName, const TCHAR* userStr, UINT userStrLen )
{
	LONG lResult = EXCEPTION_CONTINUE_SEARCH;

	HMODULE DllHandle = LoadLibrary(_T("DBGHELP.DLL"));
	if( ! DllHandle )
		return EXCEPTION_CONTINUE_SEARCH;

	MINIDUMPWRITEDUMP MiniDumpFunc = (MINIDUMPWRITEDUMP) GetProcAddress(DllHandle, "MiniDumpWriteDump");
	if ( ! MiniDumpFunc )
		return EXCEPTION_CONTINUE_SEARCH;

	HANDLE hFile = ::CreateFile( fileName, GENERIC_WRITE, FILE_SHARE_WRITE, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL );
	if( INVALID_HANDLE_VALUE == hFile )
		return lResult;

	HANDLE	hProcess = GetCurrentProcess();
	DWORD	dwProcessID = GetCurrentProcessId();
	DWORD	dwThreadID = GetCurrentThreadId();

	MINIDUMP_EXCEPTION_INFORMATION sExceptionInfo;
	ZeroMemory( &sExceptionInfo, sizeof(MINIDUMP_EXCEPTION_INFORMATION) );

	sExceptionInfo.ThreadId = dwThreadID;
	sExceptionInfo.ExceptionPointers = pException;
	sExceptionInfo.ClientPointers = false;

	MINIDUMP_USER_STREAM_INFORMATION sUserInfo;
	MINIDUMP_USER_STREAM sUseData;

	sUseData.Type = LastReservedStream + 1;
	sUseData.Buffer = (TCHAR*)userStr;
	sUseData.BufferSize = sizeof(TCHAR) * userStrLen;

	sUserInfo.UserStreamCount = 1;
	sUserInfo.UserStreamArray = &sUseData;


	/*
	if( pkException->ExceptionRecord->ExceptionCode == EXCEPTION_ARRAY_BOUNDS_EXCEEDED )
	{
		if( MiniDumpFunc( hProcess, dwProcessID, hFile, MiniDumpNormal, &sExceptionInfo, &sUserInfo, NULL ))
			lResult = EXCEPTION_CONTINUE_EXECUTION;
	}
	else
	{
		if( MiniDumpFunc( hProcess, dwProcessID, hFile, g_dumpType, &sExceptionInfo, &sUserInfo, NULL ))
			lResult = EXCEPTION_EXECUTE_HANDLER;
	}
	*/
	if( MiniDumpFunc(hProcess, dwProcessID, hFile, g_dumpType, &sExceptionInfo, &sUserInfo, NULL) )
		lResult = EXCEPTION_EXECUTE_HANDLER;

	::CloseHandle( hFile );

	return lResult;
}

static LONG WINAPI ExceptionFilter( _EXCEPTION_POINTERS* pkException )
{
	TCHAR szfileName[128];
	const SYSTEMTIME* sysTime = Time::GetSystemTime();
	_sntprintf(szfileName, MAX_PATH, _T("%d-%d-%d %d_%d_%d.dmp"), sysTime->wYear, sysTime->wMonth, sysTime->wDay, sysTime->wHour, sysTime->wMinute, sysTime->wSecond);

	TCHAR szResonStr[128];
	AcquireExceptionReason(pkException->ExceptionRecord->ExceptionCode, szResonStr, sizeof(szResonStr));

	LONG lResult = DoSaveMiniDump( pkException, szfileName, szResonStr, sizeof(szResonStr)/sizeof(TCHAR));

	return lResult;
}

void Dump::Init()
{
	//::SetErrorMode(SEM_FAILCRITICALERRORS);
	g_dumpType = MiniDumpWithFullMemory;
	g_previousExceptionFilter = SetUnhandledExceptionFilter(ExceptionFilter);
}

void Dump::Shutdown(VOID)
{
	::SetUnhandledExceptionFilter(g_previousExceptionFilter);
}
