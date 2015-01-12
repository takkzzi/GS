#pragma once

namespace Core
{

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


template <class T>
class ThreadSyncStatic
{
public :
	class ThreadSync
	{
	public:

		ThreadSync() {
			T::msCS.Enter();
		}

		~ThreadSync() {
			T::msCS.Leave();
		}
	};

protected :
	static CriticalSection	msCS;
};

template <class T>
CriticalSection ThreadSyncStatic<T>::msCS;

}