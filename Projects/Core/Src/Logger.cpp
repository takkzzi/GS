#include "PCH.h"
#include <stdarg.h>
#include <strsafe.h>

#include "Logger.h"
#include "StrUtil.h"
#include "System.h"
#include "Time.h"
#include "ThreadSync.h"


#define				MAX_LOG_BUFFER		2048
#define				CS_LOCK			mCS->Enter();
#define				CS_UNLOCK		mCS->Leave();

using namespace Core;
using namespace std;

volatile	bool				Logger::msInit = false;
TCHAR							Logger::msLogPath[MAX_PATH];
std::map<std::string, FILE*>	Logger::msFileMap;
CriticalSection*				Logger::mCS = NULL;


void Logger::Init(const TCHAR* logDir)
{
	GetCurrentDirectory(MAX_PATH, msLogPath);

	TCHAR timeStr[32];
	const SYSTEMTIME* sysTime = Time::GetSystemTime();
	_sntprintf(timeStr, 32, _T("%d-%d-%d"), sysTime->wYear, sysTime->wMonth, sysTime->wDay);

	_sntprintf(msLogPath, MAX_PATH, _T("%s\\%s"), msLogPath, logDir);
	CreateDirectory(msLogPath, NULL);

	_sntprintf(msLogPath, MAX_PATH, _T("%s\\%s\\"), msLogPath, timeStr);
	CreateDirectory(msLogPath, NULL);

	mCS = new CriticalSection();

	msInit = true;

	LogDebugString("\n<Logger Initialize.>");
}

