#include "PCH.h"
#include "Networker.h"
#include "Listener.h"
#include "TCPSession.h"

#include <process.h>


#define		CS_LOCK				mCritiSect.Enter();
#define		CS_UNLOCK			mCritiSect.Leave();

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

			TCPSession* sess = overlapped->session;
			OverlappedIoType io = overlapped->ioType;
			if ( result ) {
				if ( io == IO_ACCEPT ) {
					sess->OnAccept(INVALID_SOCKET);
				}
				else if ( io == IO_SEND ) {
					sess->OnSendComplete(overlapped, transferBytes);
				}
				else if ( io == IO_RECV ) {
					if ( transferBytes <= 0 )
						sess->OnDisconnect();
					else
						sess->OnRecvComplete(overlapped, transferBytes);
				}
			}
			else {
				//Remote Socket Closed
				sess->OnDisconnect();
			}
		}
	}
	return 0;
}


unsigned __stdcall SessionUpdater (void* arg) 
{
	Networker* networker = (Networker*)arg;

	while (networker->IsThreadUpdatingSessions()) {
		networker->UpdateSessions();
	}

	return 0;
}


Networker::Networker(bool bUseThreadUpdateSession, int ioThreadCount, int sessionReserveCount, int sessionLimitCount, int sendBufferSize, int recvBufferSize)
	: mIocp(INVALID_HANDLE_VALUE)
	, mIoThreadCount(ioThreadCount)
	, mListener(NULL)
	, mbPreAccept(false)
	, mSessionLimitCount(sessionLimitCount)
	, mSendBufferSize(sendBufferSize)
	, mRecvBufferSize(recvBufferSize)
	, mbThreadUpdateSessions(bUseThreadUpdateSession)
	, mSessUpdateThread(INVALID_HANDLE_VALUE)
	, mNetEventDelegator(NULL)
{
	if ( mSessionLimitCount <= 0 )
		mSessionLimitCount = 1;

	if ( sessionReserveCount <= 0 )
		sessionReserveCount = 1;

	mSessionVec.reserve(mSessionLimitCount);

	for(int i = 0; i < sessionReserveCount; ++i) {
		TCPSession* s = new TCPSession(this, i, mSendBufferSize, mRecvBufferSize);
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

	mIoThreadHandles.reserve(mIoThreadCount);
	for (UINT i = 0; i < mIoThreadCount; i++)
	{
		HANDLE hThread = (HANDLE) ::_beginthreadex(NULL, 0, IOCPWorker, this, 0, NULL);
		mIoThreadHandles.push_back(hThread);
		//CloseHandle (hThread);
	}
	
	CS_UNLOCK
}

void Networker::EndIo()
{	
	CS_LOCK
	for (UINT i = 0; i < mIoThreadCount; i++)
		PostQueuedCompletionStatus(GetIocpHandle(), 0, NULL, NULL);
	
	//Wait for All Io Threads Finished
	WaitForMultipleObjects(mIoThreadHandles.size(), (HANDLE*)&mIoThreadHandles.begin(), TRUE, INFINITE);
	for (UINT i = 0; i < mIoThreadCount; i++)
		CloseHandle(mIoThreadHandles[i]);
	mIoThreadHandles.clear();

	if ( mIocp != INVALID_HANDLE_VALUE) {
		CloseHandle(mIocp);
		mIocp = INVALID_HANDLE_VALUE;
	}
	CS_UNLOCK
}

bool Networker::BeginListen(UINT16 port, bool bPreAccept)
{
	EndListen();

	CS_LOCK
	bool bListen = false;
	mbPreAccept = bPreAccept;
	if ( bPreAccept ) {
		mListener = new IocpListener(this, port);
		bListen = mListener->BeginListen();
	}
	else {
		mListener = new SelectListener(this, port);
		bListen = mListener->BeginListen();
	}
	CS_UNLOCK
	return bListen;
}

void Networker::EndListen()
{	
	CS_LOCK
	if ( mListener ) {
		
		mListener->EndListen();
		SAFE_DELETE(mListener);
	}
	CS_UNLOCK
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

TCPSession* Networker::GetSession(int id)
{ 
	ASSERT(0 <= id && id < GetSessionCount() );
	return mSessionVec[id]; 
}

TCPSession* Networker::GetNewSession()
{
	CS_LOCK

		TCPSession* newSession = NULL;
	for(int i = 0, n = mSessionVec.size(); i < n; ++i) {
		TCPSession* s = mSessionVec[i];
		if ( s == NULL ) {
			s = new TCPSession(this, i, mSendBufferSize, mRecvBufferSize);
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

TCPSession* Networker::AddSession()
{
	TCPSession* newSess = NULL;
	if ( mSessionVec.size() < (std::size_t)mSessionLimitCount ) {
		newSess = new TCPSession(this, GetSessionCount(), mSendBufferSize, mRecvBufferSize);
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

void Networker::UpdateSessions()
{
	CS_LOCK

	bool isPreaccepting = IsPreAccept();

	TCPSession* acceptingSession = NULL;
	for(auto &sess : mSessionVec) {
		if ( sess ) {
			sess->Update();

			if ( isPreaccepting && sess->IsState(SESSIONSTATE_ACCEPTING) ) {
				acceptingSession = sess;
			}
		}
	}

	//If No Accepting Session, Create New Accepting Session;
	if ( isPreaccepting && (acceptingSession == NULL) ) {
		AddSession();
	}
	CS_UNLOCK
}

SOCKET Networker::GetListnerSocket()			
{ 
	return (mListener ? mListener->GetSocket() : NULL);
}

void Networker::SetEventDelegator(NetEventDelegator* eventDelegator)
{
	mNetEventDelegator = eventDelegator;
	if ( mNetEventDelegator ) {
		mNetEventDelegator->SetNetworker(this);
	}
}