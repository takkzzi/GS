#pragma once

class Session;

class SessionMgr
{
public:
	SessionMgr(void);
	virtual ~SessionMgr(void);

public:
	Session*	CreateSessions(int n = 1);
	Session*	GetSession();

	void		Update();

private:
	void DestroyAllSessions();

private:
	
	std::list<Session*>		m_sessionList;
};
