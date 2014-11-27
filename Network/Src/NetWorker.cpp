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

	/*
	virtual DWORD ThreadTick()
	{
		DWORD		cbTransferred;
		Session*	keySession = NULL;
		Overlapped*	overlapped = NULL;

#ifdef    _WIN64
		BOOL ret = ::GetQueuedCompletionStatus(mNetworker->GetIocpHandle(), &cbTransferred, (PULONG_PTR)&keySession, (LPOVERLAPPED*)&overlapped, INFINITE);
#else
		BOOL ret = ::GetQueuedCompletionStatus(mNetworker->GetIocpHandle(), &cbTransferred, (LPDWORD)&keySession, (LPOVERLAPPED*)&overlapped, INFINITE);
#endif
		if ( ! ret) {
			Logger::LogError(_T("IOCPThread GetQueuedCompletionStatus() Error (ErrorCode:%l)"), GetLastError());
			return 1;	//Keep Calling This Function
		}

		if ( ! keySession ) {
			//End() 가 호출되어 PostQueuedCompletionStatus() 가 호출된 경우.
			Logger::Log(_T("IOCPThread"), _T("GetQueuedCompletionStatus() Break;"));
			return 0;	// End Thread
		}

		if ( overlapped->iotype == IO_SEND ) {
			keySession->OnSendComplete(cbTransferred);
		}
		else if ( overlapped->iotype == IO_RECV ) {
			keySession->OnRecvComplete(cbTransferred);
		}

		return 1;	//Keep Calling This Function
	}
	*/

	virtual DWORD ThreadTick()
	{
		DWORD		cbTransferred;
		IOKey*		ioKey = NULL;
		Overlapped*	overlapped = NULL;

		BOOL ret = ::GetQueuedCompletionStatus(mNetworker->GetIocpHandle(), &cbTransferred, (PULONG_PTR)&ioKey, (LPOVERLAPPED*)&overlapped, INFINITE);

		if ( ! ret) {
			Logger::LogError(_T("IOCPThread GetQueuedCompletionStatus() Error (ErrorCode:%l)"), GetLastError());
			return 1;	//Keep Calling This Function
		}

		if ( ! ioKey ) {
			//End() 가 호출되어 PostQueuedCompletionStatus() 가 호출된 경우.
			Logger::Log(_T("IOCPThread"), _T("ThreadTick() return 0;"));
			return 0;	// End Thread
		}


		if ( ioKey->mType == IOKey_Listener ) {
			Listener* listen = (Listener*)(ioKey);
			listen->OnAccept();
		}
		else if ( ioKey->mType == IOKey_Session ) {
			Session* sess = (Session*)(ioKey);
			if ( overlapped->iotype == IO_SEND ) {
				sess->OnSendComplete(cbTransferred);
			}
			else if ( overlapped->iotype == IO_RECV ) {
				sess->OnRecvComplete(cbTransferred);
			}
		}

		return 1;	//Keep Calling This Function

	}
	virtual bool End() 
	{
		//Wakeup & Return Thrad
		PostQueuedCompletionStatus(mNetworker->GetIocpHandle(), 0, NULL, NULL);

		//__super::End();
		return true;
	}

	virtual void OnEnd(bool bTerminated=false) 
	{
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

	Logger::GetLastErrorMsg(_T("IOCP Creation Fail."), 64);

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

void Networker::BeginListen(UINT16 port)
{
	EndListen();

	mListener = new Listener(this, port);
}

void Networker::EndListen()
{
	SAFE_DELETE(mListener);
}

Session* Networker::GetNewSession()
{
	Session* newSession = NULL;

	for(int i = 0, n = mSessionVec.size(); i < n; ++i) {
		Session* s = mSessionVec[i];
		if ( s == NULL ) {
			s = new Session(this, i, mSendBufferSize, mRecvBufferSize);
			mSessionVec[i] = s;
			newSession = s;
			break;
		}
		else {
			if ( s->IsState(SESSIONSTATE_NONE) ) {
				newSession = s;
				break;
			}
		}
	}

	if ( ! newSession ) {
		newSession = new Session(this, mSessionVec.size(), mSendBufferSize, mRecvBufferSize);
		mSessionVec.push_back(newSession);
	}

	return newSession;
}

void Networker::DeleteAllSessions()
{
	for(auto &i : mSessionVec) {
		SAFE_DELETE(i);
	}
	mSessionVec.clear();
}

//NOTE: Call in main thread.
void Networker::Update()
{
	for(auto &i : mSessionVec) {
		Session* session = i;
		if ( session ) {
			//TODO : Send

			//TODO : Processing Received Packet
		}
	}
}

Session* Networker::GetSession(int id)			
{ 
	ASSERT(0 <= id && id < GetSessionCount() );
	return mSessionVec[id]; 
}