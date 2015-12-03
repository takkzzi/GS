#pragma once


class Client : public SessionEventObject
{
public:
	Client(TcpSession* session);
	~Client();

	bool			Connect(const CHAR* addr, const int port);
	void			Disconnect();
	void			Update();

public:
	void			SendPacket();
	void			RecvPacket();

	virtual	void	OnDisconnect();

protected:
	void*			GetPacket();

protected:

	TcpSession*			mSession;
	UINT64				mPacketSequence;

	float				mSentElapsedTime;
};



class ClientSimulator : public Thread
{
public:

	ClientSimulator();
	virtual ~ClientSimulator();

	virtual bool Begin(bool bSuspend = false);
	virtual bool End();

	virtual		DWORD	ThreadTick();
	void				RunSimulation();

	static void			Update();

protected:
	TcpNetworker*					mIocp;
	std::vector<Client*>			mClients;

	
	double							mPrevAppTime;

public:
	static	volatile	UINT64		msRecvedData;
};

