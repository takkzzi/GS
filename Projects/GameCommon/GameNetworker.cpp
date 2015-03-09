#include "pch.h"
#include "GameNetworker.h"
#include "PacketProcessor.h"


using namespace Game;

#define		DEFAULT_USER_COUNT		1000
#define		MAX_USER_COUNT			5000
#define		SEND_BUFFER_SIZE		10240
#define		RECV_BUFFER_SIZE		10240

#define		PORT					42999
#define		RECV_BUFFER_SIZE		10240



GameNetworker::GameNetworker(void)
{
	mIocpNetworker = new Networker(true, 0, DEFAULT_USER_COUNT, MAX_USER_COUNT, SEND_BUFFER_SIZE, RECV_BUFFER_SIZE);
	mIocpNetworker->BeginListen(PORT, true);
	
	mPackeProcessor = new PacketProcessor();
	mPackeProcessor->Init();
}


GameNetworker::~GameNetworker(void)
{
}

void GameNetworker::Update(float dt)
{
	for(int i = 0, n = mIocpNetworker->GetSessionCount(); i < n; ++i) 
	{
		Network::Session* se = mIocpNetworker->GetSession(i);
		if ( se && se->IsState(SESSIONSTATE_CONNECTED) ) 
		{
			mPackeProcessor->Process(se);
		}
	}
}
