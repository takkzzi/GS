#include "PCH.h"
#include "Networker.h"
#include "Listener.h"
#include "Session.h"

#include <process.h>


#define		CS_LOCK				mCritiSect.Enter();
#define		CS_UNLOCK			mCritiSect.Leave();

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
					sess->OnSendComplete(overlapped, transferBytes);
				}
				else if ( io == IO_RECV ) {
					sess->OnRecvComplete(overlapped, transferBytes);
				}
			}
			else {
				//Remote Socket Closed
				sess->ResetState(true);
			}
		}
	}
	networker->OnEndIoThread();
	return 0;
}


unsigned __stdcall SessionUpdater (void* arg) 
{
	Networker* networker = (Networker*)arg;

	while (networker->IsThreadUpdatingSessions()) {
		networker->UpdateSessions();
		Sleep(10);
	}

	return 0;
}


Networker::Networker(bool bUseThreadUpdateSession, int ioThreadCount, int sessionReserveCount, int sessionLimitCount, int sendBufferSize, int recvBufferSize)
	: mIocp(INVALID_HANDLE_VALUE)
	, mIoThreadCount(ioThreadCount)
	, mIoWorkingCount(0)
	, mListener(NULL)
	, mbPreAccept(false)
	, mSessionLimitCount(sessionLimitCount)
	, mSendBufferSize(sendBufferSize)
	, mRecvBufferSize(recvBufferSize)
	, mbThreadUpdateSessions(bUseThreadUpdateSession)
	, mSessUpdateThread(INVALID_HANDLE_VALUE)
{
	if ( mSessionLimitCount <= 0 )
		mSessionLimitCount = 1;

	if ( sessionReserveCount <= 0 )
		sessionReserveCount = 1;

	mSessionVec.reserve(mSessionLimitCount);

	for(int i = 0; i < sessionReserveCount; ++i) {
		Session* s = new Session(this, i, mSendBufferSize, mRecvBufferSize);
		s->Init();
		mSessionVec.push_back(s);
	}

	BeginIo();
	if ( mbThreadUpdateSessions )
		BeginSessionUpdate();
}

Networker::~Networker(void)
{
	EndSessionUpdate();
	EndListen();
	EndIo();
	DeleteAllSessions();
}

//Thread Creation & Start Thread
void Networker::BeginIo()
{
	CS_LOCK
	
	mIocp = ::CreateIoCompletionPort(INVALID_HANDLE_VALUE, NULL, 0, 0);
	if (mIocp == NULL) {
		LOG_ERROR(_T("IOCP Creation Fail."));
	}

	if (mIoThreadCount <= 0 ) {
		DWORD processors = System::GetProcessorCount();
		mIoThreadCount = processors * 2 + 1; //optimized count for processors
	}

	mIoWorkingCount = mIoThreadCount;
	for (UINT i = 0; i < mIoThreadCount; i++)
	{
		HANDLE hThread = (HANDLE) ::_beginthreadex(NULL, 0, IOCPWorker, this, 0, NULL);
		CloseHandle (hThread);
	}
	
	CS_UNLOCK
}

void Networker::EndIo()
{	
	CS_LOCK
	for (UINT i = 0; i < mIoThreadCount; i++) {
		PostQueuedCompletionStatus(GetIocpHandle(), 0, NULL, NULL);
	}

	while ( mIoWorkingCount > 0) {
	}

	if ( mIocp != INVALID_HANDLE_VALUE) {
		CloseHandle(mIocp);
		mIocp = INVALID_HANDLE_VALUE;
	}
	CS_UNLOCK
}

void Networker::BeginListen(UINT16 port, bool bPreAccept)
{
	EndListen();

	CS_LOCK
	
	if ( bPreAccept ) {
		mListener = new IocpListener(this, port);
		mListener->BeginListen();
		//StartAcceptAll();
		mbPreAccept = bPreAccept;
	}
	else {
		mListener = new SelectListener(this, port);
		mListener->BeginListen();
	}

	CS_UNLOCK
}

void Networker::EndListen()
{	
	if ( mListener ) {
		CS_LOCK
		mListener->EndListen();
		SAFE_DELETE(mListener);
		CS_UNLOCK
	}
}

void Networker::BeginSessionUpdate()
{
	mSessUpdateThread = (HANDLE) ::_beginthreadex(NULL, 0, SessionUpdater, this, 0, NULL);
}

void Networker::EndSessionUpdate()
{
	if ( mbThreadUpdateSessions ) {
		mbThreadUpdateSessions = false;
		WaitForSingleObject(mSessUpdateThread, INFINITE);
	}
}

Session* Networker::GetSession(int id)			
{ 
	ASSERT(0 <= id && id < GetSessionCount() );
	return mSessionVec[id]; 
}

Session* Networker::GetNewSession()
{
	CS_LOCK

	Session* newSession = NULL;
	for(int i = 0, n = mSessionVec.size(); i < n; ++i) {
		Session* s = mSessionVec[i];
		if ( s == NULL ) {
			s = new Session(this, i, mSendBufferSize, mRecvBufferSize);
			s->Init();
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

	CS_UNLOCK

	return newSession;
}

Session* Networker::AddSession()
{
	Session* newSess = NULL;
	if ( mSessionVec.size() < (std::size_t)mSessionLimitCount ) {
		newSess = new Session(this, GetSessionCount(), mSendBufferSize, mRecvBufferSize);
		newSess->Init();
		mSessionVec.push_back(newSess);
	}
	return newSess;
}

void Networker::DeleteAllSessions()
{
	CS_LOCK
	for(auto &i : mSessionVec) {
		SAFE_DELETE(i);
	}
	mSessionVec.clear();
	CS_UNLOCK
}

void Networker::OnEndIoThread()
{
	::InterlockedDecrement(&mIoWorkingCount);
	ASSERT(mIoWorkingCount >= 0);
}

void Networker::UpdateSessions()
{
	CS_LOCK

	Session* acceptingSession = NULL;
	for(auto &sess : mSessionVec) {
		if ( sess ) {
			sess->Update();

			if ( IsPreAccepter() && sess->IsState(SESSIONSTATE_ACCEPTING) ) {
				acceptingSession = sess;
			}
		}
	}

	//If No Accepting Session, Create New Accepting Session;
	if ( IsPreAccepter() && ! acceptingSession ) {
		AddSession();
	}
	CS_UNLOCK
}

bool Networker::IsPreAccepter()	
{ 
	return (mListener && mbPreAccept);
}

SOCKET Networker::GetListnerSocket()			
{ 
	SOCKET listenSock = mListener ? mListener->GetSocket() : NULL;
	return listenSock; 
}
