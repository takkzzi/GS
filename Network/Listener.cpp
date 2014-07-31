#include "StdAfx.h"
#include "NetDefines.h"
#include "Listener.h"
#include "Network.h"

#include <process.h>


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

Listener::Listener(UINT16 port)
: m_port(port)
, m_sock(INVALID_SOCKET)
, m_event(WSA_INVALID_EVENT)
{	
}

Listener::~Listener(void)
{
}
 
bool Listener::Init() {
	m_sock = WSASocket (PF_INET, SOCK_STREAM, IPPROTO_TCP, 0, 0, 0);
	if (m_sock == INVALID_SOCKET)
		return false;

	struct sockaddr_in addr;
	addr.sin_family		= AF_INET;
	addr.sin_port		= htons (m_port);
	addr.sin_addr.s_addr= htonl (INADDR_ANY);

	m_event = WSACreateEvent();
	if (m_event == WSA_INVALID_EVENT)
		return false;

	if ( bind (m_sock, (struct sockaddr*) &addr, sizeof(addr)) == SOCKET_ERROR )
		return false;

	
	if ( WSAEventSelect (m_sock, m_event, FD_ACCEPT) == SOCKET_ERROR )
		return false;

	if ( listen (m_sock, SOMAXCONN) == SOCKET_ERROR )
		return false;

	HANDLE hThread = (HANDLE) _beginthreadex( NULL, 0, EventSelectThread, (LPVOID)this, 0, NULL );
	CloseHandle( hThread );	

	return true;
}

void Listener::OnConnect() {
	m_network->OnConnect();
}


