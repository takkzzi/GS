#include "StdAfx.h"
#include "NetDefines.h"
#include "SessionMgr.h"
#include "Session.h"
#include "Listener.h"

using namespace std;

//TODO : SessionBundle Ã³¸® Thread 
unsigned __stdcall ThreadFunc(void* network) 
{
	return 0;
}

SessionMgr::SessionMgr()
{
}

SessionMgr::~SessionMgr(void)
{
	DestroyAllSessions();
}

Session* SessionMgr::CreateSessions(int n)
{
	Session* firstSess = NULL;
	
	for (int i = 0; i < n; ++i)
	{
		Session* newSess = new Session;
		m_sessionList.push_back(newSess);

		if ( i == 0 )
			firstSess = newSess;
	}

	return firstSess;
}

void SessionMgr::Update()
{
}

void SessionMgr::DestroyAllSessions()
{
	list<Session*>::iterator itr = m_sessionList.begin();
	for (; itr != m_sessionList.end(); ++itr)
	{
		Session* sess = (*itr);
		delete sess;
	}
}

