#include "pch.h"
#include "GameClient.h"
#include "GameNetworker.h"


#define			RESERVE_USER_COUNT		1
#define			MAX_USER_COUNT			2
#define			BUFFER_SIZE				1024

#define			PORT					42999

using namespace Game;

GameClient::GameClient()
{
}

GameClient::~GameClient()
{
}

void GameClient::Init()
{
	GameCommon::Init();

	mGameNetworker->Init(RESERVE_USER_COUNT, MAX_USER_COUNT, BUFFER_SIZE);
}