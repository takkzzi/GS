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
	, mOverlappedSend(NULL)
	, mOverlappedRecv(NULL)
{
	mType = IOKey_Session;
	//TODO : Buffer Create
	//mSendBuffer->Init();
	//mRecvBuffer->Init();
}

Session::~Session(void)
{
	if ( IsState(SESSIONSTATE_CONNECTED) )
		Disconnect();

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

	memset(&mRemoteAddr, 0, sizeof(mRemoteAddr));

	mRemoteAddr.sin_family				= AF_INET;
	mRemoteAddr.sin_port				= htons( port );
	mRemoteAddr.sin_addr.S_un.S_addr	= inet_addr(addr);

	if ( WSAConnect(mSock, (LPSOCKADDR) &mRemoteAddr, sizeof(SOCKADDR_IN), NULL, NULL, NULL, NULL) == SOCKET_ERROR ) 
	{
		DWORD errCode = WSAGetLastError();
		if (errCode != WSAEWOULDBLOCK)
		{
			closesocket(mSock);
			mSock = INVALID_SOCKET;

			return FALSE;
		}
	}

	::CreateIoCompletionPort((HANDLE)mSock, mNetworker->GetIocpHandle(), (ULONG_PTR)this, 0);

	SetState(SESSIONSTATE_CONNECTED);

	return true;
}

bool Session::Disconnect()
{
	if ( mSock == INVALID_SOCKET)
		return TRUE;

	//Cancel Sending, Receiving
	if ( SOCKET_ERROR == shutdown( mSock, SD_BOTH ) ) {
		int lasterr = WSAGetLastError();
		Logger::Log("Session", "Id : %d, shutdown() error (ErrorCode:%d)\n", mId, lasterr );
	}

	int err = ::closesocket( mSock );
	if( err == SOCKET_ERROR ) {
		int lasterr = GetLastError();
		Logger::Log("Session", "Id : %d, closesocket error (ErrorCode:%d)\n", mId, lasterr );
	}

	mSock = INVALID_SOCKET;
	SetState(SESSIONSTATE_NONE);

	//TODO : Clear All Buffers
	//mRecvBuffer->ClearBuffer();
	//mSendBuffer->ClearBuffer();

	return true;
}

bool Session::Send(BYTE* data, int dataLen)
{
	//TODO : Sending
	return true;
}

void Session::OnAccept(SOCKET listenSock)
{
	int addrlen = sizeof(SOCKADDR);
	mSock = accept(listenSock, (SOCKADDR*)&mRemoteAddr, &addrlen);

	if ( mOverlappedSend )
		mOverlappedSend->Reset();
	else 
		mOverlappedSend = new Overlapped(IO_SEND, MAX_BUFFER_LENGTH);
	
	if ( mOverlappedRecv )
		mOverlappedRecv->Reset();
	else
		mOverlappedRecv = new Overlapped(IO_RECV, MAX_BUFFER_LENGTH);

	::CreateIoCompletionPort((HANDLE)mSock, mNetworker->GetIocpHandle(), (ULONG_PTR)this, 0);

	//mRecvBuffer->ClearBuffer();
	//mSendBuffer->ClearBuffer();

	SetState(SESSIONSTATE_CONNECTED);

	//First Reserve Receive
	DWORD dwBytes, dwFlags = 0;
	int res = ::WSARecv(mSock, &mOverlappedRecv->wsaBuf, 1, &dwBytes, &dwFlags, (WSAOVERLAPPED*)&(mOverlappedRecv->ov), NULL);
	if ( res == SOCKET_ERROR && ( WSAGetLastError() != ERROR_IO_PENDING ) ) {
		Disconnect();
	}
}

void Session::OnSendComplete(int sendSize)
{
	if ( sendSize <= 0 ) {
		OnDisconnect();
		return;
	}

	//TODO : Send Complete
	//mSendBuffer->Complete(sendSize)
}

void Session::OnRecvComplete(int recvSize)
{
	if ( recvSize <= 0 ) { //Close At Remote Session
		OnDisconnect();
		return;
	}

	DWORD dwBytes, dwFlags = 0;
	int res = ::WSARecv(mSock, &mOverlappedRecv->wsaBuf, 1, &dwBytes, &dwFlags, (WSAOVERLAPPED*)&(mOverlappedRecv->ov), NULL);
	if ( res == SOCKET_ERROR && ( WSAGetLastError() != ERROR_IO_PENDING ) ) {
		OnDisconnect();
		return;
	}

	//TODO : Receive Complete
	//mRecvBuffer->Complete(recvSize)
}

void Session::OnDisconnect() 
{
	//Logger::Log("Session", "Id : %d, OnDisconnect", mId, WSAGetLastError() );
	Disconnect();
}