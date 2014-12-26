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
	, mAcceptBuffer(NULL)
	, mListenSock(INVALID_SOCKET)
	, mIsAccepter(false)
	, mbSendCompleted(true)
	, mbRecvCompleted(true)
{	
	mCriticalSec.Enter();
	//I/O Buffer Init
	mAcceptIoData.Init(IO_ACCEPT, this);
	mSendIoData.Init(IO_SEND, this);
	mRecvIoData.Init(IO_RECV, this);

	mAcceptBuffer = new char[64];
	mSendBuffer.Init(sendBufferSize, 0x0000ffff);
	mRecvBuffer.Init(recvBufferSize, 0x0000ffff);

	mCriticalSec.Leave();
}

void Session::Init()
{
	if ( mNetworker->IsPreAccepter() ) {
		StartAccept(mNetworker->GetListnerSocket());
	}
}

Session::~Session(void)
{
	ResetState(false);

	if ( mAcceptBuffer ) {
		mCriticalSec.Enter();
		delete[] mAcceptBuffer;
		mCriticalSec.Leave();
	}
}

void Session::SetState(SessionState state) 
{
	::InterlockedExchange((LONG*)&mState, (LONG)state);
}

void Session::ResetState(bool bClearDataQ) 
{
	//mCriticalSec.Enter();

	if ( IsState (SESSIONSTATE_NONE) )
		return;

	SetState(SESSIONSTATE_NONE);

	if ( mSock != INVALID_SOCKET ) {
		::closesocket(mSock);
		mSock = INVALID_SOCKET;
	}

	mbSendCompleted = true;
	mbRecvCompleted = true;

	if ( bClearDataQ ) {
		mRecvBuffer.ClearAll();
		mSendBuffer.ClearAll();
	}

	//mCriticalSec.Leave();
}

bool Session::Connect(const CHAR* addr, USHORT port)
{
	if ( ! IsState(SESSIONSTATE_NONE) ) 
		return FALSE;
	
	mCriticalSec.Enter();

	mSock = WSASocket( AF_INET, SOCK_STREAM, 0, NULL, 0, WSA_FLAG_OVERLAPPED );
	if (mSock == INVALID_SOCKET) {
		mCriticalSec.Leave();
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
			mCriticalSec.Leave();

			ResetState(true);
			return FALSE;
		}
	}

	mCriticalSec.Leave();

	OnConnect();

	return true;
}

bool Session::Disconnect()
{
	if ( mSock == INVALID_SOCKET)
		return false;

	if ( IsState(SESSIONSTATE_CONNECTED) ) {
		mCriticalSec.Enter();
		::shutdown( mSock, SD_BOTH );
		mCriticalSec.Leave();
	}

	ResetState(true);
	return true;
}

