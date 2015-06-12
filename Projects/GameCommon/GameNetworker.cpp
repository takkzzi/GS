#include "pch.h"
#include "GameNetworker.h"
#include "GamePacketReader.h"
#include "Player.h"
#include "NetUserManager.h"


using namespace Game;




GameNetworker::GameNetworker()
	: mIocpNetworker(NULL)
	, mUserManager(NULL)
	, mPacketReader(NULL)
{
}

GameNetworker::~GameNetworker(void)
{
	SAFE_DELETE(mPacketReader);
	SAFE_DELETE(mUserManager);
	SAFE_DELETE(mIocpNetworker);
}

void GameNetworker::Init(int reservUserCount, int maxUserCount, int bufferSize)
{
	mIocpNetworker = new Networker(true, 0, reservUserCount, maxUserCount, bufferSize, bufferSize);
	mUserManager = new NetUserManager(reservUserCount, maxUserCount);

	mPacketReader = new GamePacketReader();
	mPacketReader->Init();
}

void GameNetworker::ServerStart(UINT16 port)
{
	mIocpNetworker->BeginListen(port, true);
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
			mPacketReader->ProcessUserPacket(user);
		}

		//Disconnected
		else if ( NetUser* user = mUserManager->GetNetUser(i) ) {
			if ( ! user->IsDestroyed() )
				user->Destroy();
		}
	}
}
