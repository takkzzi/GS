// TestServer.cpp : Defines the entry point for the application.
//

#include "stdafx.h"
#include "GameServer.h"
#include "GameCommon/GameNetworker.h"


using namespace Game;
using namespace Server;


GameServer::GameServer()
	: mGameNetworker(NULL)
	, mAppRuntime(0.0)
{
	CoreSystem::Init(_T("ServerLog"));
	NetworkSystem::Init();
}

GameServer::~GameServer()
{
	NetworkSystem::Shutdown();
	CoreSystem::Shutdown();
}

#define			RESERVE_USER_COUNT		10000
#define			MAX_USER_COUNT			30000
#define			BUFFER_SIZE				4096

void GameServer::Init()
{
	mGameNetworker = new GameNetworker();
	mGameNetworker->Init(RESERVE_USER_COUNT, MAX_USER_COUNT, BUFFER_SIZE);

	mAppRuntime = Core::Time::GetAppTime();
}

void GameServer::Shutdown()
{
	SAFE_DELETE(mGameNetworker);
}

void GameServer::BeginServer(int port)
{
	mGameNetworker->BeginListen(port);
}

void GameServer::MainLoop()
{
	double currentTime = Core::Time::GetAppTime();
	float dt = (float)(currentTime - mAppRuntime);
	mAppRuntime = currentTime;

	mGameNetworker->Update(dt);
}

