#include "PCH.h"
#include "Session.h"
#include "Networker.h"

#include <mswsock.h>
#include <mstcpip.h>


using namespace Core;
using namespace Network;

//#define THREADSYNC_DEBUG

#ifdef THREADSYNC_DEBUG
	#define				CS_LOCK				Logger::LogDebugString("%s (%s)", "Enter", __FUNCTION__);		mCritiSect.Enter(); 
	#define				CS_UNLOCK			mCritiSect.Leave();		Logger::LogDebugString("%s (%s)", "Leave", __FUNCTION__);
#else
	#define				CS_LOCK				mCritiSect.Enter()
	#define				CS_UNLOCK			mCritiSect.Leave()
#endif // DEBUG


//Call the 'AcceptEx' function directly, rather than refer to the Mswsock.lib library.
LPFN_ACCEPTEX GetAcceptExFunction(SOCKET sock)
{
	static LPFN_ACCEPTEX	fnAcceptEx = NULL;
	if ( fnAcceptEx == NULL ) {
		DWORD dwBytes;
		GUID GuidAcceptEx = WSAID_ACCEPTEX;
		int iRes = ::WSAIoctl(sock, SIO_GET_EXTENSION_FUNCTION_POINTER, &GuidAcceptEx, sizeof(GuidAcceptEx), &fnAcceptEx, sizeof(fnAcceptEx), &dwBytes, NULL, NULL);
		ASSERT(iRes == 0);
	}

	return fnAcceptEx;
}

Session::Session(Networker* networker, int id, int sendBufferSize, int recvBufferSize)
	: mNetworker(networker)
	, mId(id)
	, mState(SESSIONSTATE_NONE)
	, mSock(INVALID_SOCKET)
	, mEvent(WSA_INVALID_EVENT)
	, mAcceptBuffer(NULL)
	, mListenSock(INVALID_SOCKET)
	, mIsAccepter(false)
	, mbSendPending(false)
	, mbRecvStarted(false)
	, mbRecvLock(false)
{	
	CS_LOCK;

	//I/O Buffer Init
	mAcceptIoData.Init(IO_ACCEPT, this);
	mSendIoData.Init(IO_SEND, this);
	mRecvIoData.Init(IO_RECV, this);

	mAcceptBuffer = new char[64];
	mSendBuffer.Init(sendBufferSize, 0x0000ffff);
	mRecvBuffer.Init(recvBufferSize, 0x0000ffff);

	CS_UNLOCK;

	Init();
}

void Session::Init()
{
	if ( mNetworker->IsPreAccept() ) {
		StartAccept(mNetworker->GetListnerSocket());
	}
}

Session::~Session(void)
{
	Disconnect();

	if ( mAcceptBuffer ) {
		CS_LOCK;
		SAFE_DELETE_ARRAY(mAcceptBuffer);
		CS_UNLOCK;
	}
}

void Session::SetState(SessionState state) 
{
	::InterlockedExchange((LONG*)&mState, (LONG)state);
}

void Session::ResetState(bool bClearDataQ) 
{
	if ( IsState (SESSIONSTATE_NONE) )
		return;

	SetState(SESSIONSTATE_NONE);

	if ( mSock != INVALID_SOCKET ) {
		CS_LOCK;
		::closesocket(mSock);
		mSock = INVALID_SOCKET;
		CS_UNLOCK;
	}

	mbSendPending = false;
	mbRecvStarted = false;

	if ( bClearDataQ ) {
		mRecvBuffer.ClearAll();
		mSendBuffer.ClearAll();
	}
}

bool Session::Connect(const CHAR* addr, USHORT port)
{
	if ( ! IsState(SESSIONSTATE_NONE) ) 
		return FALSE;
	
	CS_LOCK;

	mSock = WSASocket( AF_INET, SOCK_STREAM, 0, NULL, 0, WSA_FLAG_OVERLAPPED );
	if (mSock == INVALID_SOCKET) {
		CS_UNLOCK;
		return false;
	}

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
			CS_UNLOCK;

			ResetState(true);
			return FALSE;
		}
	}

	OnConnect();

	CS_UNLOCK;

	return true;
}

bool Session::Disconnect()
{
	if ( mSock == INVALID_SOCKET)
		return false;

	if ( IsState(SESSIONSTATE_CONNECTED) ) {
		CS_LOCK;
		::shutdown( mSock, SD_BOTH );
		CS_UNLOCK;
	}

	ResetState(true);
	return true;
}

