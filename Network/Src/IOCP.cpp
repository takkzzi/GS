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

	virtual DWORD Run()
	{
		DWORD listeningTimeout = 1000;
		DWORD cbTransferred;
		Session* keySession;
		Overlapped* overlapped = NULL;

		while (1)
		{
			if ( IsState(THREAD_END) )
				break;

			BOOL ret = ::GetQueuedCompletionStatus(mIocp->GetIocpHandle(), &cbTransferred, (LPDWORD)&keySession, (LPOVERLAPPED*)&overlapped, INFINITE);
			if ( ! ret) {
				Logger::LogWarning(_T("GetQueuedCompletionStatus Error.- &d"), GetLastError());
				continue;
			}

			if ( keySession ) {
				if (cbTransferred == 0) {
					keySession->Disconnect();
					continue;
				}
				else if ( overlapped->iotype == IO_SEND ) {
					keySession->OnSendComplete(cbTransferred);
				}
				else if ( overlapped->iotype == IO_RECV ) {
					keySession->OnRecvComplete(cbTransferred);
				}
			}
			else {
				//End() 가 호출되어 PostQueuedCompletionStatus() 가 호출된 경우.
				Logger::Log(_T("IOCPThread"), _T("GetQueuedCompletionStatus() Break;"));
				break;
			}
		}
		return 0;
	}

	virtual void End(bool bForceTerminate=false) 
	{
		//Wakeup & Return Thrad
		PostQueuedCompletionStatus(mIocp->GetIocpHandle(), 0, NULL, NULL);

		__super::End();
	}

	virtual void OnEnd(bool bTerminated=false) 
	{
		__super::OnEnd(bTerminated);
	}

protected:
	IOCP*		mIocp;

};


IOCP::IOCP(int threadCount, int sessionCount, bool bPrecacheSession, int sendBufferSize, int recvBufferSize)
	: mIocp(INVALID_HANDLE_VALUE)
	, mListener(NULL)
	, mSessionCount(sessionCount)
	, mSendBufferSize(sendBufferSize)
	, mRecvBufferSize(recvBufferSize)
{
	mIocp = ::CreateIoCompletionPort(INVALID_HANDLE_VALUE, NULL, 0, 0);
	if (mIocp == NULL) {
		Logger::LogError(_T("IOCP Creation Fail."));
	}

	mSessionVec.reserve(mSessionCount);

	int i = 0;
	for(int i = 0; i < sessionCount; ++i) {
		mSessionVec.push_back(bPrecacheSession ? (new Session(i, mSendBufferSize, mRecvBufferSize)) : NULL);
		mSessionMap[i] = mSessionVec[i];
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

	for(int i = 0; i < threadCount; ++i) {
		IOCPThread* th = new IOCPThread(this);
		mThreadVec.push_back(th);
		th->Begin();
	}
}

void IOCP::EndIo()
{
	for(auto &i : mThreadVec) {
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
	for(int i = 0, n = mSessionVec.size(); i < n; ++i) {
		Session* s = mSessionVec[i];
		if ( s == NULL ) {
			s = new Session(i, mSendBufferSize, mRecvBufferSize);
			mSessionVec[i] = s;
			mSessionMap[i] = s;
			return s;
		}
		else {
			if ( s->IsState(SESSION_NONE) ) {
				return s;
			}
		}
	}

	return NULL;
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
	return mSessionVec[id]; 
}