#include "StdAfx.h"
#include "NetDefines.h"
#include "Network.h"
#include "Listener.h"
#include "SessionMgr.h"
#include "Session.h"

#include <winsock2.h>

bool Network::ms_bInit = false;

//static
void Network::Startup()
{
	if ( ms_bInit ) return;

	WSADATA wsaData;
	WSAStartup(MAKEWORD(2,2), &wsaData);
	ms_bInit = true;
}

//static
void Network::Shutdown()
{
	if ( ! ms_bInit ) return;

	WSACleanup();
	ms_bInit  = false;
}

//static
Network* Network::Create(NetMethod method, NetRole role, UINT16 port, int sessionCount) 
{
	if ( ! ms_bInit ) {
		//TODO : Log.DebugLog("Error", "Network Not Startup Yet. Should Call Network::Startup().", bAssert);
		return NULL;
	}

	return new Network(method, role, port, sessionCount);
}

//static
void Network::Destroy(Network* network) {
	if ( ! network ) return;
	delete network;
}

Network::Network(NetMethod method, NetRole role, UINT16 port, int sessionCount) 
	: m_listener(NULL)
	, m_sessionMgr(NULL)
{
	//For Server
	if ( role == NetRole::Server ) {
		m_listener = new Listener(port);
		m_listener->Init();
	}

	m_sessionMgr = new SessionMgr;
}

Network::~Network(void)
{	
	delete m_listener;
	delete m_sessionMgr;
}

void Network::OnConnect() {

}
