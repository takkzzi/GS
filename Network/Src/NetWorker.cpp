#include "PCH.h"
#include "Networker.h"
#include "Listener.h"
#include "Session.h"


using namespace Core;
using namespace Network;


class IOCPThread : public Thread
{
public :

	IOCPThread(Networker* networker)
		: mNetworker(networker)
	{
	}

	virtual DWORD ThreadTick()
	{
		DWORD		cbTransferred;
		void*		ioKey		= NULL;
		Overlapped*	overlapped	= NULL;

		BOOL result = ::GetQueuedCompletionStatus(mNetworker->GetIocpHandle(), &cbTransferred, (PULONG_PTR)&ioKey, (LPOVERLAPPED*)&overlapped, INFINITE);

		if ( result && ! ioKey && ! overlapped )	//End() 의 PostQueuedCompletionStatus() 가 호출된 경우.
			return 0;	// End Thread

		if ( ioKey && overlapped ) 
		{
			Session* sess = (Session*)overlapped->owner;
			ASSERT(overlapped->owner);
			sess->OnCompletionStatus(overlapped, cbTransferred);
		}

		return 1;	//Keep Calling This Function
	}
	
	virtual bool End() 
	{
		//Wakeup & Return Thrad
		PostQueuedCompletionStatus(mNetworker->GetIocpHandle(), 0, NULL, NULL);
		return __super::End();
	}

	virtual void OnEnd(bool bTerminated=false) 
	{
		Logger::LogWarning(_T("IOCPThread %s"), _T("OnEnd()"));
		__super::OnEnd(bTerminated);
	}

protected:
	Networker*		mNetworker;

};


Networker::Networker(int threadCount, int reserveSessionCount, int sessionLimitCount, int sendBufferSize, int recvBufferSize)
	: mIocp(INVALID_HANDLE_VALUE)
	, mListener(NULL)
	, mSessionLimitCount(sessionLimitCount)
	, mSendBufferSize(sendBufferSize)
	, mRecvBufferSize(recvBufferSize)
{
	mIocp = ::CreateIoCompletionPort(INVALID_HANDLE_VALUE, NULL, 0, 0);
	if (mIocp == NULL) {
		Logger::LogError(_T("IOCP Creation Fail."));
	}

	mSessionVec.reserve(reserveSessionCount);

	int i = 0;
	for(int i = 0; i < reserveSessionCount; ++i) {
		mSessionVec.push_back(new Session(this, i, mSendBufferSize, mRecvBufferSize));
	}

	BeginIo(threadCount);
}

Networker::~Networker(void)
{
	EndListen();
	EndIo();

	if ( mIocp != INVALID_HANDLE_VALUE) {
		CloseHandle(mIocp);
		mIocp = INVALID_HANDLE_VALUE;
	}
}

//Thread Creation & Start Thread
void Networker::BeginIo(int threadCount)
{
	mThreadVec.reserve(threadCount);

	if (threadCount <= 0 ) {
		DWORD processors = System::GetProcessorCount();
		threadCount = processors * 2 + 1; //optimized count for processors
	}

	for(int i = 0; i < threadCount; ++i) {
		IOCPThread* th = new IOCPThread(this);
		mThreadVec.push_back(th);
		th->Begin();
	}
}

void Networker::EndIo()
{
	for(auto &i : mThreadVec) {
		i->End();
		SAFE_DELETE(i);
	}
	mThreadVec.clear();

	DeleteAllSessions();
}

void Networker::BeginListen(UINT16 port, bool bPreAccept)
{
	EndListen();

	mbPreAccept = bPreAccept;

	if ( bPreAccept ) {
		mListener = new IocpListener(this, port);
		mListener->BeginListen();
		PreacceptAll();
	}
	else {
		mListener = new SelectListener(this, port);
		mListener->BeginListen();
	}
}

void Networker::EndListen()
{
	if ( mListener )
		mListener->EndListen();
	SAFE_DELETE(mListener);
}


Session* Networker::GetNewSession()
{
	mCriticalSec.Enter();

	Session* newSession = NULL;

	for(int i = 0, n = mSessionVec.size(); i < n; ++i) {
		Session* s = mSessionVec[i];
		if ( s == NULL ) {
			s = new Session(this, i, mSendBufferSize, mRecvBufferSize);
			if ( mbPreAccept )
				s->Accept(mListener->GetSocket());

			mSessionVec[i] = s;
			newSession = s;
			break;
		}
		else if ( s->IsState(SESSIONSTATE_NONE) ) {
			newSession = s;
			break;
		}
	}

	if ( ! newSession ) {
		newSession = new Session(this, mSessionVec.size(), mSendBufferSize, mRecvBufferSize);
		if ( mbPreAccept ) {
			newSession->Accept(mListener->GetSocket());
		}

		mSessionVec.push_back(newSession);
	}

	mCriticalSec.Leave();

	return newSession;
}

void Networker::PreacceptAll()
{
	mCriticalSec.Enter();

	for(unsigned int i = 0; i < mSessionVec.size(); ++i) {
		mSessionVec[i]->Accept(mListener->GetSocket());
	}

	mCriticalSec.Leave();
}

void Networker::DeleteAllSessions()
{
	mCriticalSec.Enter();

	for(auto &i : mSessionVec) {
		SAFE_DELETE(i);
	}
	mSessionVec.clear();

	mCriticalSec.Leave();
}

//NOTE: Call in main thread.
void Networker::Update()
{
	mCriticalSec.Enter();

	for(auto &i : mSessionVec) {
		Session* session = i;
		if ( session ) {
			//TODO : Send

			//TODO : Processing Received Packet
		}
	}

	mCriticalSec.Leave();
}

Session* Networker::GetSession(int id)			
{ 
	ASSERT(0 <= id && id < GetSessionCount() );
	return mSessionVec[id]; 
}