#include "PCH.h"
#include <stdarg.h>
#include <strsafe.h>

#include "Logger.h"
#include "StrUtil.h"
#include "System.h"
#include "ThreadSync.h"


#define				MAX_LOG_BUFFER		2048
#define				CS_LOCK			mCS->Enter();
#define				CS_UNLOCK		mCS->Leave();

using namespace Core;
using namespace std;

volatile	bool				Logger::msInit = false;
TCHAR							Logger::msLogPath[MAX_PATH];
map<const LPTSTR, FILE*>		Logger::msFileMap;
CriticalSection*				Logger::mCS = NULL;


void Logger::Init(const TCHAR* logDir)
{
	GetCurrentDirectory(MAX_PATH, msLogPath);

	TCHAR timeStr[32];
	const SYSTEMTIME* sysTime = Time::GetSystemTime();
	_sntprintf(timeStr, 32, _T("%d-%d-%d"), sysTime->wYear, sysTime->wMonth, sysTime->wDay);

	_sntprintf(msLogPath, MAX_PATH, _T("%s\\%s\\"), msLogPath, logDir);
	CreateDirectory(msLogPath, NULL);

	_sntprintf(msLogPath, MAX_PATH, _T("%s\\%s\\"), msLogPath, timeStr);
	CreateDirectory(msLogPath, NULL);

	mCS = new CriticalSection();

	msInit = true;
}

void Logger::Shutdown() 
{
	msInit = false;

	CS_LOCK
	{
		for(auto &i : msFileMap)
		{
			FILE* file = i.second;
			//fflush(file);
			fclose(file);
		}
		msFileMap.clear();
	}
	CS_UNLOCK

	SAFE_DELETE(mCS);
}

void Logger::LogError(const LPTSTR logData, ...)
{
	if ( ! msInit ) return;

	CS_LOCK
		va_list		ap;
		TCHAR		logBuff[MAX_LOG_BUFFER]	= {0,};
		va_start(ap, logData);
		_vstprintf(logBuff, MAX_LOG_BUFFER, logData, ap);
		va_end(ap);
	CS_UNLOCK

	LogWithDate(_T("[Error]"), logData);
	ASSERT(0);
	
}

void Logger::LogError(const CHAR* logData, ...)
{
	if ( ! msInit ) return;
	
	CS_LOCK
		va_list		ap;
		CHAR		logBuff[MAX_LOG_BUFFER]	= {0,};
		va_start(ap, logData);
		vsprintf(logBuff, logData, ap);
		va_end(ap);

	CS_UNLOCK

	LogWithDate("[Error]", logData);
	ASSERT(0);
}


void Logger::LogWarning(const LPTSTR logData, ...)
{
	if ( ! msInit ) return;
	
	CS_LOCK
		va_list		ap;
		TCHAR		logBuff[MAX_LOG_BUFFER]	= {0,};
		va_start(ap, logData);
		_vstprintf(logBuff, MAX_LOG_BUFFER, logData, ap);
		va_end(ap);
	CS_UNLOCK

	LogWithDate(_T("[Warning]"), logBuff);
}

void Logger::LogWarning(const CHAR* logData, ...)
{
	if ( ! msInit  ) return;
	
	CS_LOCK
		va_list		ap;
		CHAR		logBuff[MAX_LOG_BUFFER]	= {0,};
		va_start(ap, logData);
		vsprintf(logBuff, logData, ap);
		va_end(ap);
	CS_UNLOCK

	LogWithDate("[Warning]", logBuff);
	
}

