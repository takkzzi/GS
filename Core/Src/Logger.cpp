#include "PCH.h"
#include "Logger.h"
#include <stdarg.h>

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

bool Logger::LogError(LPTSTR logData, ...)
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
	
	return TRUE;
}

bool Logger::LogWarning(LPTSTR logData, ...)
{
	va_list		ap;
	TCHAR		logBuff[MAX_LOG_BUFFER]	= {0,};
	va_start(ap, logData);
	_vstprintf(logBuff, MAX_LOG_BUFFER, logData, ap);
	va_end(ap);

	LogWithDate(_T("[Warning]"), logData);

	if ( ::IsDebuggerPresent() ) {
		DebugBreak();
	}

	return TRUE;
}

bool Logger::Log(const LPTSTR category, LPTSTR logData, ...)
{
	FILE* file = NULL;
	auto search = msFileMap.find(category);
    if(search != msFileMap.end()) {
		file = (FILE*)search->second;
	}
	
	TCHAR		fileName[MAX_PATH]		= {0,};
	TCHAR		debugLogBuff[MAX_LOG_BUFFER]		= {0,};
	TCHAR		logBuff[MAX_LOG_BUFFER]	= {0,};

	if ( file == NULL ) {
		_sntprintf(fileName, MAX_PATH, _T("%s%s.log"), 
			msLogPath, 
			category);

		file = _tfopen(fileName, _T("a"));
		if (!file)
			return FALSE;

		msFileMap[category] = file;;
	}

	SYSTEMTIME	SystemTime;
	GetLocalTime(&SystemTime);

	/*
	TCHAR		CurrentDate[32]					= {0,};
	_sntprintf(CurrentDate, 32, _T("%d-%d-%d %d:%d:%d"),
		SystemTime.wYear, 
		SystemTime.wMonth, 
		SystemTime.wDay, 
		SystemTime.wHour,
		SystemTime.wMinute,
		SystemTime.wSecond);
	*/

	va_list		ap;
	va_start(ap, logData);
	_vstprintf(logBuff, MAX_LOG_BUFFER, logData, ap);
	va_end(ap);

	_ftprintf(file, _T("%s\n"), logBuff);
	fflush(file);

	//fclose(FilePtr);

	if ( IsDebuggerPresent() ) {
		//_sntprintf(debugLogBuff, MAX_LOG_BUFFER, _T("[%s] %s\n"), CurrentDate, logBuff);
		_sntprintf(debugLogBuff, MAX_LOG_BUFFER, _T(" %s\n"), logBuff);
		OutputDebugString(debugLogBuff);

#ifdef _CONSOLE
		_tprintf(_T("%s"), debugLogBuff);
#endif // DEBUG	
	}

	return true;
}

bool Logger::LogWithDate(const LPTSTR category, LPTSTR logData, ...)
{
	FILE* file = NULL;
	auto search = msFileMap.find(category);
    if(search != msFileMap.end()) {
		file = (FILE*)search->second;
	}
	
	TCHAR		fileName[MAX_PATH]		= {0,};
	TCHAR		debugBuff[MAX_LOG_BUFFER]		= {0,};

	TCHAR		logBuff[MAX_LOG_BUFFER]	= {0,};

	if ( file == NULL ) {
		_sntprintf(fileName, MAX_PATH, _T("%s%s.log"), 
			msLogPath, 
			category);

		file = _tfopen(fileName, _T("a"));
		if (!file)
			return FALSE;

		msFileMap[category] = file;
	}

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

	return true;
}

