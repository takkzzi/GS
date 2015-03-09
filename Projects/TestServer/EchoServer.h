#pragma once
#include "Core/Thread.h"



class EchoServer : public Thread 
{
public :

	EchoServer(int sessionCount, int sessionLimit);
	virtual ~EchoServer();

	bool				Begin(int port);

	virtual bool		End();
	virtual DWORD		ThreadTick();

	void				DoEcho();
	void				SessionLog(Session* sess, char* msg, size_t size );

protected :
	Networker*					mIocp;
	std::vector<Session*>		mSessions;

	double						mSendTime;
};