#pragma once

#include <string>
#include <map>

//#include "ThreadSync.h"


namespace Core
{
	class CriticalSection;

	class Logger// : public ThreadSyncStatic<Logger>
	{
	public :
		static		void			Init(const TCHAR* logDir);
		static		void			Shutdown();

		/*
		static		void			LogError(const LPTSTR log, ...);
		static		void			LogError(const CHAR* log, ...);

		static		void			LogWarning(const LPTSTR log, ...);
		static		void			LogWarning(const CHAR* log, ...);
		*/

		static		void			Log(const LPTSTR category, const LPTSTR log, ...);
		static		void			Log(const CHAR* category, const CHAR* log, ...);

		static		void			LogDebugString(const CHAR* logMsg, ...);

		static		void			LogWithDate(const LPTSTR category, const LPTSTR log, ...);
		static		void			LogWithDate(const CHAR* category, const CHAR* log, ...);

		static		LPTSTR			GetLastErrorMsg(const TCHAR* userMsg, bool bMsgBox=false);
		static		LPSTR			GetLastErrorMsg(const CHAR* userMsg, bool bMsgBox=false);

	private :
		static		FILE*			FindFile(const LPTSTR name);
		static		FILE*			FindFile(const CHAR* name);

	private :

		static		volatile	bool					msInit;
		static		TCHAR								msLogPath[MAX_PATH];
		static		std::map<std::string, FILE*>		msFileMap;
		static		CriticalSection*					mCS;
	};

	

#define		LOG					Logger::Log
#define		LOG_DATE			Logger::LogWithDate

#define		LOG_LASTERROR_T(categName, bMsgBox)	Logger::Log(categName, _T("%s (File : %s , Line : %d)"), Logger::GetLastErrorMsg((TCHAR*)NULL, bMsgBox), _CRT_WIDE(__FILE__), __LINE__)
#define		LOG_LASTERROR_A(categName, bMsgBox)	Logger::Log(categName, "%s (File : %s , Line : %d)", Logger::GetLastErrorMsg((CHAR*)NULL, bMsgBox), _CRT_WIDE(__FILE__), __LINE__)

#define	LOG_ERROR_T(msg)	\
	Logger::LogWithDate(_T("[Error]"), _T("%s (File : %s , Line : %d)"), msg, _CRT_WIDE(__FILE__), __LINE__);	\
	ASSERT(0)
#define	LOG_ERROR_A(msg)	\
	Logger::LogWithDate("[Error]", "%s (File : %s , Line : %d)", msg, __FILE__, __LINE__);	\
	ASSERT(0)

#define	LOG_WARNING_T(msg)	\
	Logger::LogWithDate(_T("[Warning]"), _T("%s (File : %s , Line : %d)"), msg, _CRT_WIDE(__FILE__), __LINE__)
#define	LOG_WARNING_A(msg)	\
	Logger::LogWithDate("[Warning]", "%s (File : %s , Line : %d)", msg, __FILE__, __LINE__)


#ifdef UNICODE
	#define	LOG_LASTERROR		LOG_LASTERROR_T
	#define	LOG_ERROR			LOG_ERROR_T
	#define	LOG_WARNING			LOG_WARNING_T

#else // UNICODE
	#define	LOG_LASTERROR		LOG_LASTERROR_A
	#define	LOG_ERROR			LOG_ERROR_A
	#define	LOG_WARNING			LOG_WARNING_A
#endif

}
