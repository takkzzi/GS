#pragma once

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

		static		void			LogError(const LPTSTR log, ...);
		static		void			LogError(const CHAR* log, ...);

		static		void			LogWarning(const LPTSTR log, ...);
		static		void			LogWarning(const CHAR* log, ...);

		static		void			Log(const LPTSTR category, const LPTSTR log, ...);
		static		void			Log(const CHAR* category, const CHAR* log, ...);

		static		void			LogWithDate(const LPTSTR category, const LPTSTR log, ...);
		static		void			LogWithDate(const CHAR* category, const CHAR* log, ...);

		static		LPTSTR			GetLastErrorMsg(const TCHAR* userMsg, bool bMsgBox=false);

	private :
		static		FILE*			FindFile(const LPTSTR name);
		static		FILE*			FindFile(const CHAR* name);

	private :

		static		volatile	bool					msInit;
		static		TCHAR								msLogPath[MAX_PATH];
		static		std::map<const LPTSTR, FILE*>		msFileMap;
		static		CriticalSection*					mCS;
	};
}
