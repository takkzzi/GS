#pragma once

#ifdef __cpluscplus
extern "C" {
#endif // __cpluscplus

namespace Core 
{
	class System 
	{
	public :
		static				void			Init();

		//System Info
		static				void			CheckWindowVersion();
		static		const	SYSTEM_INFO*	GetSystemInfo();
		static				DWORD			GetProcessorCount();
		static		const	MEMORYSTATUS*	GetMemoryStatus();
		static				SIZE_T			GetTotalPhysicalMemory();		// in byte
		static				SIZE_T			GetAvailPhysicalMemory();
		static				DWORD			GetLastErrorMessage(TCHAR* outStr, INT outStrLen);

	protected :
		static		bool					msbInit;
		static		SYSTEM_INFO				msSysInfo;
		static		MEMORYSTATUS			msMemStatus;

	};


	class Time 
	{
	public :
		static				void			Init();

		static				double			GetAppTime();
		static				DWORD			GetAppTicks();
		static				double			GetSecFromTicks(DWORD tick);

	protected :
		static		bool					msbInit;
		static		LARGE_INTEGER			msFrequency;
		static		LARGE_INTEGER			msInitCounter;
	};

}

#ifdef __cpluscplus
}
#endif // __cpluscplus