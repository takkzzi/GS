#include "PCH.h"
#include <stdarg.h>
#include <strsafe.h>

#include "Logger.h"
#include "StrUtil.h"
#include "System.h"


#define				MAX_LOG_BUFFER		2048
#define				LOG_DIR				_T("Log")

using namespace Core;
using namespace std;

volatile	bool				Logger::msInit = false;
TCHAR							Logger::msLogPath[MAX_PATH];
map<const LPTSTR, FILE*>		Logger::msFileMap;


void Logger::Init()
{
	GetCurrentDirectory(MAX_PATH, msLogPath);
	_sntprintf(msLogPath, MAX_PATH, _T("%s\\%s\\"), msLogPath, LOG_DIR);
	CreateDirectory(msLogPath, NULL);

	msInit = true;
}

void Logger::Shutdown() 
{
	msInit = false;

	ThreadSync sync;
	
	for(auto &i : msFileMap)
	{
		FILE* file = i.second;
		//fflush(file);
		fclose(file);
	}
	msFileMap.clear();
}

void Logger::LogError(const LPTSTR logData, ...)
{
	if ( ! msInit ) return;
	ThreadSync sync;

	va_list		ap;
	TCHAR		logBuff[MAX_LOG_BUFFER]	= {0,};
	va_start(ap, logData);
	_vstprintf(logBuff, MAX_LOG_BUFFER, logData, ap);
	va_end(ap);

	LogWithDate(_T("[Error]"), logData);

	ASSERT(0);
}

void Logger::LogError(const CHAR* logData, ...)
{
	if ( ! msInit ) return;
	ThreadSync sync;

	va_list		ap;
	CHAR		logBuff[MAX_LOG_BUFFER]	= {0,};
	va_start(ap, logData);
	vsprintf(logBuff, logData, ap);
	va_end(ap);

	LogWithDate("[Error]", logData);

	ASSERT(0);
}


void Logger::LogWarning(const LPTSTR logData, ...)
{
	if ( ! msInit ) return;
	ThreadSync sync;

	va_list		ap;
	TCHAR		logBuff[MAX_LOG_BUFFER]	= {0,};
	va_start(ap, logData);
	_vstprintf(logBuff, MAX_LOG_BUFFER, logData, ap);
	va_end(ap);

	LogWithDate(_T("[Warning]"), logData);
}

void Logger::LogWarning(const CHAR* logData, ...)
{
	if ( ! msInit  ) return;
	ThreadSync sync;

	va_list		ap;
	CHAR		logBuff[MAX_LOG_BUFFER]	= {0,};
	va_start(ap, logData);
	vsprintf(logBuff, logData, ap);
	va_end(ap);

	LogWithDate("[Warning]", logData);
}

void Logger::Log(const LPTSTR category, const LPTSTR logData, ...)
{
	if ( ! msInit  ) return;
	ThreadSync sync;

	FILE* file = FindFile(category);
	if ( ! file ) return;

	TCHAR		debugLogBuff[MAX_LOG_BUFFER]		= {0,};
	TCHAR		logBuff[MAX_LOG_BUFFER]	= {0,};

	va_list		ap;
	va_start(ap, logData);
	_vstprintf(logBuff, MAX_LOG_BUFFER, logData, ap);
	va_end(ap);

	_ftprintf(file, _T("%s\n"), logBuff);
	fflush(file);

	if ( IsDebuggerPresent() ) {
		//_sntprintf(debugLogBuff, MAX_LOG_BUFFER, _T("[%s] %s\n"), CurrentDate, logBuff);
		_sntprintf(debugLogBuff, MAX_LOG_BUFFER, _T(" %s\n"), logBuff);
		OutputDebugString(debugLogBuff);

#ifdef _CONSOLE
		_tprintf(_T("%s"), debugLogBuff);
#endif
	}
}

void Logger::Log(const CHAR* category, const CHAR* logData, ...)
{
	if ( ! msInit  ) return;
	ThreadSync sync;

	FILE* file = FindFile(category);
	if ( ! file ) return;

	CHAR		debugLogBuff[MAX_LOG_BUFFER]		= {0,};
	CHAR		logBuff[MAX_LOG_BUFFER]				= {0,};

	va_list		ap;
	va_start(ap, logData);
	vsprintf(logBuff, logData, ap);
	va_end(ap);

	fprintf(file, "%s\n", logBuff);
	fflush(file);

	if ( IsDebuggerPresent() ) {
		_snprintf(debugLogBuff, MAX_LOG_BUFFER, " %s\n", logBuff);
		OutputDebugStringA(debugLogBuff);

#ifdef _CONSOLE
		_tprintf("%s", debugLogBuff);
#endif
	}
}

