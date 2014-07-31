#pragma once

class Network;

class Listener
{
	friend unsigned __stdcall EventSelectThread( LPVOID param );

public:
	Listener(UINT16 port);
	~Listener(void);

	bool			Init();

private:
	void			OnConnect();

private:
	
	UINT16			m_port;
	SOCKET			m_sock;
	HANDLE			m_event;

	Network*		m_network;

};

