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
	mSendBuffer.Init(sendBufferSize, 0x0000ffff);
	mRecvBuffer.Init(recvBufferSize, 0x0000ffff);
}

Session::~Session(void)
{
	if ( IsState(SESSIONSTATE_CONNECTED) )
		Disconnect(false);
	else
		ResetState(false);
	
	mCriticalSec.Enter();
	if ( mAcceptBuffer )
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
		mRecvBuffer.ClearAll();
		mSendBuffer.ClearAll();
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
			LOG_LASTERROR_A("Session", false);
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

	int bufferLen ((sizeof(sockaddr_in) + 16) * 2);
	BOOL bPreAccept = ::AcceptEx(listenSock, mSock, mAcceptBuffer, bufferLen, sizeof(SOCKADDR_IN)+16, sizeof(SOCKADDR_IN)+16, 0, &(mAcceptIoData.ov));
	
	if ( ! bPreAccept ){
		DWORD err = WSAGetLastError();
		if (err != ERROR_IO_PENDING && err != WSAEWOULDBLOCK) {	// Erro Condition 
			LOG_LASTERROR_A("Session",  true);
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

	char* emptyBuf = NULL;
	int emptySize = 0;		//Much as Possible
	if ( mRecvBuffer.GetEmpty(&emptyBuf, &emptySize) ) {

		mRecvIoData.Reset(emptyBuf);

		WSABUF wsabuf;
		wsabuf.buf = emptyBuf;
		wsabuf.len = emptySize;

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
	}

	mCriticalSec.Leave();
}

bool Session::Send()
{
	if ( mIsPendingSend )
		return false;

	mCriticalSec.Enter();

	bool bResult = false;
	char* dataBuf = NULL;
	int dataSize = 0x0000ffff;	//== Max TCP/IP Packet Size

	if ( mSendBuffer.GetData(&dataBuf, &dataSize, true, true) )
	{
		WSABUF wsabuf;
		wsabuf.buf = dataBuf;
		wsabuf.len = dataSize;

		mSendIoData.Reset(dataBuf);

		DWORD transferBytes;
		int res = ::WSASend(mSock, &(wsabuf), 1, &transferBytes, 0, &(mSendIoData.ov), NULL);

		if ( res == 0 ) {
			mIsPendingSend = bResult = true;
			OnSendComplete(&mSendIoData, transferBytes);
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

	return bResult;;
}

void Session::OnConnect() 
{
	SetState(SESSIONSTATE_CONNECTED);

	::CreateIoCompletionPort((HANDLE)mSock, mNetworker->GetIocpHandle(), (ULONG_PTR)this, 0);

	mSendBuffer.ClearAll();
	mRecvBuffer.ClearAll();

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

	if ( IsState(SESSIONSTATE_ACCEPTING) ) {
		/*TODO : Remote Session Info
		GetAcceptExSockaddrs(mAcceptBuffer, 0, sizeof(sockaddr_in) + 16, sizeof(sockaddr_in) + 16,
		(sockaddr **) &Local, &LocalLength, (sockaddr **) &Remote,&RemoteLength);
		*/
	}

	mCriticalSec.Leave();

	OnConnect();
}

void Session::OnSendComplete(OverlappedIoData* ioData, DWORD sendSize)
{
	if ( sendSize > 0 ) {
		mCriticalSec.Enter();
		if ( mIsPendingSend ) { 
			ASSERT( ioData->bufPtr == mSendBuffer.GetDataHead() );
			if ( mSendBuffer.ClearData(sendSize) )
				mIsPendingSend = false;
			else 
				ASSERT(0 && "SendBuffer Clear Error !");
		}
		mCriticalSec.Leave();
	}
	else {
		Disconnect(true);
	}
}

void Session::OnRecvComplete(OverlappedIoData* ioData, DWORD recvSize)
{
	if ( recvSize > 0 ) { 
		if ( mRecvBuffer.GetDataTail() == ioData->bufPtr ) {
			if ( mRecvBuffer.ReserveData(recvSize) ) {
				PreReceive();
			}
			else {
				ASSERT(0 && "mRecvBuffer.ReserveData() OnRecv Error!");
			}
		}
		else {
			ASSERT(0 && "mRecvBuffer.GetDataTail() != ioData->bufPtr");
		}		
	}
	else {
	//Remote Session Closed
		Disconnect(true);
	}
}

bool Session::PushSend(char* data, int dataLen)
{
	mCriticalSec.Enter();
	bool bResult = mSendBuffer.PushData(data, dataLen);
	mCriticalSec.Leave();
	return bResult;
}

//Note : Returned Buffer Must be "ClearRecv()" after Use.
PacketBase* Session::PopRecv()
{
	mCriticalSec.Enter();
	char* data = NULL;
	int size = sizeof(PacketBase);
	PacketBase* packet = NULL;
	if ( mRecvBuffer.GetData(&data, &size, false, false) ) {
		size = ((PacketBase*)data)->mPacketSize;
		if ( mRecvBuffer.GetData(&data, &size, false, true) ) {
			packet = (PacketBase*)data;
		}
	}
	mCriticalSec.Leave();
	return packet;
}

bool Session::ClearRecv(int bufSize)
{
	mCriticalSec.Enter();
	return mRecvBuffer.ClearData(bufSize);
	mCriticalSec.Leave();
}