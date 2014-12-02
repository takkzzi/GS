#include "PCH.h"
#include "Session.h"
#include "Networker.h"


using namespace Core;
using namespace Network;

Session::Session(Networker* networker, int id, int sendBufferSize, int recvBufferSize)
	: mNetworker(networker)
	, mId(id)
	, mState(SESSIONSTATE_NONE)
	, mSock(INVALID_SOCKET)
	, mEvent(WSA_INVALID_EVENT)
	, mOverlappedAccept(NULL)
	, mOverlappedSend(NULL)
	, mOverlappedRecv(NULL)
	, mListenSock(INVALID_SOCKET)
	, mIsAccepter(false)
{	
	mOverlappedAccept = new Overlapped(this, IO_ACCEPT, 0);
	mOverlappedSend = new Overlapped(this, IO_SEND, IOBUFFER_LENGTH);
	mOverlappedRecv = new Overlapped(this, IO_RECV, IOBUFFER_LENGTH);

	//TODO : Buffer Create
	//mSendBuffer->Init();
	//mRecvBuffer->Init();
}

Session::~Session(void)
{
	if ( IsState(SESSIONSTATE_CONNECTED) )
		Disconnect();

	SAFE_DELETE(mOverlappedAccept);
	SAFE_DELETE(mOverlappedSend);
	SAFE_DELETE(mOverlappedRecv);

}

void Session::SetState(SessionState state) 
{
	::InterlockedExchange((LONG*)&mState, (LONG)state);
}

bool Session::Connect(const CHAR* addr, USHORT port)
{
	if ( ! IsState(SESSIONSTATE_NONE) ) 
		return FALSE;
	
	mSock = WSASocket( AF_INET, SOCK_STREAM, 0, NULL, 0, WSA_FLAG_OVERLAPPED );
	if (mSock == INVALID_SOCKET)
		return false;

	BOOL bOptVal = TRUE;
	setsockopt(mSock, SOL_SOCKET, SO_REUSEADDR, (char *) &bOptVal, sizeof(bOptVal));

	memset(&mRemoteAddr, 0, sizeof(mRemoteAddr));

	mRemoteAddr.sin_family				= AF_INET;
	mRemoteAddr.sin_port				= htons( port );
	mRemoteAddr.sin_addr.S_un.S_addr	= inet_addr(addr);

	if ( WSAConnect(mSock, (LPSOCKADDR) &mRemoteAddr, sizeof(SOCKADDR_IN), NULL, NULL, NULL, NULL) == SOCKET_ERROR ) 
	{
		DWORD errCode = WSAGetLastError();
		if (errCode != WSAEWOULDBLOCK)
		{
			Logger::Log(_T("Session"), _T("%d Connect() Error(%s)"), mId, Logger::GetLastErrorMsg(NULL) );

			closesocket(mSock);
			mSock = INVALID_SOCKET;

			return FALSE;
		}
	}

	OnConnect();
	return true;
}

//TODO : Minimum supported client
// Minimum supported client : Windows 8.1, Windows Vista [desktop apps only]
// Minimum supported server : Windows Server 2003 [desktop apps only]
bool Session::Accept(SOCKET listenSock) {

	if ( ! IsState(SESSIONSTATE_NONE) )
		return false;

	mSock = WSASocket( AF_INET, SOCK_STREAM, 0, NULL, 0, WSA_FLAG_OVERLAPPED );
	if (mSock == INVALID_SOCKET)
		return false;

	BOOL bOptVal = TRUE;
	setsockopt(mSock, SOL_SOCKET, SO_REUSEADDR, (char *) &bOptVal, sizeof(bOptVal));

	BOOL bPreAccept = AcceptEx(listenSock, mSock, mOverlappedRecv->wsaBuf.buf, 0, sizeof(SOCKADDR_IN)+16, sizeof(SOCKADDR_IN)+16, 0, (WSAOVERLAPPED*)&(mOverlappedAccept->ov));
	
	if ( ! bPreAccept ){
		if (WSAGetLastError() != ERROR_IO_PENDING && WSAGetLastError() != WSAEWOULDBLOCK)	// Erro Condition
			return false;
	}

	SetState(SESSIONSTATE_ACCEPTING);
	mListenSock = listenSock;
	mIsAccepter = true;
	return true;
}

