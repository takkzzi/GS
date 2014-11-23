#include "PCH.h"
#include "IOCP.h"
#include "Listener.h"
#include "Session.h"


using namespace Core;
using namespace Network;


class IOCPThread : public Thread
{
public :

	IOCPThread(IOCP* iocp)
		: mIocp(iocp)
	{
	}

	virtual DWORD ThreadTick()
	{
		DWORD		cbTransferred;
		Session*	keySession = NULL;
		Overlapped*	overlapped = NULL;

#ifdef    _WIN64
		BOOL ret = ::GetQueuedCompletionStatus(mIocp->GetIocpHandle(), &cbTransferred, (PULONG_PTR)&keySession, (LPOVERLAPPED*)&overlapped, INFINITE);
#else
		BOOL ret = ::GetQueuedCompletionStatus(mIocp->GetIocpHandle(), &cbTransferred, (LPDWORD)&keySession, (LPOVERLAPPED*)&overlapped, INFINITE);
#endif
		if ( ! ret) {
			Logger::LogWarning(_T("GetQueuedCompletionStatus Error.- %d"), GetLastError());
			return 1;	//Keep Going Thread
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

		return 1;	//Keep Going Thread
	}

	virtual bool End() 
	{
		//Wakeup & Return Thrad
		PostQueuedCompletionStatus(mIocp->GetIocpHandle(), 0, NULL, NULL);

		//__super::End();
		return true;
	}

	virtual void OnEnd(bool bTerminated=false) 
	{
		__super::OnEnd(bTerminated);
	}

protected:
	IOCP*		mIocp;

};


IOCP::IOCP(int threadCount, int reserveSessionCount, int sessionLimitCount, int sendBufferSize, int recvBufferSize)
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
		mSessionVec.push_back(new Session(i, mSendBufferSize, mRecvBufferSize));
	}

	BeginIo(threadCount);
}

IOCP::~IOCP(void)
{
	EndListen();
	EndIo();

	if ( mIocp != INVALID_HANDLE_VALUE) {
		CloseHandle(mIocp);
		mIocp = INVALID_HANDLE_VALUE;
	}
}

//Thread Creation & Start Thread
void IOCP::BeginIo(int threadCount)
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

void IOCP::EndIo()
{
	for(auto &i : mThreadVec) {
		i->End();
		SAFE_DELETE(i);
	}
	mThreadVec.clear();

	DeleteAllSessions();
}

void IOCP::BeginListen(UINT16 port)
{
	EndListen();

	mListener = new Listener(this, port);
	bool bListen = mListener->Begin();
	//ASSERT(bListen);
}

void IOCP::EndListen()
{
	if ( mListener ) {
		mListener->End();
		SAFE_DELETE(mListener);
	}
}

Session* IOCP::GetNewSession()
{
	Session* newSession = NULL;

	for(int i = 0, n = mSessionVec.size(); i < n; ++i) {
		Session* s = mSessionVec[i];
		if ( s == NULL ) {
			s = new Session(i, mSendBufferSize, mRecvBufferSize);
			mSessionVec[i] = s;
			newSession = s;
			break;
		}
		else {
			if ( s->IsState(SESSION_NONE) ) {
				newSession = s;
				break;
			}
		}
	}

	if ( ! newSession ) {
		newSession = new Session(mSessionVec.size(), mSendBufferSize, mRecvBufferSize);
		mSessionVec.push_back(newSession);
	}

	return newSession;
}

void IOCP::DeleteAllSessions()
{
	for(auto &i : mSessionVec) {
		SAFE_DELETE(i);
	}
	mSessionVec.clear();
}

//NOTE: Call in main thread.
void IOCP::Update()
{
	for(auto &i : mSessionVec) {
		Session* session = i;
		if ( session ) {
			//TODO : Send

			//TODO : Processing Received Packet
		}
	}
}

Session* IOCP::GetSession(int id)			
{ 
	ASSERT(0 <= id && id < GetSessionCount() );
	return mSessionVec[id]; 
}