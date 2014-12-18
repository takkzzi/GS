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
	, mListenSock(INVALID_SOCKET)
	, mIsAccepter(false)
	, mIsPendingSend(false)
{	
	//I/O Buffer Init
	mAcceptIoData.Init(IO_ACCEPT, this);
	mSendIoData.Init(IO_SEND, this);
	mRecvIoData.Init(IO_RECV, this);

	mAcceptBuffer = new char[recvBufferSize];
	mSendBufferQ.Init(SESSION_BUFFER_Q_SIZE, sendBufferSize);
	mRecvBufferQ.Init(SESSION_BUFFER_Q_SIZE, recvBufferSize);
}

Session::~Session(void)
{
	if ( IsState(SESSIONSTATE_CONNECTED) )
		Disconnect(false);
	else
		ResetState(false);

	mCriticalSec.Enter();
	delete[] mAcceptBuffer;
	mCriticalSec.Leave();
}

void Session::SetState(SessionState state) 
{
	::InterlockedExchange((LONG*)&mState, (LONG)state);
}

void Session::ResetState(bool bClearDataQ) 
{
	mCriticalSec.Enter();

	if ( mSock != INVALID_SOCKET ) {
		closesocket(mSock);
		mSock = INVALID_SOCKET;
	}

	SetState(SESSIONSTATE_NONE);
	mIsPendingSend = false;

	if ( bClearDataQ ) {
		mRecvBufferQ.Clear();
		mSendBufferQ.Clear();
	}

	mCriticalSec.Leave();
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
			LogLastError(_T("Session"), _T("Connect() Error"), false);
			ResetState(true);
			return FALSE;
		}
	}

	OnConnect();
	return true;
}

bool Session::Disconnect(bool bReAccept)
{
	if ( mSock == INVALID_SOCKET)
		return false;

	mCriticalSec.Enter();
	::shutdown( mSock, SD_BOTH );
	mCriticalSec.Leave();

	ResetState(true);

	if ( bReAccept && mIsAccepter ) {
		PreAccept(mListenSock);
	}

	return true;
}

//TODO : Minimum supported client
// Minimum supported client : Windows 8.1, Windows Vista [desktop apps only]
// Minimum supported server : Windows Server 2003 [desktop apps only]
bool Session::PreAccept(SOCKET listenSock) {

	if ( ! IsState(SESSIONSTATE_NONE) )
		return false;

	mCriticalSec.Enter();

	mSock = WSASocket( AF_INET, SOCK_STREAM, 0, NULL, 0, WSA_FLAG_OVERLAPPED );
	if (mSock == INVALID_SOCKET) {
		mCriticalSec.Leave();
		return false;
	}

	BOOL bOptVal = TRUE;
	setsockopt(mSock, SOL_SOCKET, SO_REUSEADDR, (char *) &bOptVal, sizeof(bOptVal));

	BOOL bPreAccept = ::AcceptEx(listenSock, mSock, mAcceptBuffer, 0, sizeof(SOCKADDR_IN)+16, sizeof(SOCKADDR_IN)+16, 0, &(mAcceptIoData.ov));
	
	if ( ! bPreAccept ){
		DWORD err = WSAGetLastError();
		if (err != ERROR_IO_PENDING && err != WSAEWOULDBLOCK) {	// Erro Condition 
			LogLastError(_T("Session"), _T("AcceptEx() Error!"), true);
			mCriticalSec.Leave();
			return false;
		}
	}

	SetState(SESSIONSTATE_ACCEPTING);
	mListenSock = listenSock;
	mIsAccepter = true;

	mCriticalSec.Leave();
	return true;
}

void Session::PreReceive()
{
	mCriticalSec.Enter();

	SessionBuffer* recvBuff = mRecvBufferQ.GetEmpty();
	mRecvIoData.Reset(recvBuff);

	WSABUF wsabuf;
	wsabuf.buf = recvBuff->buf;
	wsabuf.len = recvBuff->len;

	DWORD transferBytes;
	DWORD dwFlags = 0;

	int res = ::WSARecv(mSock, &wsabuf, 1, &transferBytes, &dwFlags, (WSAOVERLAPPED*)&(mRecvIoData.ov), NULL);

	if ( res == 0 ) {
		//Received Immediately
		//OnRecvComplete(recvBuff, dwBytes);
	}
	else if ( (res == SOCKET_ERROR) && ( WSAGetLastError() != ERROR_IO_PENDING ) ) {
		Disconnect(true);
	}

	mCriticalSec.Leave();
}

bool Session::Send()
{
	if ( mIsPendingSend )
		return false;

	mCriticalSec.Enter();

	bool bResult = false;
	SessionBuffer* sendBuf = mSendBufferQ.GetFront();
	if ( sendBuf )
	{
		WSABUF wsabuf;
		wsabuf.buf = sendBuf->buf;
		wsabuf.len = sendBuf->len;

		mSendIoData.Reset(sendBuf);

		DWORD transferBytes;
		int res = ::WSASend(mSock, &(wsabuf), 1, &transferBytes, 0, &(mSendIoData.ov), NULL);

		if ( res == 0 ) {
			mIsPendingSend = bResult = true;
			OnSendComplete(sendBuf, transferBytes);
		}
		else if ( res == SOCKET_ERROR ){
			if ( WSAGetLastError() != ERROR_IO_PENDING ) {
				//LogLastError(_T("Session"), _T("Session:Send Error"), true);
				Disconnect(true);
				bResult = false;
			}
			else {
				mIsPendingSend = bResult = true;
			}
		}
	}
	
	mCriticalSec.Leave();

	return true;;
}

void Session::OnConnect() 
{
	SetState(SESSIONSTATE_CONNECTED);

	::CreateIoCompletionPort((HANDLE)mSock, mNetworker->GetIocpHandle(), (ULONG_PTR)this, 0);

	mSendBufferQ.Clear();
	mRecvBufferQ.Clear();

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
	mCriticalSec.Enter();

	if ( IsState(SESSIONSTATE_NONE) ){
		int addrlen = sizeof(SOCKADDR);
		mSock = accept(listenSock, (SOCKADDR*)&mRemoteAddr, &addrlen);
	}
	Logger::Log("Session", "OnAccept() : %d", mId);

	mCriticalSec.Leave();

	OnConnect();
}

void Session::OnSendComplete(SessionBuffer* buf, DWORD sendSize)
{
	if ( sendSize > 0 ) {
		mCriticalSec.Enter();
		if ( mIsPendingSend && mSendBufferQ.OnIoComplete(buf, sendSize) ) {
			mIsPendingSend = false;
		}
		mCriticalSec.Leave();
	}
	else {
		Disconnect(true);
	}
}

void Session::OnRecvComplete(SessionBuffer* buf, DWORD recvSize)
{
	if ( recvSize > 0 ) { 
		mRecvBufferQ.OnIoComplete(buf, recvSize);
		PreReceive();
	}
	else {
	//Remote Session Closed
		Disconnect(true);
	}
}

bool Session::PushSend(char* data, int dataLen)
{
	mCriticalSec.Enter();
	mSendBufferQ.Push(data, dataLen);
	mCriticalSec.Leave();
	return true;
}

//Note : Returned Buffer Must be "Clear()" after Use.
SessionBuffer* Session::PopRecv()
{
	mCriticalSec.Enter();
	SessionBuffer* buf = mRecvBufferQ.Pop(false);	//No Clear
	mCriticalSec.Leave();
	return buf;
}