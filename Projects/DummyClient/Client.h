#pragma once


class Client
{
public:
	Client(TcpSession* session);
	~Client();

	bool			Connect(const CHAR* addr, const int port);
	void			Disconnect();
	void			Update();

//protected:
	void			SendPacket();
	void			RecvPacket();

	void*			GetChatPacket();

protected:

	TcpSession*			mSession;
};



class ClientSimulator : public Thread
{
public:

	ClientSimulator(int clientCount);
	virtual ~ClientSimulator();

	virtual bool Begin(bool bSuspend = false);
	virtual bool End();

	virtual		DWORD	ThreadTick();
	void				RunSimulation();




protected:
	TcpNetworker*					mIocp;
	std::vector<Client*>			mClients;


	double							mLastSendTime;
};

