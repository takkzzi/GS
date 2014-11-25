#include "PCH.h"
#include "Listener.h"
#include "Networker.h"
#include "Session.h"
//#include <process.h>


using namespace Core;
using namespace Network;

Listener::Listener(Networker* networker, UINT16 port)
: mPort(port)
, mSock(INVALID_SOCKET)
, mEvent(WSA_INVALID_EVENT)
, mNetworker(networker)
{
}

Listener::~Listener(void)
{
}


bool Listener::Begin(bool bSuspend/*=false*/) 
{	
	if (mSock != INVALID_SOCKET || ! IsState(THREAD_NONE) )
		return FALSE;

	mSock = WSASocket (PF_INET, SOCK_STREAM, IPPROTO_TCP, 0, 0, WSA_FLAG_OVERLAPPED);
	if (mSock == INVALID_SOCKET)
		return false;

	SOCKADDR_IN addr;
	addr.sin_family		= AF_INET;
	addr.sin_port		= htons (mPort);
	addr.sin_addr.s_addr= htonl (INADDR_ANY);

	mEvent = WSACreateEvent();
	if (mEvent == WSA_INVALID_EVENT)
		return false;

	if ( ::bind (mSock, (struct sockaddr*) &addr, sizeof(addr)) == SOCKET_ERROR )
		return false;
	
	if ( ::WSAEventSelect (mSock, mEvent, FD_ACCEPT) == SOCKET_ERROR )
		return false;

	if ( ::listen (mSock, SOMAXCONN) == SOCKET_ERROR )
		return false;

	__super::Begin(true);
	__super::Resume();

	return TRUE;
}

bool Listener::End()
{
	return Thread::End();
}

DWORD Listener::ThreadTick()
{
	DWORD listeningTimeout = 1000;

	DWORD dwRet = ::WSAWaitForMultipleEvents(1, &mEvent, FALSE, listeningTimeout, FALSE);

	if ( dwRet != WSA_WAIT_EVENT_0 )
		return 1;

	WSANETWORKEVENTS netEvt;
	int netEvent = ::WSAEnumNetworkEvents(mSock, mEvent, &netEvt);
	if ( netEvent != 0 ) 
	{
		//ASSERT( netEvent != 0 && _T("Listner::WSAEnumNetworkEvents is non-zero.") );
		Logger::LogWarning(_T("Network"), _T("Listner::WSAEnumNetworkEvents is non-zero. Error:%d"), GetLastError());
		return 1;
	}

	if (netEvt.lNetworkEvents & FD_ACCEPT)
		OnAccept();

	return 1;
}

void Listener::OnEnd(bool bTerminated)
{
	Thread::OnEnd(bTerminated);

	Logger::Log(_T("Listener"), TEXT("Listener::OnEnd"));

	//Reset Variables
	closesocket(mSock);
	mSock = INVALID_SOCKET;

	CloseHandle(mEvent);
	mEvent = WSA_INVALID_EVENT;
}

void Listener::OnAccept()
{
	Session* se = mNetworker->GetNewSession();
	se->OnAccept(mNetworker, mSock);
}