void Logger::Log(const LPTSTR category, const LPTSTR logData, ...)
{
	if ( ! msInit  ) return;
	
	FILE* file = FindFile(category);
	if ( ! file ) return;

	CS_LOCK
	{
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
	CS_UNLOCK
}

void Logger::Log(const CHAR* category, const CHAR* logData, ...)
{
	if ( ! msInit  ) return;
	
	FILE* file = FindFile(category);
	if ( ! file ) return;

	CS_LOCK
	{
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
	CS_UNLOCK

	
}

void Logger::LogWithDate(const LPTSTR category, const LPTSTR logData, ...)
{
	if ( ! msInit  ) return;
	
	FILE* file = FindFile(category);
	if ( ! file ) return;

	CS_LOCK
	{
		const TCHAR* timeStr = Time::GetSystemTimeStr();

		TCHAR		debugBuff[MAX_LOG_BUFFER]		= {0,};
		TCHAR		logBuff[MAX_LOG_BUFFER]			= {0,};

		va_list		ap;
		va_start(ap, logData);
		_vstprintf(logBuff, MAX_LOG_BUFFER, logData, ap);
		va_end(ap);

		_ftprintf(file, _T("[%s] %s \n"), timeStr, logBuff);
		fflush(file);

		if ( IsDebuggerPresent() ) {
			_sntprintf(debugBuff, MAX_LOG_BUFFER, _T("[%s] %s \n"), timeStr, logBuff);
			OutputDebugString(debugBuff);

	#ifdef _CONSOLE
			_tprintf(_T("%s"), debugBuff);
	#endif
		}
	}
	CS_UNLOCK
}

void Logger::LogWithDate(const CHAR* category, const CHAR* logData, ...)
{
	if ( ! msInit ) return;
	
	
	FILE* file = FindFile(category);
	if ( ! file ) return;
	
	CS_LOCK
	{
		const TCHAR* currTime = Time::GetSystemTimeStr();
		const CHAR* currTimeA = StringUtil::AnsiFromTCHAR(currTime);

		CHAR		debugBuff[MAX_LOG_BUFFER]		= {0,};
		CHAR		logBuff[MAX_LOG_BUFFER]			= {0,};

		va_list		ap;
		va_start(ap, logData);
		vsprintf(logBuff, logData, ap);
		va_end(ap);

		fprintf(file, "[%s] %s \n", currTimeA, logBuff);
		fflush(file);

		//fclose(FilePtr);

		if ( IsDebuggerPresent() ) {
			_snprintf(debugBuff, MAX_LOG_BUFFER, "[%s] %s \n", currTimeA, logBuff);
			OutputDebugStringA(debugBuff);

	#ifdef _CONSOLE
			printf("%s", debugBuff);
	#endif
		}
	}
	CS_UNLOCK
}

FILE* Logger::FindFile(const LPTSTR name)
{
	if ( ! msInit  ) return NULL;
	
	CS_LOCK

		auto search = msFileMap.find(name);
		if(search != msFileMap.end()) {
			CS_UNLOCK
			return (FILE*)search->second;
		}

		TCHAR		fileName[MAX_PATH]		= {0,};
		_sntprintf(fileName, MAX_PATH, _T("%s%s.log"), msLogPath, name);

		FILE* file = _tfopen(fileName, _T("w+"));
		ASSERT(file && "Logger::FindFile() Failed");

		if (! file) {
			CS_UNLOCK
			return NULL;
		}

		msFileMap[name] = file;

	CS_UNLOCK

	return file;
}

FILE* Logger::FindFile(const CHAR* name)
{
	if ( ! msInit  ) return NULL;
	
	CS_LOCK
	static TCHAR nameTemp[128] = {0, };	
	StringUtil::CopyAnsi2TCHAR(nameTemp, name);
	CS_UNLOCK

	return FindFile(nameTemp);
}

LPTSTR Logger::GetLastErrorMsg(const TCHAR* userMsg, bool bMsgBox/*=false*/)
{
	if ( ! msInit  ) return NULL;
	
	CS_LOCK

		static TCHAR totalMsg[256];	
		TCHAR* errorMsg = NULL;
		const DWORD errorCode = ::GetLastError();

		::FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS, 
			NULL, errorCode, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), (LPWSTR)&errorMsg, 0, NULL );
	
		::_sntprintf((LPTSTR)totalMsg, sizeof(totalMsg) / sizeof(TCHAR), TEXT("%s (Code:%d) %s "), userMsg, errorCode, errorMsg);

		if ( bMsgBox )
			::MessageBox(NULL, (LPCTSTR)totalMsg, TEXT("Error"), MB_OK);

	CS_UNLOCK


	return totalMsg;
}