//TODO : Minimum supported client
// Minimum supported client : Windows 8.1, Windows Vista [desktop apps only]
// Minimum supported server : Windows Server 2003 [desktop apps only]
bool Session::StartAccept(SOCKET listenSock) {

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

	int bufferLen = 0;//((sizeof(sockaddr_in) + 16) * 2);
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

bool Session::StartReceive()
{
	if ( ! IsState(SESSIONSTATE_CONNECTED) )
		return false;

	if ( ! mbRecvCompleted )
		return false;

	//mCriticalSec.Enter();

	if ( ! mbRecvCompleted ) {
		//mCriticalSec.Leave();
		return false;
	}

	WSABUF wsabuf;
	wsabuf.buf = NULL;
	wsabuf.len = 0;		//Much as Possible

	if ( mRecvBuffer.GetEmpty(&(wsabuf.buf), (int*)&(wsabuf.len)) ) {

		mRecvIoData.Reset(wsabuf.buf);

		DWORD transferBytes;
		DWORD dwFlags = 0;

		mbRecvCompleted = false;

		int res = ::WSARecv(mSock, &wsabuf, 1, &transferBytes, &dwFlags, (WSAOVERLAPPED*)&(mRecvIoData), NULL);

		if ( (res == SOCKET_ERROR) && ( WSAGetLastError() != ERROR_IO_PENDING ) ) {
			SetState(SESSIONSTATE_DISCONNECTING);
		}
		else {
			
			if ( res == 0 ) {
				//Received Immediately
				//OnRecvComplete(recvBuff, dwBytes);
			}
		}
	}

	//mCriticalSec.Leave();
	return !mbRecvCompleted;
}

bool Session::Send()
{
	if ( ! IsState(SESSIONSTATE_CONNECTED) )
		return false;

	if ( ! mbSendCompleted )
		return false;

	WSABUF wsabuf;
	wsabuf.buf = NULL;
	wsabuf.len = 0x0000ffff;	//== Max TCP/IP Packet Size;

	if ( ! mSendBuffer.GetData(&wsabuf.buf, (int*)&wsabuf.len, true, true) )
		return false;

	mSendIoData.Reset(wsabuf.buf);

	mbSendCompleted = false;

	DWORD transferBytes;
	int sendResult = ::WSASend(mSock, &(wsabuf), 1, &transferBytes, 0, &(mSendIoData.ov), NULL);

	if ( (sendResult == SOCKET_ERROR) && (WSAGetLastError() != ERROR_IO_PENDING) ) {
		SetState(SESSIONSTATE_DISCONNECTING);
	}
	else {
		
		//if ( sendResult == 0 ) 
			//OnSendComplete(&mSendIoData, transferBytes);
	}
	
	return !mbSendCompleted;
}

void Session::OnConnect() 
{
	mCriticalSec.Enter();

	::CreateIoCompletionPort((HANDLE)mSock, mNetworker->GetIocpHandle(), (ULONG_PTR)this, 0);

	mSendBuffer.ClearAll();
	mRecvBuffer.ClearAll();

	SetState(SESSIONSTATE_CONNECTED);

	mbRecvCompleted = true;
	mbSendCompleted = true;

	mCriticalSec.Leave();

	StartReceive();
}

void Session::OnAccept(SOCKET listenSock)
{
	mCriticalSec.Enter();

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
	mCriticalSec.Leave();

	OnConnect();
}

void Session::OnSendComplete(OverlappedIoData* ioData, DWORD sendSize)
{
	if ( mbSendCompleted )
		return;

	mCriticalSec.Enter();
	ASSERT( ioData->bufPtr == mSendBuffer.GetDataHead() );
	if ( mSendBuffer.ClearData(sendSize) )
		mbSendCompleted = true;
	else 
		ASSERT(0 && "SendBuffer Clear Error !");
	mCriticalSec.Leave();
}

void Session::OnRecvComplete(OverlappedIoData* ioData, DWORD recvSize)
{
	if ( recvSize <= 0 ) {  //Remote Session Closed
		ResetState(true);
		return;
	}

	//mCriticalSec.Enter();

	if ( ! mbRecvCompleted ) {

		if ( mRecvBuffer.ReserveData(recvSize) ) {
			mbRecvCompleted = true;
		}
		else {
			ASSERT(0 && "RecvBuffer.Reserve() Error !");
		}
	}

	//mCriticalSec.Leave();

	StartReceive();
}

bool Session::PushSend(char* data, int dataLen)
{
	mCriticalSec.Enter();
	//TEST
	static UINT packetId = 0;
	AlphabetPacket* alphaPacket = (AlphabetPacket*)data;
	alphaPacket->mPacketId = packetId++;

	bool bResult = mSendBuffer.PushData(data, dataLen);
	

	//TEST
	if ( bResult ) {
		AlphabetPacket* alphaPacket = (AlphabetPacket*)data;
		LogPacket("Send", alphaPacket);
	}
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

	int dataSize = mRecvBuffer.GetDataSize();

	if ( mRecvBuffer.GetData(&data, &size, false, false) ) {
		size = ((PacketBase*)data)->mPacketSize;
		if ( mRecvBuffer.GetData(&data, &size, false, true) ) {
			packet = (PacketBase*)data;
		}
	}

	//TEST
	if ( packet ) {
		AlphabetPacket* alphaPacket = (AlphabetPacket*)packet;
		LogPacket("Received", alphaPacket);
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

void Session::Update()
{
	if ( IsState(SESSIONSTATE_DISCONNECTING) ) {
		Disconnect();
	}

	if ( IsState(SESSIONSTATE_NONE) ) {
		if ( mNetworker->IsPreAccepter() ) {
			StartAccept(mNetworker->GetListnerSocket() );
		}
	}
	else if ( IsState(SESSIONSTATE_CONNECTED) ) {
		Send();
		StartReceive();
	}
}

//TEST
void Session::LogPacket(char* prefix, AlphabetPacket* packet)
{

	ASSERT(sizeof(*packet) == sizeof(AlphabetPacket));

	char categ[64];
	sprintf_s(categ, "%s Session(Id:%d)", prefix, mId);

	char logMsg[32];
	size_t msgSize = sizeof(packet->mData);
	memcpy(logMsg, packet->mData, msgSize);
	logMsg[msgSize] = '\0';

	Logger::Log(categ, "%s(Num:%d)", logMsg, packet->mPacketId);
}