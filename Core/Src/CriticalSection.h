#pragma once

class CriticalSection 
{
public :
	CriticalSection() 
	{
		::InitializeCriticalSection(&mCS);
	}

	~CriticalSection()
	{
		::DeleteCriticalSection(&mCS);
	}

	void Enter()
	{
		::EnterCriticalSection(&mCS);
	}

	void Leave()
	{
		::LeaveCriticalSection(&mCS);
	}

private :
	CRITICAL_SECTION	mCS;
};
