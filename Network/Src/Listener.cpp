#include "PCH.h"
#include "Listener.h"
#include "IOCP.h"
#include "Session.h"
//#include <process.h>


using namespace Core;
using namespace Network;

/*
unsigned __stdcall EventSelectThread( LPVOID param ) 
{
	Listener* listener = (Listener*)param;

	while (1)
	{
		DWORD dwRet = WSAWaitForMultipleEvents(1, &listener->m_event, FALSE, INFINITE, FALSE);
		if (dwRet == WSA_WAIT_TIMEOUT)
			continue;

		WSANETWORKEVENTS netEvt;
		WSAEnumNetworkEvents (listener->m_sock, listener->m_event, &netEvt);

		if (netEvt.lNetworkEvents & FD_ACCEPT)
			listener->OnConnect();
	}
	return 0;
}
*/

Listener::Listener(IOCP* iocp, UINT16 port)
: mPort(port)
, mSock(INVALID_SOCKET)
, mEvent(WSA_INVALID_EVENT)
, mIocp(iocp)
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

void Listener::End(bool bForceTerminate/*=false*/)
{
	Thread::End(true);
}

DWORD Listener::Run()
{
	DWORD listeningTimeout = 1000;

	while (1)
	{
		//End Thread
		if ( IsState(THREAD_END) ) {
			break;
		}

		DWORD dwRet = ::WSAWaitForMultipleEvents(1, &mEvent, FALSE, listeningTimeout, FALSE);

		if ( dwRet != WSA_WAIT_EVENT_0 )
			continue;

		WSANETWORKEVENTS netEvt;
		int netEvent = ::WSAEnumNetworkEvents(mSock, mEvent, &netEvt);
		if ( netEvent != 0 ) 
		{
			//ASSERT( netEvent != 0 && _T("Listner::WSAEnumNetworkEvents is non-zero.") );
			Logger::LogWarning(_T("Network"), _T("Listner::WSAEnumNetworkEvents is non-zero. Error:%d"), GetLastError());
			continue;
		}

		if (netEvt.lNetworkEvents & FD_ACCEPT)
			OnAccept();
	}
	return 0;
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
	Session* se = mIocp->GetNewSession();
	se->OnAccept(mIocp, mSock);
}
