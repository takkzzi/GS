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
	, mAcceptBufferItem(new BufferItem(this, IO_ACCEPT, recvBufferSize, 0))
	, mSendBuffer(new SessionBuffer())
	, mRecvBuffer(new SessionBuffer())
	, mListenSock(INVALID_SOCKET)
	, mIsAccepter(false)
{	
	//I/O Buffer Init
	mSendBuffer->Init(this, IO_SEND, 32, sendBufferSize);
	mRecvBuffer->Init(this, IO_RECV, 32, recvBufferSize);
}

Session::~Session(void)
{
	if ( IsState(SESSIONSTATE_CONNECTED) )
		Disconnect();

	SAFE_DELETE(mAcceptBufferItem);
	SAFE_DELETE(mRecvBuffer);
	SAFE_DELETE(mSendBuffer);

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
bool Session::PreAccept(SOCKET listenSock) {

	if ( ! IsState(SESSIONSTATE_NONE) )
		return false;

	mSock = WSASocket( AF_INET, SOCK_STREAM, 0, NULL, 0, WSA_FLAG_OVERLAPPED );
	if (mSock == INVALID_SOCKET)
		return false;

	BOOL bOptVal = TRUE;
	setsockopt(mSock, SOL_SOCKET, SO_REUSEADDR, (char *) &bOptVal, sizeof(bOptVal));

	BOOL bPreAccept = AcceptEx(listenSock, mSock, mAcceptBufferItem->mWsaBuf.buf, sizeof(mAcceptBufferItem->mWsaBuf.buf), sizeof(SOCKADDR_IN)+16, sizeof(SOCKADDR_IN)+16, 0, (WSAOVERLAPPED*)&(mAcceptBufferItem->mOverlapped->ov));
	
	if ( ! bPreAccept ){
		DWORD err = WSAGetLastError();
		if (err != ERROR_IO_PENDING && err != WSAEWOULDBLOCK)	// Erro Condition
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

	mRecvBuffer->Clear();
	mSendBuffer->Clear();

	if ( bAccept && mIsAccepter ) {
		PreAccept(mListenSock);
	}

	return true;
}

bool Session::Send(char* data, int dataLen)
{
	mSendBuffer->PushCopy(data);

	return true;
}

void Session::OnCompletionStatus(Overlapped* overlapped, DWORD transferSize)
{
	if ( overlapped->ioType == IO_ACCEPT )
		OnAccept(INVALID_SOCKET);
	else if ( overlapped->ioType == IO_SEND )
		OnSendComplete(overlapped, transferSize);
	else if ( overlapped->ioType == IO_RECV )
		OnRecvComplete(overlapped, transferSize);
	else
		ASSERT(0);
}

void Session::OnConnect() 
{
	SetState(SESSIONSTATE_CONNECTED);

	::CreateIoCompletionPort((HANDLE)mSock, mNetworker->GetIocpHandle(), (ULONG_PTR)this, 0);

	mRecvBuffer->Clear();
	mSendBuffer->Clear();

	//Start Overlapped Receive
	PreReceive();
}

void Session::OnDisconnect() 
{
	Logger::Log("Session", "OnDisconnect() : %d", mId);
	Disconnect(true);
}

void Session::OnAccept(SOCKET listenSock)
{
	if ( IsState(SESSIONSTATE_NONE) ){
		int addrlen = sizeof(SOCKADDR);
		mSock = accept(listenSock, (SOCKADDR*)&mRemoteAddr, &addrlen);
	}

	Logger::Log("Session", "OnAccept() : %d", mId);

	OnConnect();
}

void Session::OnSendComplete(Overlapped* overlapped, DWORD sendSize)
{
	if ( sendSize == 0 ) {
		OnDisconnect();
		return;
	}

	//TODO : Send Complete
	mSendBuffer->OnSend(overlapped, sendSize);
}

void Session::OnRecvComplete(Overlapped* overlapped, DWORD recvSize)
{
	if ( recvSize == 0 ) { //Close At Remote Session
		OnDisconnect();
		return;
	}
	//TODO : Receive Complete
	mRecvBuffer->OnRecv(overlapped, recvSize);

	PreReceive();	
}

void Session::PreReceive()
{
	DWORD dwBytes, dwFlags = 0;
	
	BufferItem* buf = mRecvBuffer->GetEmptyBuffer();
	int res = ::WSARecv(mSock, &buf->mWsaBuf, 1, &dwBytes, &dwFlags, (WSAOVERLAPPED*)&(buf->mOverlapped), NULL);

	if ( res == 0 ) {
		//Received Immediately
		OnRecvComplete(buf->mOverlapped, dwBytes);
	}
	else if ( (res == SOCKET_ERROR) && ( WSAGetLastError() != ERROR_IO_PENDING ) ) {
		Disconnect();
	}
}