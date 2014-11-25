#include "PCH.h"
#include "Thread.h"
#include "Logger.h"


using namespace Core;
using namespace std;

#define		ThreadLog			_T("ThreadLog")

DWORD CALLBACK Thread::ThreadRunner(LPVOID param)
{
	if ( ! param)
		return -1;

	Thread* th = (Thread*)param;
	while(true) {
		if ( th->IsState(THREAD_END) )
			break;

		if ( th->ThreadTick() == 0 )
			break;
	}

	th->OnEnd(false);

	return 0;
}

Thread::Thread() 
	: mhThread(INVALID_HANDLE_VALUE)
	, mThreadId(0)
	, mState(THREAD_NONE)
	, mTermWaitTime(1000)
{	
}

Thread::~Thread()
{
	if ( mState != THREAD_NONE ) {
		//bool ended = End();
		//if ( ! ended ) {
			Termainate();
		//}
	}
}

bool Thread::Begin(bool bSuspend) 
{
	if ( mhThread != INVALID_HANDLE_VALUE )
		return FALSE;

	if ( ! IsState(THREAD_NONE) )
		return FALSE;

	mhThread = ::CreateThread(NULL, 0, Thread::ThreadRunner, this, bSuspend ? CREATE_SUSPENDED : 0, &mThreadId);
	//::CloseHandle( mhThread );	//이후 mhThread 핸들 사용 불가

	if ( mhThread == NULL ) 
		return FALSE;

	mState = bSuspend ? THREAD_SUSPEND : THREAD_RUNNING;

	return TRUE;
}

bool Thread::End() 
{
	if ( IsState(THREAD_END) )
		return false;

	if ( IsState(THREAD_SUSPEND) )
		Resume();

	mState = THREAD_END;

	DWORD waitRes = ::WaitForSingleObject(mhThread, mTermWaitTime);		//Waiting For Thread Return.
	bool bEnded = (waitRes == WAIT_OBJECT_0);

	return bEnded;
}

bool Thread::Suspend() 
{
	if ( mState != THREAD_RUNNING )
		return FALSE;

	DWORD prevSusCount = ::SuspendThread(mhThread);
	if ( prevSusCount >= 0 ) {
		mState = THREAD_SUSPEND;
		return TRUE;
	}
	else {
		Logger::Log(ThreadLog, _T("Thread::Suspend Error : %d"), GetLastError());
	}

	return FALSE;
}

bool Thread::Resume() 
{
	if ( mState != THREAD_SUSPEND )
		return FALSE;

	DWORD prevSusCount = MAXIMUM_SUSPEND_COUNT;
	while ( prevSusCount > 1 ) {
		prevSusCount = ::ResumeThread(mhThread);
	}

	if ( prevSusCount < 0 ) {
		ASSERT((prevSusCount >= 0) && "Thread::Resume Error");
		return FALSE;
	}

	if ( prevSusCount <= 1 )
		mState = THREAD_RUNNING;

	return TRUE;
}

bool Thread::Termainate() 
{
	if ( ! IsState(THREAD_RUNNING) )
		return false;

	if ( TerminateThread(mhThread, 0) ) {
		//Logger::Log(ThreadLog, TEXT("TerminateThread! Handle : %d"), mhThread);
		OnEnd(true);
		return true;
	}

	return false;
}

void Thread::OnEnd(bool bTerminated) 
{
	if ( ! IsState(THREAD_END) )
		return;

	BOOL bClose = CloseHandle(mhThread);
	ASSERT(bClose);
	mhThread = INVALID_HANDLE_VALUE;
	mState = THREAD_NONE;
}
