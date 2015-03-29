#include "pch.h"
#include "GameNetworker.h"
#include "GamePacketReader.h"
#include "Player.h"
#include "Level.h"

using namespace Game;

#define			RESERVE_USER_COUNT		1000
#define			MAX_USER_COUNT			5000
#define			SEND_BUFFER_SIZE		10240
#define			RECV_BUFFER_SIZE		10240

#define			PORT					42999
#define			RECV_BUFFER_SIZE		10240


#define			APP_SERVER



GameNetworker::GameNetworker()
{
	mIocpNetworker = new Networker(true, 0, RESERVE_USER_COUNT, MAX_USER_COUNT, SEND_BUFFER_SIZE, RECV_BUFFER_SIZE);
	
	//TODO : Only Server-Side
#ifdef APP_SERVER
	mIocpNetworker->BeginListen(PORT, true);
#endif
	
	mPackeReader = new GamePacketReader();
	mPackeReader->Init();

	mLevel = TheGame->GetLevel();
}

GameNetworker::~GameNetworker(void)
{
	SAFE_DELETE(mPackeReader);
	SAFE_DELETE(mIocpNetworker);
}

void GameNetworker::Update(float dt)
{
	for(int i = 0, n = mIocpNetworker->GetSessionCount(); i < n; ++i) 
	{
		Network::Session* sess = mIocpNetworker->GetSession(i);
		if ( ! sess )
			continue;

		//Connected
		if ( sess->IsState(SESSIONSTATE_CONNECTED) ) {
			Player* player  = mLevel->GetPlayer(i, true);
			ASSERT(player);
			mPackeReader->ReadPacket(sess);
		}

		//Disconnected -> Delete
		else if ( Player* player = mLevel->GetPlayer(i, false) ) {
			player->Destroy();
		}
	}
}
