#include "PCH.h"
#include "Networker.h"
#include "Listener.h"
#include "Session.h"

#include <process.h>

using namespace Core;
using namespace Network;



unsigned __stdcall IOCPWorker (void* arg)
{
	Networker* networker = (Networker*)arg;
	while(1) 
	{
		DWORD				transferBytes;
		void*				ioKey		= NULL;
		OverlappedIoData*	overlapped	= NULL;

		BOOL result = ::GetQueuedCompletionStatus(networker->GetIocpHandle(), &transferBytes, (PULONG_PTR)&ioKey, (LPOVERLAPPED*)&overlapped, INFINITE);

		//PostQueuedCompletionStatus() 가 호출된 경우.
		if ( ! ioKey && ! overlapped )
			break;

		if ( ioKey && overlapped && overlapped->session ) {

			Session* sess = overlapped->session;
			OverlappedIoType io = overlapped->ioType;
			if ( result ) {
				if ( io == IO_ACCEPT ) {
					sess->OnAccept(INVALID_SOCKET);
				}
				else if ( io == IO_SEND ) {
					sess->OnSendComplete(overlapped->sessBuf, transferBytes);
				}
				else if ( io == IO_RECV ) {
					sess->OnRecvComplete(overlapped->sessBuf, transferBytes);
				}
			}
			else {
				//Socket Closed
				sess->ResetState(true);
			}
		}
	}
	networker->OnEndIoThread();
	return 0;
}




Networker::Networker(int threadCount, int reserveSessionCount, int sessionLimitCount, int sendBufferSize, int recvBufferSize)
	: mIocp(INVALID_HANDLE_VALUE)
	, mThreadCount(threadCount)
	, mListener(NULL)
	, mbPreAccept(false)
	, mSessionLimitCount(sessionLimitCount)
	, mSendBufferSize(sendBufferSize)
	, mRecvBufferSize(recvBufferSize)
{
	if ( mSessionLimitCount <= 0 )
		mSessionLimitCount = 1;

	if ( reserveSessionCount <= 0 )
		reserveSessionCount = 1;

	mSessionVec.reserve(mSessionLimitCount);

	for(int i = 0; i < reserveSessionCount; ++i) {
		mSessionVec.push_back(new Session(this, i, mSendBufferSize, mRecvBufferSize));
	}

	BeginIo();
}

Networker::~Networker(void)
{
	EndListen();
	EndIo();
}

//Thread Creation & Start Thread
void Networker::BeginIo()
{
	mCriticalSec.Enter();
	
	mIocp = ::CreateIoCompletionPort(INVALID_HANDLE_VALUE, NULL, 0, 0);
	if (mIocp == NULL) {
		LOG_ERROR(_T("IOCP Creation Fail."));
	}

	if (mThreadCount <= 0 ) {
		DWORD processors = System::GetProcessorCount();
		mThreadCount = processors * 2 + 1; //optimized count for processors
	}

	mWorkingcCount = mThreadCount;
	for (UINT i = 0; i < mThreadCount; i++)
	{
		HANDLE hThread = (HANDLE) ::_beginthreadex(NULL, 0, IOCPWorker, this, 0, NULL);
		CloseHandle (hThread);
	}
	
	mCriticalSec.Leave();
}

void Networker::EndIo()
{	
	mCriticalSec.Enter();
	for (UINT i = 0; i < mThreadCount; i++) {
		PostQueuedCompletionStatus(GetIocpHandle(), 0, NULL, NULL);
	}

	while ( mWorkingcCount > 0) {
	}

	if ( mIocp != INVALID_HANDLE_VALUE) {
		CloseHandle(mIocp);
		mIocp = INVALID_HANDLE_VALUE;
	}
	mCriticalSec.Leave();

	DeleteAllSessions();
}

void Networker::BeginListen(UINT16 port, bool bPreAccept)
{
	EndListen();

	mCriticalSec.Enter();
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
	mCriticalSec.Leave();
}

void Networker::EndListen()
{
	mCriticalSec.Enter();
	if ( mListener )
		mListener->EndListen();
	SAFE_DELETE(mListener);
	mCriticalSec.Leave();
}

Session* Networker::GetSession(int id)			
{ 
	ASSERT(0 <= id && id < GetSessionCount() );
	return mSessionVec[id]; 
}

Session* Networker::GetNewSession()
{
	mCriticalSec.Enter();

	Session* newSession = NULL;
	for(int i = 0, n = mSessionVec.size(); i < n; ++i) {
		Session* s = mSessionVec[i];
		if ( s == NULL ) {
			s = new Session(this, i, mSendBufferSize, mRecvBufferSize);
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
		newSession = AddSession();
	}

	mCriticalSec.Leave();

	return newSession;
}

Session* Networker::AddSession()
{
	Session* newSess = NULL;
	if ( mSessionVec.size() < (std::size_t)mSessionLimitCount ) {
		newSess = new Session(this, GetSessionCount(), mSendBufferSize, mRecvBufferSize);
		mSessionVec.push_back(newSess);
	}
	return newSess;
}

void Networker::PreacceptAll()
{
	mCriticalSec.Enter();

	for(unsigned int i = 0; i < mSessionVec.size(); ++i) {
		mSessionVec[i]->PreAccept(mListener->GetSocket());
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

void Networker::OnEndIoThread()
{
	::InterlockedDecrement(&mWorkingcCount);
	ASSERT(mWorkingcCount >= 0);
}

void Networker::Update()
{
	mCriticalSec.Enter();
	Session* acceptingSession = NULL;
	for(auto &sess : mSessionVec) {
		if ( sess ) {
			if ( sess->IsState(SESSIONSTATE_CONNECTED) ) {
				sess->Send();		// Send Data Packet One-by-One
			}
		}

		if ( mbPreAccept ) {
			switch (sess->GetState()) {
			case SESSIONSTATE_NONE :
				sess->PreAccept(mListener->GetSocket());
			case SESSIONSTATE_ACCEPTING :
				acceptingSession = sess;
			}
		}
	}
	
	if ( ! acceptingSession && mbPreAccept ) {
		acceptingSession = AddSession();
		if ( acceptingSession)
			acceptingSession->PreAccept(mListener->GetSocket());
	}
	mCriticalSec.Leave();
}

