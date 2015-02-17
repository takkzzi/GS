#include "PCH.h"
#include "Time.h"

using namespace Core;


bool			Time::msbInit = false;
LARGE_INTEGER	Time::msFrequency;
LARGE_INTEGER	Time::msInitCounter;
SYSTEMTIME		Time::msSystemTime;

void Time::Init() 
{
	if ( msbInit ) return;

	QueryPerformanceFrequency(&msFrequency);
	QueryPerformanceCounter(&msInitCounter);
	msbInit = true;
}

const SYSTEMTIME* Time::GetSystemTime() {
	GetLocalTime(&msSystemTime);
	return &msSystemTime;
}

const TCHAR* Time::GetSystemTimeStr() {
	static TCHAR strBuff[32];
	GetSystemTime();
	_sntprintf(strBuff, 32, _T("%d-%d-%d %d:%d:%d"), msSystemTime.wYear, msSystemTime.wMonth, msSystemTime.wDay, msSystemTime.wHour, msSystemTime.wMinute, msSystemTime.wSecond);
	return strBuff;
}

//Elpased time since Time::Init
double Time::GetAppTime()
{
	LARGE_INTEGER counter;
    QueryPerformanceCounter(&counter);
    return (double)( (long double)(counter.QuadPart - msInitCounter.QuadPart) / (long double)msFrequency.QuadPart);
}

DWORD Time::GetAppTicks()
{
	LARGE_INTEGER counter;
    QueryPerformanceCounter(&counter);

	return (DWORD)counter.QuadPart;
}

double Time::GetSecFromTicks(DWORD ticks)
{
	return ((double)(ticks / msFrequency.QuadPart) * 1000.0);
}

