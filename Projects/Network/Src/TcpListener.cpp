#include "PCH.h"
#include "TcpListener.h"
#include "TcpNetworker.h"
#include "TcpSession.h"
//#include <process.h>


using namespace Core;
using namespace Network;


bool Listener::EndListen()
{
	if ( mSock == INVALID_SOCKET )
		return false;

	closesocket(mSock);
	mSock = INVALID_SOCKET;
	return true;
}

//////////////// End of Listener ///////////////////////


SelectListener::SelectListener(TcpNetworker* networker, UINT16 port)
	: Listener(networker, port)
{
	mEvents[0] = WSA_INVALID_EVENT;
	mEvents[1] = WSA_INVALID_EVENT;
}

SelectListener::~SelectListener(void)
{
}

bool SelectListener::BeginListen()
{
	if (mSock != INVALID_SOCKET || ! IsState(THREAD_NONE) )
		return FALSE;

	mSock = ::WSASocket (PF_INET, SOCK_STREAM, IPPROTO_TCP, 0, 0, WSA_FLAG_OVERLAPPED);
	if (mSock == INVALID_SOCKET)
		return false;

	BOOL bOptVal = TRUE;
	setsockopt(mSock, SOL_SOCKET, SO_REUSEADDR, (char *) &bOptVal, sizeof(bOptVal));

	SOCKADDR_IN addr;
	addr.sin_family		= AF_INET;
	addr.sin_port		= htons (mPort);
	addr.sin_addr.s_addr= htonl (INADDR_ANY);	

	if ( ::bind (mSock, (struct sockaddr*) &addr, sizeof(addr)) == SOCKET_ERROR )
		return false;
	
	mEvents[0] = CreateEvent(NULL, true, false, NULL);		//Important : For Quit Listening
	mEvents[1] = WSACreateEvent();

	if (mEvents[1] == WSA_INVALID_EVENT)
		return false;

	if ( ::WSAEventSelect (mSock, mEvents[1], FD_ACCEPT) == SOCKET_ERROR )
		return false;

	if ( ::listen (mSock, SOMAXCONN) == SOCKET_ERROR )
		return false;

	return __super::Begin(false);
}

bool SelectListener::EndListen()
{
	SetEvent(mEvents[0]);						//For Quit Thread Loop
	::WaitForSingleObject(mhThread, INFINITE);	//Waiting For Thread Return.

	CloseHandle(mEvents[0]);
	CloseHandle(mEvents[1]);

	__super::EndListen();

	return true;
}

DWORD SelectListener::ThreadTick()
{
	DWORD dwRet = ::WSAWaitForMultipleEvents(2, mEvents, FALSE, INFINITE, FALSE);
	int eventIndex = dwRet - WSA_WAIT_EVENT_0;
	if (eventIndex == 0)	//Quit Thread Loop
		return 0;

	WSANETWORKEVENTS netEvt;
	int netEvent = ::WSAEnumNetworkEvents(mSock, mEvents[1], &netEvt);
	if ( netEvent != 0 )
	{
		LOG_LASTERROR(_T("SelectListener"), false);
		return 1;
	}

	if (netEvt.lNetworkEvents & FD_ACCEPT)
		OnAccept();

	return 1;
}

void SelectListener::OnEnd(bool bTerminated)
{
	Thread::OnEnd(bTerminated);

	//Reset Variables
	closesocket(mSock);
	mSock = INVALID_SOCKET;

	CloseHandle(mEvents[0]);
	CloseHandle(mEvents[1]);
	mEvents[0] = WSA_INVALID_EVENT;
	mEvents[1] = WSA_INVALID_EVENT;

	Logger::Log(_T("Listener"), TEXT("SelectListener::OnEnd"));
	
}

void SelectListener::OnAccept()
{
	TcpSession* se = mNetworker->GetNewSession();
	se->OnAccept(mSock);
}

///////////////////// End of SelectListener ////////////////////////////////////////



IocpListener::IocpListener(TcpNetworker* networker, UINT16 port)
	: Listener(networker, port)
{
}

IocpListener::~IocpListener(void)
{
}

bool IocpListener::BeginListen() 
{
	if (mSock != INVALID_SOCKET)
		return FALSE;

	mSock = WSASocket (PF_INET, SOCK_STREAM, IPPROTO_TCP, 0, 0, WSA_FLAG_OVERLAPPED);
	if (mSock == INVALID_SOCKET)
		return false;

	BOOL bOptVal = TRUE;
	setsockopt(mSock, SOL_SOCKET, SO_REUSEADDR, (char *) &bOptVal, sizeof(bOptVal));

	SOCKADDR_IN			addr;
	addr.sin_family		= AF_INET;
	addr.sin_port		= htons (mPort);
	addr.sin_addr.s_addr= htonl (INADDR_ANY);	

	if ( ::bind (mSock, (struct sockaddr*) &addr, sizeof(addr)) == SOCKET_ERROR ) {
		LOG_LASTERROR(_T("IocpListener"), true);
		return false;
	}

	if ( ::listen (mSock, SOMAXCONN) == SOCKET_ERROR ) {
		LOG_LASTERROR(NULL, true);
		return false;
	}

	CreateIoCompletionPort((HANDLE)mSock, mNetworker->GetIocpHandle(), (DWORD)mSock, 0);

	Logger::Log(_T("Log"), _T("Start Listen..."));

	return true;
}

bool IocpListener::EndListen()
{
	Logger::Log(_T("Log"), _T("End Listen... "));
	return __super::EndListen();
}