bool Session::StartAccept(SOCKET listenSock) {

	if ( ! IsState(SESSIONSTATE_NONE) )
		return false;

	CS_LOCK;
    
	mSock = WSASocket( AF_INET, SOCK_STREAM, 0, NULL, 0, WSA_FLAG_OVERLAPPED );
	ASSERT(mSock != INVALID_SOCKET);

	LPFN_ACCEPTEX pfnAcceptEx = GetAcceptExFunction(listenSock);

	if (mSock != INVALID_SOCKET && pfnAcceptEx) 
	{
		BOOL bOptVal = TRUE;
		setsockopt(mSock, SOL_SOCKET, SO_REUSEADDR, (char *) &bOptVal, sizeof(bOptVal));

		int bufferLen = 0;//((sizeof(sockaddr_in) + 16) * 2);
		//BOOL bPreAccept = ::AcceptEx(listenSock, mSock, mAcceptBuffer, bufferLen, sizeof(SOCKADDR_IN)+16, sizeof(SOCKADDR_IN)+16, 0, &(mAcceptIoData.ov));
		BOOL bPreAccept = pfnAcceptEx(listenSock, mSock, mAcceptBuffer, bufferLen, sizeof(SOCKADDR_IN)+16, sizeof(SOCKADDR_IN)+16, 0, &(mAcceptIoData.ov));
	
		DWORD lastErr = WSAGetLastError();
		if ( ! bPreAccept && (lastErr != ERROR_IO_PENDING && lastErr != WSAEWOULDBLOCK) ) {
			LOG_LASTERROR_A("Session",  true);
		
			CS_UNLOCK;
			return false;
		}
	}

	SetState(SESSIONSTATE_ACCEPTING);
	mListenSock = listenSock;
	mIsAccepter = true;

	CS_UNLOCK;
	return true;
}

bool Session::StartReceive()
{
	if ( ! IsState(SESSIONSTATE_CONNECTED) )
		return false;

	if ( mbRecvStarted || mbRecvLock )
		return false;

	CS_LOCK;

	WSABUF wsabuf = { 0, NULL };	// len == 0 Much as Possible
	wsabuf.buf = mRecvBuffer.GetEmpty((int*)&(wsabuf.len));

	if ( wsabuf.buf && wsabuf.len > 0 ) {

		mRecvIoData.Reset(wsabuf.buf);

		DWORD transferBytes;
		DWORD dwFlags = 0;

		mbRecvStarted = true;

		int res = ::WSARecv(mSock, &wsabuf, 1, &transferBytes, &dwFlags, (WSAOVERLAPPED*)&(mRecvIoData), NULL);

		if ( (res == SOCKET_ERROR) && ( WSAGetLastError() != ERROR_IO_PENDING ) ) {
			LOG_LASTERROR(_T("WSARecv() Error!"), true);
			SetState(SESSIONSTATE_DISCONNECTING);
		}
		else {
			Logger::LogDebugString("Recv %d (head %d, tail %d)", wsabuf.len, mRecvBuffer.GetDataHeadPos(), mRecvBuffer.GetDataTailPos());
			if ( res == 0 ) {  //Received Immediately
			}
		}
	}

	CS_UNLOCK;
	return mbRecvStarted;
}

bool Session::Send()
{
	if ( ! IsState(SESSIONSTATE_CONNECTED) )
		return false;

	if ( mbSendPending )
		return false;

	CS_LOCK;

	WSABUF wsabuf = { 0x0000ffff, NULL};	// len = Max TCP/IP Packet Size;
	wsabuf.buf = mSendBuffer.Read((int*)&wsabuf.len, true, true);
	if ( wsabuf.buf && wsabuf.len > 0) {

		mSendIoData.Reset(wsabuf.buf);

		mbSendPending = true;

		DWORD transferBytes;
		int sendResult = ::WSASend(mSock, &(wsabuf), 1, &transferBytes, 0, &(mSendIoData.ov), NULL);

		if ( (sendResult == SOCKET_ERROR) && (WSAGetLastError() != ERROR_IO_PENDING) ) {
			SetState(SESSIONSTATE_DISCONNECTING);
		}
		else {

			if ( sendResult == 0 )  {  //Send Immediately
				//OnSendComplete(&mSendIoData, transferBytes);
			}
		}
	}
	CS_UNLOCK;

	return mbSendPending;
}

void Session::OnAccept(SOCKET listenSock)
{
	CS_LOCK;

	if ( IsState(SESSIONSTATE_NONE) ){
		int addrlen = sizeof(SOCKADDR);
		mSock = accept(listenSock, (SOCKADDR*)&mRemoteAddr, &addrlen);
	}

	//Test
	Logger::Log("Session", "OnAccept() : %d", mId);

	if ( IsState(SESSIONSTATE_ACCEPTING) ) {
		/*TODO : Remote Session Info
		GetAcceptExSockaddrs(mAcceptBuffer, 0, sizeof(sockaddr_in) + 16, sizeof(sockaddr_in) + 16,
		(sockaddr **) &Local, &LocalLength, (sockaddr **) &Remote,&RemoteLength);
		*/
	}
	
	OnConnect();

	SetKeepAliveOpt();

	CS_UNLOCK;
}

