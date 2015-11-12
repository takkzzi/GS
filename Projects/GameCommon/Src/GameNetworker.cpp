#include "pch.h"
#include "GameNetworker.h"
#include "GamePacketReader.h"
#include "Player.h"
#include "NetUserManager.h"


using namespace Game;




GameNetworker::GameNetworker()
	: mbInit(false)
	, mTcpNetworker(NULL)
	, mUserManager(NULL)
	, mPacketReader(NULL)
{
}

GameNetworker::~GameNetworker(void)
{
	SAFE_DELETE(mPacketReader);
	SAFE_DELETE(mUserManager);
	SAFE_DELETE(mTcpNetworker);
}

void GameNetworker::Init(int reserveSessionCount, int maxUserCount, int bufferSize)
{
	if (mbInit) return;

	mbInit = true;

	mUserManager = new NetUserManager(reserveSessionCount, maxUserCount);
	mTcpNetworker = new TcpNetworker(true, 0, reserveSessionCount, maxUserCount, bufferSize, bufferSize);

	mPacketReader = new GamePacketReader();
	mPacketReader->Init();
}

void GameNetworker::BeginListen(UINT16 port)
{
	if (!mbInit) return;

	mTcpNetworker->BeginListen(port, true);
}

void GameNetworker::Update(float dt)
{
	if (!mTcpNetworker)
		return;

	//mTcpNetworker->UpdateSessions();

	for(int i = 0, n = mTcpNetworker->GetSessionCount(); i < n; ++i)
	{
		Network::TcpSession* sess = mTcpNetworker->GetSession(i);
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
