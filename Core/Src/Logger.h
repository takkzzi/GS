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

		static		bool			LogError(LPTSTR log, ...);
		static		bool			LogWarning(LPTSTR log, ...);
		static		bool			Log(const LPTSTR category, LPTSTR log, ...);
		static		bool			LogWithDate(const LPTSTR category, LPTSTR log, ...);


		static		TCHAR						msLogPath[MAX_PATH];
		static		std::map<const LPTSTR, FILE*>		msFileMap;
	};
}

#ifdef __cpluscplus
}
#endif // __cpluscplus