void Session::OnConnect() 
{
	::CreateIoCompletionPort((HANDLE)mSock, mNetworker->GetIocpHandle(), (ULONG_PTR)this, 0);

	mSendBuffer.ClearAll();
	mRecvBuffer.ClearAll();

	SetState(SESSIONSTATE_CONNECTED);

	mbSendPending = false;
	mbRecvStarted = false;
}

void Session::OnSendComplete(OverlappedIoData* ioData, DWORD sendSize)
{
	ASSERT( ioData->bufPtr == mSendBuffer.GetDataHead() );
	mbSendPending = ! mSendBuffer.ClearData(sendSize);
	ASSERT(!mbSendPending && "SendBuffer Clear Error !");
}

void Session::OnRecvComplete(OverlappedIoData* ioData, DWORD recvSize)
{
	/*
	if ( recvSize <= 0 ) {  //Remote Session Closed
		OnDisconnect();
		return;
	}
	*/

	if ( mRecvBuffer.AddDataTail(recvSize) ) {
		mbRecvStarted = false;
		Logger::LogDebugString("Recved %d (Head %d, Tail %d)", recvSize, mRecvBuffer.GetDataHeadPos(), mRecvBuffer.GetDataTailPos());
	}
	else {
		ASSERT(0 && "RecvBuffer.Reserve() Error !");
	}
}

void Session::OnDisconnect()
{
	ResetState(true);
}

bool Session::WriteSendBuffer(char* data, int dataLen)
{
	bool bResult = mSendBuffer.Write(data, dataLen);
	return bResult;
}

/*
//Note : Returned Buffer Must be "ClearRecv()" after Use.
PacketBase* Session::ReadData()
{
	CS_LOCK;

	bool bCanMakeCircleData = mbRecvLock = ! mbRecvStarted;	//Important : Don't Change DataTailPos in Recv-Progress.
	char* data = NULL;

	int minSize = sizeof(PacketBase);
	PacketBase* packet = NULL;

	if ( data = mRecvBuffer.Read(&minSize, false, bCanMakeCircleData) ) {
		minSize = ((PacketBase*)data)->mPacketSize;
		if ( data = mRecvBuffer.Read(&minSize, false, bCanMakeCircleData) ) {
			packet = (PacketBase*)data;
		}
	}

	//if ( dataTail != mRecvBuffer.GetDataTailPos() )	//If Tail Changed, Lock Recv Until ClearRecv.
	mbRecvLock = false;

	//TEST
	if ( packet ) {
		AlphabetPacket* alphaPacket = (AlphabetPacket*)packet;
		LogPacket("PopData", alphaPacket);
	}

	CS_UNLOCK;
	return packet;
}
*/

char* Session::ReadRecvBuffer(int bufSize)
{
	if ( mbRecvLock )
		return NULL;

	CS_LOCK;

	bool bCanMakeCircleData = mbRecvLock = ! mbRecvStarted;
	char* data = mRecvBuffer.Read(&bufSize, false, bCanMakeCircleData);
	mbRecvLock = false;

	CS_UNLOCK;
	return data;
}

bool Session::ClearRecvBuffer(int bufSize)
{
	if ( mbRecvLock )
		return false;

	bool bClear = mRecvBuffer.ClearData(bufSize);
	ASSERT(bClear);
	CS_LOCK;
	mbRecvLock = !bClear;
	CS_UNLOCK;
	return bClear;
}

void Session::Update()
{
	if ( IsState(SESSIONSTATE_DISCONNECTING) ) {
		Disconnect();
	}

	if ( IsState(SESSIONSTATE_NONE) ) {
		Init();
	}
	else if ( IsState(SESSIONSTATE_CONNECTED) ) {
		Send();
		StartReceive();
	}
}

//Set KeepAlive Option
void Session::SetKeepAliveOpt()
{
	bool optval = TRUE;
	int valsize = sizeof(optval);
	if ( 0 == ::getsockopt(mSock, SOL_SOCKET, SO_KEEPALIVE, (char*)&optval, &valsize) ) {
		if ( (false == optval) && (0 != ::setsockopt(mSock, SOL_SOCKET, SO_KEEPALIVE, (char*)&optval, valsize)) ) {
			LOG_LASTERROR(_T("Socket"), true);
		}
	}

	tcp_keepalive keepalive;
	keepalive.onoff = true;
	keepalive.keepalivetime = KEEPALIVE_TIME;
	keepalive.keepaliveinterval = KEEPALIVE_INTERVAL;

	DWORD bufferSize = sizeof(tcp_keepalive);
	DWORD cbBytesReturned = 0;

	int ioctlRes = WSAIoctl(mSock, SIO_KEEPALIVE_VALS,  (LPVOID)&keepalive, bufferSize, NULL, 0, (LPDWORD)&cbBytesReturned, NULL, NULL);
	ASSERT(ioctlRes == 0);
}

