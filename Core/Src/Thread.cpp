#include "pch.h"
#include "Thread.h"

using namespace Core;
using namespace std;

#define		DebugLog	OutputDebugString


DWORD	Thread::msTermWaitTime		=	500;

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
	, mhEndEvent(INVALID_HANDLE_VALUE)
	, mState(STATE_NONE)
{	
}

Thread::~Thread()
{
	if ( mState == STATE_NONE )
		return;

	End(true);
}

TCHAR* Thread::GetErrorString(DWORD errorCode) 
{
	switch(errorCode) {
	case WAIT_OBJECT_0 :
		return TEXT("WAIT_OBJECT_0");
	case WAIT_ABANDONED : 
		return TEXT("WAIT_ABANDONED");
	case WAIT_TIMEOUT : 
		return TEXT("WAIT_TIMEOUT");
	case WAIT_FAILED : 
		return TEXT("WAIT_FAILED");
	}


	return NULL;
}

void Thread::Begin(bool bSuspend) 
{
	if ( mhThread != INVALID_HANDLE_VALUE )
		return;
	
	DWORD id;
	mhThread = ::CreateThread(NULL, 0, Thread::ThreadRunner, this, bSuspend ? CREATE_SUSPENDED : 0, &id);
	//::CloseHandle( mhThread );	//이후 mhThread 핸들 사용 불가

	if ( mhThread == NULL ) 
	{	
		DWORD error = GetLastError();
		std::wstring	str;
		str = TEXT("test");

		return;
	}

	mhEndEvent = ::CreateEvent(NULL, FALSE, FALSE, NULL);
}

void Thread::End(bool bForceTerminate) 
{
	//상속받는 클래스의 Run 함수 내에서 
	//mhEndEvent 이벤트를 감지하여 Thread 함수를 종료하자.
	if ( ! ::SetEvent(mhEndEvent) ) {
		bForceTerminate = true;
	}

	mState = STATE_ENDING;

	DWORD waitTime = bForceTerminate ? msTermWaitTime : INFINITE;
	DWORD waitRes = ::WaitForSingleObject(mhThread, waitTime);
	BOOL bEnded = (waitRes == WAIT_OBJECT_0);

	if ( ! bEnded) {
		DebugLog(GetErrorString(waitRes));
		if ( TerminateThread(mhThread, 0) ) {
			DebugLog(TEXT("TerminateThread"));
			bEnded = true;
			this->OnEnd(true);
		}
	}

	if ( bEnded ) {
		if ( ::CloseHandle(mhEndEvent) )
			mhEndEvent = INVALID_HANDLE_VALUE;
		if ( ::CloseHandle(mhThread) )
			mhThread = INVALID_HANDLE_VALUE;

		mState = STATE_END;
	}
}

bool Thread::Resume() 
{
	return TRUE;
}

void Thread::OnEnd(bool bTerminated) 
{
	if ( bTerminated )
		DebugLog(TEXT("OnEnd Terminated"));
	else
		DebugLog(TEXT("OnEnd Success"));
}

DWORD Thread::Run() 
{
	while(1) {
		if ( ::WaitForSingleObject(mhEndEvent, 1) == WAIT_OBJECT_0 ) {
			break;
		}

		Sleep(1000);
		DebugLog(TEXT("Running\n"));
	}

	return 0;
}
