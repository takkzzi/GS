#include "PCH.h"
#include "Listener.h"
#include "Networker.h"
#include "Session.h"
//#include <process.h>


using namespace Core;
using namespace Network;

/*
Listener::Listener(Networker* networker, UINT16 port)
: mPort(port)
, mSock(INVALID_SOCKET)
, mNetworker(networker)
{
	mEvents[0] = WSA_INVALID_EVENT;
	mEvents[1] = WSA_INVALID_EVENT;
}

Listener::~Listener(void)
{
}


bool Listener::Begin(bool bSuspend) 
{	
	if (mSock != INVALID_SOCKET || ! IsState(THREAD_NONE) )
		return FALSE;

	mSock = WSASocket (PF_INET, SOCK_STREAM, IPPROTO_TCP, 0, 0, WSA_FLAG_OVERLAPPED);
	if (mSock == INVALID_SOCKET)
		return false;

	//Important : For Quit Listening
	mEvents[0] = CreateEvent(NULL, true, false, NULL);

	SOCKADDR_IN addr;
	addr.sin_family		= AF_INET;
	addr.sin_port		= htons (mPort);
	addr.sin_addr.s_addr= htonl (INADDR_ANY);	

	if ( ::bind (mSock, (struct sockaddr*) &addr, sizeof(addr)) == SOCKET_ERROR )
		return false;
	
	mEvents[1] = WSACreateEvent();
	if (mEvents[1] == WSA_INVALID_EVENT)
		return false;

	if ( ::WSAEventSelect (mSock, mEvents[1], FD_ACCEPT) == SOCKET_ERROR )
		return false;

	if ( ::listen (mSock, SOMAXCONN) == SOCKET_ERROR )
		return false;

	return __super::Begin();
}

bool Listener::End()
{
	SetEvent(mEvents[0]);	//For Quit Thread Loop

	return __super::End();
}

DWORD Listener::ThreadTick()
{
	DWORD dwRet = ::WSAWaitForMultipleEvents(2, mEvents, FALSE, INFINITE, FALSE);
	int eventIndex = dwRet - WSA_WAIT_EVENT_0;
	if (eventIndex == 0)	//Quit Thread Loop
		return 0;

	WSANETWORKEVENTS netEvt;
	int netEvent = ::WSAEnumNetworkEvents(mSock, mEvents[1], &netEvt);
	if ( netEvent != 0 ) 
	{
		//ASSERT( netEvent != 0 && _T("Listner::WSAEnumNetworkEvents is non-zero.") );
		Logger::LogWarning(_T("Listener"), _T("Listner::WSAEnumNetworkEvents is non-zero. Error:%l"), GetLastError());
		return 1;
	}

	if (netEvt.lNetworkEvents & FD_ACCEPT)
		OnAccept();

	return 1;
}

void Listener::OnEnd(bool bTerminated)
{
	Thread::OnEnd(bTerminated);

	//Reset Variables
	closesocket(mSock);
	mSock = INVALID_SOCKET;

	CloseHandle(mEvents[0]);
	CloseHandle(mEvents[1]);
	mEvents[0] = WSA_INVALID_EVENT;
	mEvents[1] = WSA_INVALID_EVENT;

	Logger::Log(_T("Listener"), TEXT("Listener::OnEnd"));
	
}

void Listener::OnAccept()
{
	Session* se = mNetworker->GetNewSession();
	se->OnAccept(mSock);
}
*/

Listener::Listener(Networker* networker, UINT16 port)
: mbBegan(false)
, mPort(port)
, mSock(INVALID_SOCKET)
, mNetworker(networker)
{
	mType = IOKey_Listener;

	bool begin = Begin();
	ASSERT(begin);

}

Listener::~Listener(void)
{
	End();
}

bool Listener::Begin() 
{	
	if (mSock != INVALID_SOCKET)
		return FALSE;

	mSock = WSASocket (PF_INET, SOCK_STREAM, IPPROTO_TCP, 0, 0, WSA_FLAG_OVERLAPPED);
	if (mSock == INVALID_SOCKET)
		return false;

	SOCKADDR_IN addr;
	addr.sin_family		= AF_INET;
	addr.sin_port		= htons (mPort);
	addr.sin_addr.s_addr= htonl (INADDR_ANY);	

	if ( ::bind (mSock, (struct sockaddr*) &addr, sizeof(addr)) == SOCKET_ERROR )
		return false;

	if ( ::listen (mSock, SOMAXCONN) == SOCKET_ERROR )
		return false;

	CreateIoCompletionPort((HANDLE)mSock, mNetworker->GetIocpHandle(), (ULONG_PTR)this, 0);
	mbBegan = true;

	Logger::Log(_T("Listener"), TEXT("Listener::Begin()"));
	return true;
}

bool Listener::End()
{
	if ( ! mbBegan )
		return false;

	closesocket(mSock);
	mSock = INVALID_SOCKET;

	Logger::Log(_T("Listener"), TEXT("Listener::End()"));
	return true;
}

void Listener::OnAccept()
{
	Session* se = mNetworker->GetNewSession();
	se->OnAccept(mSock);
}