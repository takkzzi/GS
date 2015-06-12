// TestServer.cpp : Defines the entry point for the application.
//

#include "stdafx.h"
#include "GameServer.h"
#include "GameCommon\GameNetworker.h"

#define			RESERVE_USER_COUNT		10000
#define			MAX_USER_COUNT			30000
#define			BUFFER_SIZE				1024

#define			PORT					42999


using namespace Server;


GameServer::GameServer()
{
}

GameServer::~GameServer()
{
}

void GameServer::Init()
{
	GameCommon::Init();

	mGameNetworker->Init(RESERVE_USER_COUNT, MAX_USER_COUNT, BUFFER_SIZE);
	mGameNetworker->ServerStart(PORT);
}