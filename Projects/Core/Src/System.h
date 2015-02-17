#pragma once


namespace Core 
{
	class System 
	{
	public :
		static				void			Init();

		static				void			CheckWindowVersion();
		static		const	SYSTEM_INFO*	GetSystemInfo();
		static				DWORD			GetProcessorCount();
		static		const	MEMORYSTATUS*	GetMemoryStatus();
		static				SIZE_T			GetTotalPhysicalMemory();		// in byte
		static				SIZE_T			GetAvailPhysicalMemory();
		static				DWORD			GetLastErrorMessage(TCHAR* outStr, INT outStrLen);

	protected :

	protected :
		static		bool					msbInit;
		static		SYSTEM_INFO				msSysInfo;
		static		MEMORYSTATUS			msMemStatus;

	};

}
