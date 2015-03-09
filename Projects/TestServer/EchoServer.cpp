#include "stdafx.h"
#include "EchoServer.h"



EchoServer::EchoServer(int sessionCount, int sessionLimit)
{
	mIocp = new Networker(true, 5, sessionCount, sessionLimit, 1024, 19);

	mSendTime = Time::GetAppTime();
}

EchoServer::~EchoServer() 
{
	SAFE_DELETE(mIocp);
}

bool EchoServer::Begin(int port)
{
	bool bRes = mIocp->BeginListen(port, true);
	if ( bRes ) {
		return __super::Begin(false);
	}
	return false;
}

bool EchoServer::End()
{
	return __super::End();
}

DWORD EchoServer::ThreadTick()
{
	if ( IsState(THREAD_END) )
		return 0;

	if ( ! mIocp->IsThreadUpdatingSessions() )
		mIocp->UpdateSessions();

	DoEcho();

	return 1;
}

void EchoServer::DoEcho()
{
	/*
	for(int i = 0; i < mIocp->GetSessionCount(); ++i) {
		Session* se = mIocp->GetSession(i);
		if ( se && se->IsState(SESSIONSTATE_CONNECTED) ) 
		{
			while( PacketBase* packet = se->ReadData() )	
			{	
				ASSERT( packet->mPacketSize == sizeof(AlphabetPacket) );

				AlphabetPacket* alpha = (AlphabetPacket*)packet;
				se->WriteSendBuffer((char*)(packet), packet->mPacketSize);	//Echo

				se->ClearRecvBuffer(sizeof(AlphabetPacket));
			}
		}
	}
	*/
}

void EchoServer::SessionLog(Session* sess, char* msg, size_t size ) 
{
	char totalMsg[2048];

	memcpy(totalMsg, msg, size);
	totalMsg[size] = '\0';

	sprintf_s(totalMsg, "%s\n", totalMsg);
	//OutputDebugStringA(totalMsg);
}