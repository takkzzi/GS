#pragma once

#include <map>

#ifdef __cpluscplus
extern "C" {
#endif // __cpluscplus

namespace Core
{
	class Logger
	{
	public :
		static		void			Init();
		static		void			Shutdown();

		static		void			LogError(const LPTSTR log, ...);
		static		void			LogError(const CHAR* log, ...);

		static		void			LogWarning(const LPTSTR log, ...);
		static		void			LogWarning(const CHAR* log, ...);

		static		void			Log(const LPTSTR category, const LPTSTR log, ...);
		static		void			Log(const CHAR* category, const CHAR* log, ...);

		static		void			LogWithDate(const LPTSTR category, const LPTSTR log, ...);
		static		void			LogWithDate(const CHAR* category, const CHAR* log, ...);

		static		FILE*			FindFile(const LPTSTR name);
		static		FILE*			FindFile(const CHAR* name);

		static		TCHAR								msLogPath[MAX_PATH];
		static		std::map<const LPTSTR, FILE*>		msFileMap;
	};
}

#ifdef __cpluscplus
}
#endif // __cpluscplus
