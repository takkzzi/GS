#include "pch.h"
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
	DWORD res = th->Run();
	if ( res == 0 ) {
		th->OnEnd();
	}

	return res;
}

Thread::Thread() 
	: mhThread(INVALID_HANDLE_VALUE)
	//, mhEndEvent(INVALID_HANDLE_VALUE)
	, mState(THREAD_NONE)
	, mTermWaitTime(500)
{	
}

Thread::~Thread()
{
	if ( mState != THREAD_NONE )
		End(true);
}

bool Thread::Begin(bool bSuspend) 
{
	if ( mhThread != INVALID_HANDLE_VALUE )
		return FALSE;
	
	DWORD id;
	mhThread = ::CreateThread(NULL, 0, Thread::ThreadRunner, this, bSuspend ? CREATE_SUSPENDED : 0, &id);
	//::CloseHandle( mhThread );	//이후 mhThread 핸들 사용 불가

	if ( mhThread == NULL ) 
	{	
		DWORD err = GetLastError();
		Logger::Log(ThreadLog, _T("Thread::Begin Error : %d"), err);
		return FALSE;
	}

	mState = bSuspend ? THREAD_SUSPEND : THREAD_RUNNING;

	//mhEndEvent = ::CreateEvent(NULL, FALSE, FALSE, NULL);
	return TRUE;
}

void Thread::End(bool bForceTerminate) 
{
	if ( IsState(THREAD_END) )
		return;

	if ( IsState(THREAD_SUSPEND) ) {
		Resume();
	}

	mState = THREAD_END;

	DWORD waitTime = bForceTerminate ? mTermWaitTime : INFINITE;
	DWORD waitRes = ::WaitForSingleObject(mhThread, waitTime);		//Waiting For Thread Return.
	BOOL bEnded = (waitRes == WAIT_OBJECT_0);

	if ( ! bEnded) {
		if ( TerminateThread(mhThread, 0) ) {
			Logger::Log(ThreadLog, TEXT("TerminateThread! Handle : %d"), mhThread);
			bEnded = true;
			this->OnEnd(true);
		}
	}

	if ( bEnded ) {
		if ( ::CloseHandle(mhThread) )
			mhThread = INVALID_HANDLE_VALUE;
	}
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
		Logger::Log(ThreadLog, _T("Thread::Resume Error : %d"), GetLastError());
		return FALSE;
	}

	if ( prevSusCount == 0 )
		mState = THREAD_RUNNING;

	return TRUE;
}

void Thread::OnEnd(bool bTerminated) 
{
	if ( bTerminated )
		Logger::Log(ThreadLog, TEXT("OnEnd Terminated (Thread Handle:%d)"), mhThread);

	mState = THREAD_NONE;
}

DWORD Thread::Run() 
{
	//상속받는 클래스의 Run 함수 내에서 아래와 같이
	//State 를 검사하여 Thread 함수를 종료하자.
	while(1) {
		if ( IsState(THREAD_END) ) 
			break;
	}

	return 0;
}
