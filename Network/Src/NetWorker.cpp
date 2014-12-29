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
					sess->OnSendComplete(overlapped, transferBytes);
				}
				else if ( io == IO_RECV ) {
					sess->OnRecvComplete(overlapped, transferBytes);
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

unsigned __stdcall SessionUpdater (void* arg)
{
	Networker* networker = (Networker*)arg;

	/*
	do{
		Sleep(10);
	}
	while(networker->UpdateSessions());
	*/

	while(1)
	{
		Sleep(20);
		if ( !networker->IsUpdatingSession() )
			break;
	}

	return 0;
}


Networker::Networker(int threadCount, int reserveSessionCount, int sessionLimitCount, int sendBufferSize, int recvBufferSize)
	: mIocp(INVALID_HANDLE_VALUE)
	, mThreadCount(threadCount)
	, mIoWorkingCount(0)
	, mListener(NULL)
	, mbPreAccept(false)
	, mSessionLimitCount(sessionLimitCount)
	, mSendBufferSize(sendBufferSize)
	, mRecvBufferSize(recvBufferSize)
	, mbUpdateSessions(false)
	, mSessUpdateThread(INVALID_HANDLE_VALUE)
{
	if ( mSessionLimitCount <= 0 )
		mSessionLimitCount = 1;

	if ( reserveSessionCount <= 0 )
		reserveSessionCount = 1;

	mSessionVec.reserve(mSessionLimitCount);

	for(int i = 0; i < reserveSessionCount; ++i) {
		Session* s = new Session(this, i, mSendBufferSize, mRecvBufferSize);
		s->Init();
		mSessionVec.push_back(s);
	}

	BeginIo();
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
	mCriticalSec.Enter();
	
	mIocp = ::CreateIoCompletionPort(INVALID_HANDLE_VALUE, NULL, 0, 0);
	if (mIocp == NULL) {
		LOG_ERROR(_T("IOCP Creation Fail."));
	}

	if (mThreadCount <= 0 ) {
		DWORD processors = System::GetProcessorCount();
		mThreadCount = processors * 2 + 1; //optimized count for processors
	}

	mIoWorkingCount = mThreadCount;
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

	while ( mIoWorkingCount > 0) {
	}

	if ( mIocp != INVALID_HANDLE_VALUE) {
		CloseHandle(mIocp);
		mIocp = INVALID_HANDLE_VALUE;
	}
	mCriticalSec.Leave();
}

void Networker::BeginListen(UINT16 port, bool bPreAccept)
{
	EndListen();

	mCriticalSec.Enter();
	
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

	mCriticalSec.Leave();
}

void Networker::EndListen()
{	
	if ( mListener ) {
		mCriticalSec.Enter();
		mListener->EndListen();
		SAFE_DELETE(mListener);
		mCriticalSec.Leave();
	}
}

void Networker::BeginSessionUpdate()
{
	mbUpdateSessions = true;
	mSessUpdateThread = (HANDLE) ::_beginthreadex(NULL, 0, SessionUpdater, this, 0, NULL);
}

void Networker::EndSessionUpdate()
{
	mbUpdateSessions = false;
	WaitForSingleObject(mSessUpdateThread, INFINITE);
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

	mCriticalSec.Leave();

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

/*
void Networker::StartAcceptAll()
{
	mCriticalSec.Enter();

	for(unsigned int i = 0; i < mSessionVec.size(); ++i) {
		mSessionVec[i]->StartAccept(mListener->GetSocket());
	}

	mCriticalSec.Leave();
}
*/


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
	::InterlockedDecrement(&mIoWorkingCount);
	ASSERT(mIoWorkingCount >= 0);
}

bool Networker::UpdateSessions()
{
	if ( ! mbUpdateSessions )
		return false;

	mCriticalSec.Enter();

	Session* acceptingSession = NULL;
	for(auto &sess : mSessionVec) {
		if ( ! mbUpdateSessions )
			break;

		if ( sess ) {
			sess->Update();

			if ( IsPreAccepter() && sess->IsState(SESSIONSTATE_ACCEPTING) ) {
				acceptingSession = sess;
			}
		}
	}
	
	//If No Accepting Session, Create New Accepting Session;
	if ( IsPreAccepter() && ! acceptingSession ) {
		//AddSession();
	}
	mCriticalSec.Leave();
	return true;
}

bool Networker::IsPreAccepter()	
{ 
	bool bRes = (mListener && mbPreAccept);
	return bRes;
}

SOCKET Networker::GetListnerSocket()			
{ 
	SOCKET listenSock = mListener ? mListener->GetSocket() : NULL;
	return listenSock; 
}
