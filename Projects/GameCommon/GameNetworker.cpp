#include "pch.h"
#include "GameNetworker.h"
#include "GamePacketReader.h"
#include "Player.h"
#include "NetUserManager.h"


using namespace Game;

#define			RESERVE_USER_COUNT		5000
#define			MAX_USER_COUNT			5000
#define			SEND_BUFFER_SIZE		10240
#define			RECV_BUFFER_SIZE		10240

#define			PORT					42999
#define			RECV_BUFFER_SIZE		10240

//TEMP
#define			APP_SERVER



GameNetworker::GameNetworker()
{
	mIocpNetworker = new Networker(true, 0, RESERVE_USER_COUNT, MAX_USER_COUNT, SEND_BUFFER_SIZE, RECV_BUFFER_SIZE);
	mUserManager = new NetUserManager(RESERVE_USER_COUNT, MAX_USER_COUNT);

#ifdef APP_SERVER
	mIocpNetworker->BeginListen(PORT, true);
#endif
	
	mPackeReader = new GamePacketReader();
	mPackeReader->Init();
}

GameNetworker::~GameNetworker(void)
{
	SAFE_DELETE(mPackeReader);
	SAFE_DELETE(mUserManager);
	SAFE_DELETE(mIocpNetworker);
}

void GameNetworker::Update(float dt)
{
	for(int i = 0, n = mIocpNetworker->GetSessionCount(); i < n; ++i) 
	{
		Network::Session* sess = mIocpNetworker->GetSession(i);
		if ( ! sess )
			continue;

		if ( sess->IsState(SESSIONSTATE_CONNECTED) ) {
			NetUser* user = mUserManager->GetNetUser(sess, true);
			mPackeReader->ProcessUserPacket(user);
		}

		//Disconnected
		else if ( NetUser* user = mUserManager->GetNetUser(i) ) {
			if ( ! user->IsDestroyed() )
				user->Destroy();
		}
	}
}
