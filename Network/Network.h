#pragma once

class Listener;
class SessionMgr;
class Session;


class Network
{
public:

	static void			Startup();
	static void			Shutdown();
	static Network*		Create(NetMethod method, NetRole role, UINT16 port, int sessionCount);
	static void			Destroy(Network* network);

protected:
	Network(NetMethod method, NetRole role, UINT16 port, int sessionCount);
	virtual ~Network(void);

public:
	void				OnConnect();
	void				Update();


private:
	static	bool		ms_bInit;

private:
	NetRole				m_role;
	NetMethod			m_method;

	Listener*			m_listener;
	SessionMgr*			m_sessionMgr;
};