void Logger::Shutdown() 
{
	LogDebugString("\n<Logger Shutdown.>");

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

/*
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
*/

void Logger::Log(const LPTSTR category, const LPTSTR logData, ...)
{
	if ( ! msInit  ) return;
	
	FILE* file = FindFile(category);
	if ( ! file ) return;

	CS_LOCK
	{
		static TCHAR logBuff[MAX_LOG_BUFFER]	= {0,};

		va_list		ap;
		va_start(ap, logData);
		_vstprintf(logBuff, MAX_LOG_BUFFER, logData, ap);
		va_end(ap);

		_stprintf_s(logBuff, _T("%s\n"), logBuff);

		//_ftprintf(file, _T("%s\n"), logBuff);
		_ftprintf(file, logBuff);
		fflush(file);

		if ( IsDebuggerPresent() ) {
			OutputDebugString(logBuff);

	#ifdef _CONSOLE
			_tprintf(_T("%s"), logBuff);
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
		static CHAR logBuff[MAX_LOG_BUFFER]				= {0,};

		va_list		ap;
		va_start(ap, logData);
		vsprintf(logBuff, logData, ap);
		va_end(ap);
		
		sprintf(logBuff, "%s\n", logBuff);
		fprintf(file, "%s", logBuff);
		fflush(file);
		
		if ( IsDebuggerPresent() ) {
			OutputDebugStringA(logBuff);

	#ifdef _CONSOLE
			_tprintf("%s", logBuff);
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

		static TCHAR logBuff[MAX_LOG_BUFFER] = {0,};
		static TCHAR resultBuff[MAX_LOG_BUFFER] = { 0, };

		va_list		ap;
		va_start(ap, logData);
		_vstprintf(logBuff, MAX_LOG_BUFFER, logData, ap);
		va_end(ap);

		_stprintf_s(resultBuff, MAX_LOG_BUFFER, _T("[%s] %s\n"), timeStr, logBuff);

		//_ftprintf(file, _T("[%s] %s\n"), timeStr, logBuff);
		_ftprintf(file, resultBuff);
		fflush(file);

		if ( IsDebuggerPresent() ) {
			OutputDebugString(resultBuff);

	#ifdef _CONSOLE
			_tprintf(_T("%s"), logBuff);
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

		static CHAR logBuff[MAX_LOG_BUFFER]	= {0,};
		static CHAR resultBuff[MAX_LOG_BUFFER] = { 0, };

		va_list		ap;
		va_start(ap, logData);
		vsprintf(logBuff, logData, ap);
		va_end(ap);

		sprintf(resultBuff, "[%s] %s\n", currTimeA, logBuff);
		fprintf(file, resultBuff);
		fflush(file);

		//fclose(FilePtr);

		if (IsDebuggerPresent())
		{
			OutputDebugStringA(resultBuff);

	#ifdef _CONSOLE
			printf("%s", debugBuff);
	#endif
		}
	}
	CS_UNLOCK
}

void Logger::LogDebugString(const CHAR* logMsg, ...)
{
	if (!msInit) return;

	CS_LOCK
	{
		static CHAR logBuff[MAX_LOG_BUFFER] = { 0, };

	va_list		ap;
	va_start(ap, logMsg);
	vsprintf(logBuff, logMsg, ap);
	va_end(ap);

	sprintf_s(logBuff, "%s\n", logBuff);
	OutputDebugStringA(logBuff);

#ifdef _CONSOLE
	printf("%s\n", logBuff);
#endif
	}
		CS_UNLOCK
}

void Logger::LogDebugString(const LPTSTR logMsg, ...)
{
	if (!msInit) return;

	CS_LOCK
	{
		static TCHAR logBuff[MAX_LOG_BUFFER] = { 0, };

	va_list		ap;
	va_start(ap, logMsg);
	_vstprintf(logBuff, MAX_LOG_BUFFER, logMsg, ap);
	va_end(ap);

	_stprintf_s(logBuff, _T("%s\n"), logBuff);

	OutputDebugString(logBuff);

#ifdef _CONSOLE
	_tprintf(_T("%s\n"), logBuff);
#endif
	}
		CS_UNLOCK
}

FILE* Logger::FindFile(const LPTSTR name)
{
	if ( ! msInit  ) return NULL;
	
	CS_LOCK
		static CHAR nameTemp[128] = {0, };	
		StringUtil::CopyTCHAR2Ansi(nameTemp, name);
	CS_UNLOCK

	return FindFile(nameTemp);
}

FILE* Logger::FindFile(const CHAR* name)
{
	if ( ! msInit  ) return NULL;

	CS_LOCK
		
	auto itor = msFileMap.find(name);
	if(itor != msFileMap.end()) {
		CS_UNLOCK
		return (FILE*)itor->second;
	}

	static CHAR	fileName[MAX_PATH]		= {0,};
	StringUtil::CopyTCHAR2Ansi(fileName, msLogPath);
	_snprintf(fileName, MAX_PATH, "%s%s.log", fileName, name);

	FILE* file = fopen(fileName, "w+");
	ASSERT(file && "Logger::FindFile() Failed");

	if (! file) {
		CS_UNLOCK
		return NULL;
	}
	msFileMap[name] = file;

	CS_UNLOCK

	return file;
}

LPTSTR Logger::GetLastErrorMsg(const TCHAR* userMsg, bool bMsgBox/*=false*/)
{
	if ( ! msInit  ) return NULL;
	
	CS_LOCK
		static TCHAR totalMsg[256];	
		TCHAR* errorMsg = NULL;
		const DWORD errorCode = ::GetLastError();

		::FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS, 
			NULL, errorCode, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), (LPTSTR)&errorMsg, 0, NULL );
	
		::_sntprintf((LPTSTR)totalMsg, sizeof(totalMsg) / sizeof(TCHAR), _T("%s [Code:%d] %s "), userMsg, errorCode, errorMsg);

		if ( bMsgBox )
			::MessageBox(NULL, (LPCTSTR)totalMsg, TEXT("Error"), MB_OK);
	CS_UNLOCK


	return totalMsg;
}

LPSTR Logger::GetLastErrorMsg(const CHAR* userMsg, bool bMsgBox/*=false*/)
{
	if ( ! msInit  ) return NULL;
	
	CS_LOCK

		static CHAR totalMsg[256];	
		CHAR* errorMsg = NULL;
		const DWORD errorCode = ::GetLastError();

		::FormatMessageA(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS, 
			NULL, errorCode, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), (LPSTR)&errorMsg, 0, NULL );
	
		::_snprintf(totalMsg, sizeof(totalMsg) / sizeof(CHAR), "%s [Code:%d] %s ", userMsg, errorCode, errorMsg);

		if ( bMsgBox )
			::MessageBoxA(NULL, (LPCSTR)totalMsg, "Error", MB_OK);

	CS_UNLOCK

	return totalMsg;
}