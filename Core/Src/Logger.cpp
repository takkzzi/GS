#include "PCH.h"
#include "Logger.h"
#include <stdarg.h>
#include "StrUtil.h"

#define				MAX_LOG_BUFFER		2048

using namespace Core;
using namespace std;


TCHAR							Logger::msLogPath[MAX_PATH];
map<const LPTSTR, FILE*>		Logger::msFileMap;

void Logger::Init()
{
	GetCurrentDirectory(MAX_PATH, msLogPath);
	_sntprintf(msLogPath, MAX_PATH, _T("%s%s"), msLogPath, _T("\\Log\\"));
}

void Logger::Shutdown() 
{
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
	va_list		ap;
	TCHAR		logBuff[MAX_LOG_BUFFER]	= {0,};
	va_start(ap, logData);
	_vstprintf(logBuff, MAX_LOG_BUFFER, logData, ap);
	va_end(ap);

	LogWithDate(_T("[Error]"), logData);

	/*
	if ( ::IsDebuggerPresent() ) {
		DebugBreak();
	}
	*/
	ASSERT(0);
}

void Logger::LogError(const CHAR* logData, ...)
{
	va_list		ap;
	CHAR		logBuff[MAX_LOG_BUFFER]	= {0,};
	va_start(ap, logData);
	vsprintf(logBuff, logData, ap);
	va_end(ap);

	LogWithDate("[Error]", logData);

	/*
	if ( ::IsDebuggerPresent() ) {
		DebugBreak();
	}
	*/
	ASSERT(0);
}


void Logger::LogWarning(const LPTSTR logData, ...)
{
	va_list		ap;
	TCHAR		logBuff[MAX_LOG_BUFFER]	= {0,};
	va_start(ap, logData);
	_vstprintf(logBuff, MAX_LOG_BUFFER, logData, ap);
	va_end(ap);

	LogWithDate(_T("[Warning]"), logData);

	/*
	if ( ::IsDebuggerPresent() ) {
		DebugBreak();
	}
	*/
	ASSERT(0);
}

void Logger::LogWarning(const CHAR* logData, ...)
{
	va_list		ap;
	CHAR		logBuff[MAX_LOG_BUFFER]	= {0,};
	va_start(ap, logData);
	vsprintf(logBuff, logData, ap);
	va_end(ap);

	LogWithDate("[Warning]", logData);

	/*
	if ( ::IsDebuggerPresent() ) {
		DebugBreak();
	}
	*/
	ASSERT(0);
}

void Logger::Log(const LPTSTR category, const LPTSTR logData, ...)
{
	FILE* file = FindFile(category);

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
#endif // DEBUG	
	}
}

void Logger::Log(const CHAR* category, const CHAR* logData, ...)
{
	FILE* file = FindFile(category);

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
#endif // DEBUG	
	}
}

void Logger::LogWithDate(const LPTSTR category, const LPTSTR logData, ...)
{
	FILE* file = FindFile(category);

	SYSTEMTIME	SystemTime;
	GetLocalTime(&SystemTime);

	TCHAR		CurrentDate[32]					= {0,};
	_sntprintf(CurrentDate, 32, _T("%d-%d-%d %d:%d:%d"),
		SystemTime.wYear, 
		SystemTime.wMonth, 
		SystemTime.wDay, 
		SystemTime.wHour,
		SystemTime.wMinute,
		SystemTime.wSecond);

	TCHAR		debugBuff[MAX_LOG_BUFFER]		= {0,};
	TCHAR		logBuff[MAX_LOG_BUFFER]			= {0,};

	va_list		ap;
	va_start(ap, logData);
	_vstprintf(logBuff, MAX_LOG_BUFFER, logData, ap);
	va_end(ap);

	_ftprintf(file, _T("[%s] %s\n"), CurrentDate, logBuff);
	fflush(file);

	//fclose(FilePtr);

	if ( IsDebuggerPresent() ) {
		_sntprintf(debugBuff, MAX_LOG_BUFFER, _T("[%s] %s\n"), CurrentDate, logBuff);
		OutputDebugString(debugBuff);

#ifdef _CONSOLE
		_tprintf(_T("%s"), debugBuff);
#endif // DEBUG	
	}
}

void Logger::LogWithDate(const CHAR* category, const CHAR* logData, ...)
{
	FILE* file = FindFile(category);

	SYSTEMTIME	SystemTime;
	GetLocalTime(&SystemTime);

	CHAR		CurrentDate[32]					= {0,};
	_snprintf(CurrentDate, 32, "%d-%d-%d %d:%d:%d",
		SystemTime.wYear, 
		SystemTime.wMonth, 
		SystemTime.wDay, 
		SystemTime.wHour,
		SystemTime.wMinute,
		SystemTime.wSecond);

	CHAR		debugBuff[MAX_LOG_BUFFER]		= {0,};
	CHAR		logBuff[MAX_LOG_BUFFER]			= {0,};

	va_list		ap;
	va_start(ap, logData);
	vsprintf(logBuff, logData, ap);
	va_end(ap);

	fprintf(file, "[%s] %s\n", CurrentDate, logBuff);
	fflush(file);

	//fclose(FilePtr);

	if ( IsDebuggerPresent() ) {
		_snprintf(debugBuff, MAX_LOG_BUFFER, "[%s] %s\n", CurrentDate, logBuff);
		OutputDebugStringA(debugBuff);

#ifdef _CONSOLE
		printf("%s", debugBuff);
#endif // DEBUG	
	}
}

FILE* Logger::FindFile(const LPTSTR name)
{
	auto search = msFileMap.find(name);
    if(search != msFileMap.end()) {
		return (FILE*)search->second;
	}

	TCHAR		fileName[MAX_PATH]		= {0,};
	_sntprintf(fileName, MAX_PATH, _T("%s%s.log"), msLogPath, name);

	FILE* file = _tfopen(fileName, _T("a"));
	if (!file)
		return FALSE;

	msFileMap[name] = file;
	return file;
}

FILE* Logger::FindFile(const CHAR* name)
{
	TCHAR nameTemp[128] = {0, };
	StrUtil::CopyAnsi2TCHAR(nameTemp, name);

	return FindFile(nameTemp);
}