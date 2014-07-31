#include "PCH.h"
#include "System.h"
#include "Macro.h"


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
}

const SYSTEM_INFO* System::GetSystemInfo()
{
	//Debug::Assert(msbInit);
	return &msSysInfo;
}

DWORD System::GetProcessorCount() {
	//Debug::Assert(msbInit);
	return msSysInfo.dwNumberOfProcessors;
}

const MEMORYSTATUS*	System::GetMemoryStatus()
{
	//Debug::Assert(msbInit);
	return &msMemStatus;
}

DWORD System::GetTotalPhysicalMemory()  // in byte
{
	//Debug::Assert(msbInit);
	return msMemStatus.dwTotalPhys;
}

DWORD System::GetAvailPhysicalMemory() // in byte
{
	//Debug::Assert(msbInit);
	return msMemStatus.dwAvailPhys;
}

DWORD System::GetLastErrorMessage(TCHAR* outStr, INT outStrLen)
{
	DWORD dwLastError = ::GetLastError();
	dwLastError = ::FormatMessage( FORMAT_MESSAGE_FROM_SYSTEM, NULL, dwLastError, 0, outStr, outStrLen, NULL);
	return dwLastError;
}



bool			Time::msbInit = false;
LARGE_INTEGER	Time::msFrequency;
LARGE_INTEGER	Time::msInitCounter;


void Time::Init() 
{
	if ( msbInit ) return;

	QueryPerformanceFrequency(&msFrequency);
	QueryPerformanceCounter(&msInitCounter);
	msbInit = true;
}

//Elpased time since Time::Init
double Time::GetAppTime()
{
	//Debug::Assert(msbInit);

	LARGE_INTEGER counter;
    QueryPerformanceCounter(&counter);
    return (double)( (long double)(counter.QuadPart - msInitCounter.QuadPart) / (long double)msFrequency.QuadPart);
}

DWORD Time::GetAppTicks()
{
	//Debug::Assert(msbInit);

	LARGE_INTEGER counter;
    QueryPerformanceCounter(&counter);

	return (DWORD)counter.QuadPart;
}

double Time::GetSecFromTicks(DWORD ticks)
{
	ASSERT(msbInit);
	return ((double)(ticks / msFrequency.QuadPart) * 1000.0);
}
