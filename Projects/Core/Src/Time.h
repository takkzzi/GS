#pragma once

namespace Core
{
	class Time 
	{
	public :
		static				void			Init();

		static		const	SYSTEMTIME*		GetSystemTime();
		static		const	TCHAR*			GetSystemTimeStr();
		static				double			GetAppTime();
		static				DWORD			GetAppTicks();
		static				double			GetSecFromTicks(DWORD tick);

	protected :
		static		bool					msbInit;
		static		LARGE_INTEGER			msFrequency;
		static		LARGE_INTEGER			msInitCounter;
		static		SYSTEMTIME				msSystemTime;
	};

}