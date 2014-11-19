#include "PCH.h"
#include "Session.h"
#include "IOCP.h"


using namespace Core;
using namespace Network;

Session::Session(int id, int sendBufferSize, int recvBufferSize)
	: mId(id)
	, mState(SESSION_NONE)
	, mSock(INVALID_SOCKET)
	, mEvent(WSA_INVALID_EVENT)
	, mOverlappedSend(NULL)
	, mOverlappedRecv(NULL)
{
	//TODO : Buffer Create
}

Session::~Session(void)
{
	if ( IsState(SESSION_CONNECTED) )
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
	if ( ! IsState(SESSION_NONE) ) 
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
		if (WSAGetLastError() != WSAEWOULDBLOCK)
		{
			closesocket(mSock);
			mSock = INVALID_SOCKET;

			return FALSE;
		}
	}

	SetState(SESSION_CONNECTED);

	return true;
}

bool Session::Disconnect()
{
	if ( mSock == INVALID_SOCKET)
		return TRUE;

	//Cancel Sending, Receiving
	shutdown( mSock, SD_BOTH );

	int err = closesocket( mSock );
	if( err == SOCKET_ERROR ) {
		int lasterr = WSAGetLastError();
		Logger::Log("Session", "Id : %d, close socket error(ErrorCode:%d)\n", mId, WSAGetLastError() );
	}

	mSock = INVALID_SOCKET;
	SetState(SESSION_NONE);

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

void Session::OnAccept(IOCP* iocp, SOCKET listenSock)
{
	int addrlen = sizeof(SOCKADDR);
	mSock = accept(listenSock, (SOCKADDR*)&mRemoteAddr, &addrlen);
	
	//TODO : Pre-Accepting with AcceptEx()

	//mSock = ::WSASocket(AF_INET, SOCK_STREAM, IPPROTO_TCP, NULL, 0, WSA_FLAG_OVERLAPPED);
	//::AcceptEx(listenSock, mSock,

	::CreateIoCompletionPort((HANDLE)mSock, iocp->GetIocpHandle(), (ULONG_PTR)this, 0);

	SAFE_DELETE(mOverlappedSend);
	mOverlappedSend = new Overlapped(IO_SEND, mSock, MAX_BUFFER_LENGTH);
	SAFE_DELETE(mOverlappedRecv);
	mOverlappedRecv = new Overlapped(IO_RECV, mSock, MAX_BUFFER_LENGTH);

	//mRecvBuffer->ClearBuffer();
	//mSendBuffer->ClearBuffer();

	SetState(SESSION_CONNECTED);

	//First Reserve Receive
	DWORD dwBytes, dwFlags = 0;
	int res = ::WSARecv(mSock, &mOverlappedRecv->wsaBuf, 1, &dwBytes, &dwFlags, (WSAOVERLAPPED*)&(mOverlappedRecv->ov), NULL);
	if ( res == SOCKET_ERROR && ( WSAGetLastError() != ERROR_IO_PENDING ) ) {
		Disconnect();
	}
	else {
		Logger::Log("Session", "Id : %d, OnAccept", GetId() );
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
	Logger::Log("Session", "Id : %d, OnDisconnect", mId, WSAGetLastError() );
	Disconnect();
}