void Logger::LogWithDate(const LPTSTR category, const LPTSTR logData, ...)
{
	if ( ! msInit  ) return;
	ThreadSync sync;

	FILE* file = FindFile(category);
	if ( ! file ) return;

	const TCHAR* timeStr = Time::GetSystemTimeStr();

	TCHAR		debugBuff[MAX_LOG_BUFFER]		= {0,};
	TCHAR		logBuff[MAX_LOG_BUFFER]			= {0,};

	va_list		ap;
	va_start(ap, logData);
	_vstprintf(logBuff, MAX_LOG_BUFFER, logData, ap);
	va_end(ap);

	_ftprintf(file, _T("[%s] %s (%s - %d line)\n"), timeStr, logBuff, _T(__FILE__), __LINE__);
	fflush(file);

	if ( IsDebuggerPresent() ) {
		_sntprintf(debugBuff, MAX_LOG_BUFFER, _T("[%s] %s (%s - %d line)\n"), timeStr, logBuff, _T(__FILE__), __LINE__);
		OutputDebugString(debugBuff);

#ifdef _CONSOLE
		_tprintf(_T("%s"), debugBuff);
#endif
	}
}

void Logger::LogWithDate(const CHAR* category, const CHAR* logData, ...)
{
	if ( ! msInit ) return;
	ThreadSync sync;

	FILE* file = FindFile(category);
	if ( ! file ) return;
	
	const TCHAR* currTime = Time::GetSystemTimeStr();
	const CHAR* currTimeA = StringUtil::AnsiFromTCHAR(currTime);

	CHAR		debugBuff[MAX_LOG_BUFFER]		= {0,};
	CHAR		logBuff[MAX_LOG_BUFFER]			= {0,};

	va_list		ap;
	va_start(ap, logData);
	vsprintf(logBuff, logData, ap);
	va_end(ap);

	fprintf(file, "[%s] %s (%s - %d line)\n", currTimeA, logBuff, __FILE__, __LINE__);
	fflush(file);

	//fclose(FilePtr);

	if ( IsDebuggerPresent() ) {
		_snprintf(debugBuff, MAX_LOG_BUFFER, "[%s] %s (%s - %d line)\n", currTimeA, logBuff, __FILE__, __LINE__);
		OutputDebugStringA(debugBuff);

#ifdef _CONSOLE
		printf("%s", debugBuff);
#endif
	}
}

FILE* Logger::FindFile(const LPTSTR name)
{
	if ( ! msInit  ) return NULL;
	ThreadSync sync;

	auto search = msFileMap.find(name);
    if(search != msFileMap.end()) {
		return (FILE*)search->second;
	}

	TCHAR		fileName[MAX_PATH]		= {0,};
	_sntprintf(fileName, MAX_PATH, _T("%s%s.log"), msLogPath, name);

	FILE* file = _tfopen(fileName, _T("w+"));
	ASSERT(file && "Logger::FindFile() Failed");

	if (!file)
		return NULL;

	msFileMap[name] = file;
	return file;
}

FILE* Logger::FindFile(const CHAR* name)
{
	if ( ! msInit  ) return NULL;
	ThreadSync sync;

	TCHAR nameTemp[128] = {0, };
	StringUtil::CopyAnsi2TCHAR(nameTemp, name);

	return FindFile(nameTemp);
}

LPTSTR Logger::GetLastErrorMsg(const LPTSTR funcName, const DWORD errorCode, bool bMsgBox/*=false*/)
{
	if ( ! msInit  ) return NULL;
	ThreadSync sync;

	static TCHAR totalMsg[256];
	TCHAR* errorMsg = NULL;

    ::FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS, 
		NULL, errorCode, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), (LPWSTR)&errorMsg, 0, NULL );
	
	::_sntprintf((LPTSTR)totalMsg, sizeof(totalMsg) / sizeof(TCHAR), TEXT("Failed with error %d: %s"), errorCode, errorMsg);

	if ( bMsgBox )
		::MessageBox(NULL, (LPCTSTR)totalMsg, TEXT("Error"), MB_OK);

	return totalMsg;
}