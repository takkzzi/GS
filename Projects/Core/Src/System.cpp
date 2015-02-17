#include "PCH.h"
#include "System.h"
#include "Macro.h"
#include "Logger.h"


using namespace Core;

bool	System::msbInit = false;
SYSTEM_INFO		System::msSysInfo;
MEMORYSTATUS	System::msMemStatus;


void System::Init() 
{
	if ( msbInit ) return;

	msbInit = true;
	::GetSystemInfo(&msSysInfo);
	::GlobalMemoryStatus(&msMemStatus);

	CheckWindowVersion();
}

void System::CheckWindowVersion() 
{
	HKEY hKey; 

	if( RegOpenKeyEx(HKEY_LOCAL_MACHINE, TEXT("SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion\\") ,0,KEY_QUERY_VALUE,&hKey) != ERROR_SUCCESS )  
	{
		LOG_ERROR(_T("Error! Not NT OS!")); 
		return;
	}

	DWORD charSize = 100;  
	TCHAR ProductName[100];  
		 
	if(RegQueryValueEx (hKey, TEXT("ProductName"), NULL, NULL, (LPBYTE) ProductName, &charSize) != ERROR_SUCCESS)  
	{
		//LOG_ERROR(_T("ERROR* Fail to Load the ProductName"));
	}
	
	RegCloseKey (hKey); 
}

const SYSTEM_INFO* System::GetSystemInfo()
{
	return &msSysInfo;
}

DWORD System::GetProcessorCount() 
{
	return msSysInfo.dwNumberOfProcessors;
}

const MEMORYSTATUS*	System::GetMemoryStatus()
{
	return &msMemStatus;
}

SIZE_T System::GetTotalPhysicalMemory()  // in byte
{
	return msMemStatus.dwTotalPhys;
}

SIZE_T System::GetAvailPhysicalMemory() // in byte
{
	return msMemStatus.dwAvailPhys;
}

DWORD System::GetLastErrorMessage(TCHAR* outStr, INT outStrLen)
{
	DWORD dwLastError = ::GetLastError();
	dwLastError = ::FormatMessage( FORMAT_MESSAGE_FROM_SYSTEM, NULL, dwLastError, 0, outStr, outStrLen, NULL);
	return dwLastError;
}