bool Session::Disconnect(bool bAccept)
{
	if ( mSock == INVALID_SOCKET)
		return TRUE;

	shutdown( mSock, SD_BOTH );

	int err = ::closesocket( mSock );
	if( err == SOCKET_ERROR ) {
		int lasterr = GetLastError();
		//Logger::Log("Session", "Id : %d, closesocket error (ErrorCode:%d)\n", mId, lasterr );
	}

	mSock = INVALID_SOCKET;
	SetState(SESSIONSTATE_NONE);

	//TODO : Clear All Buffers
	//mRecvBuffer->ClearBuffer();
	//mSendBuffer->ClearBuffer();

	if ( bAccept && mIsAccepter ) {
		Accept(mListenSock);
	}

	return true;
}

bool Session::Send(BYTE* data, int dataLen)
{
	//TODO : Sending
	return true;
}

void Session::OnCompletionStatus(Overlapped* overlapped, DWORD transferSize)
{
	if ( overlapped->iotype == IO_ACCEPT )
		OnAccept(0);
	else if ( overlapped->iotype == IO_SEND )
		OnSendComplete(transferSize);
	else if ( overlapped->iotype == IO_RECV )
		OnRecvComplete(transferSize);
	else
		ASSERT(0);
}

void Session::OnConnect() 
{
	SetState(SESSIONSTATE_CONNECTED);

	::CreateIoCompletionPort((HANDLE)mSock, mNetworker->GetIocpHandle(), (ULONG_PTR)this, 0);

	//mRecvBuffer->ClearBuffer();
	//mSendBuffer->ClearBuffer();

	//First Reserve Receive
	DWORD dwBytes, dwFlags = 0;
	int res = ::WSARecv(mSock, &mOverlappedRecv->wsaBuf, 1, &dwBytes, &dwFlags, (WSAOVERLAPPED*)&(mOverlappedRecv->ov), NULL);
	if ( res == SOCKET_ERROR && ( WSAGetLastError() != ERROR_IO_PENDING ) ) {
		Disconnect();
	}
}

void Session::OnDisconnect() 
{
	Logger::Log("Session", "OnDisconnect() : %d", mId);
	Disconnect(true);
}

void Session::OnAccept(SOCKET listenSock)
{
	if ( ! IsState(SESSIONSTATE_ACCEPTING) && IsState(SESSIONSTATE_NONE) ){
		int addrlen = sizeof(SOCKADDR);
		mSock = accept(listenSock, (SOCKADDR*)&mRemoteAddr, &addrlen);
	}

	if ( mOverlappedSend )
		mOverlappedSend->Reset();

	if ( mOverlappedRecv )
		mOverlappedRecv->Reset();

	Logger::Log("Session", "OnAccept() : %d", mId);

	OnConnect();
}

void Session::OnSendComplete(DWORD sendSize)
{
	if ( sendSize == 0 ) {
		OnDisconnect();
		return;
	}

	//TODO : Send Complete
	//mSendBuffer->Complete(sendSize)
}

void Session::OnRecvComplete(DWORD recvSize)
{
	if ( recvSize == 0 ) { //Close At Remote Session
		OnDisconnect();
		return;
	}
	
	//TODO : Receive Complete
	//mRecvBuffer->Complete(recvSize)

	//Recv Again
	DWORD dwBytes, dwFlags = 0;
	int res = ::WSARecv(mSock, &mOverlappedRecv->wsaBuf, 1, &dwBytes, &dwFlags, (WSAOVERLAPPED*)&(mOverlappedRecv->ov), NULL);
	if ( res == SOCKET_ERROR && ( WSAGetLastError() != ERROR_IO_PENDING ) ) {
		OnDisconnect();
		return;
	}	
}

