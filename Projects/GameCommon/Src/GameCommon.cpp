#include "pch.h"
#include "GameCommon.h"
#include "GameNetworker.h"
#include "NetUserManager.h"
#include "Level.h"


using namespace Game;

GameCommon::GameCommon()
	: mbInit(false)
	, mLevel(NULL)
	, mGameNetworker(NULL)
{
	CoreSystem::Init(_T("GameLog"));
	NetworkSystem::Init();

	mLevel = new Level();
	mGameNetworker = new GameNetworker();

	mAppRuntime = Core::Time::GetAppTime();
}

GameCommon::~GameCommon()
{
	NetworkSystem::Shutdown();
	CoreSystem::Shutdown();
}

void GameCommon::Init()
{
	if ( mbInit )
		return;
	
	mbInit = true;
}

void GameCommon::Shutdown()
{
	if (!mbInit)
		return;

	SAFE_DELETE(mGameNetworker);
	SAFE_DELETE(mLevel);	

	mbInit = false;
}

void GameCommon::StartServer(int reserveUserCount, int maxUserCount, int bufferSize, int port) {
	if (!mbInit)
		return;

	//mGameNetworker->StartServer(port);
}

void  GameCommon::MainLoop()
{
	double currentTime = Core::Time::GetAppTime();
	float dt = (float)(currentTime - mAppRuntime);

	mGameNetworker->Update(dt);
	mLevel->Update(dt);

	mAppRuntime = currentTime;
}
