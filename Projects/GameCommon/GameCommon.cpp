#include "pch.h"
#include "GameCommon.h"
#include "GameNetworker.h"


using namespace Game;

GameCommon::GameCommon()
	: mGameNetworker(NULL)
{
}

GameCommon::~GameCommon()
{
}

void GameCommon::Init()
{
	if ( mbInit )
		return;

	CoreSystem::Init(_T("GameLog"));
	NetworkSystem::Init();

	mGameNetworker = new GameNetworker();

	mAppRuntime = Core::Time::GetAppTime();
}

void GameCommon::Shutdown()
{
	if ( ! mbInit)
		return;

	SAFE_DELETE(mGameNetworker);
}

void  GameCommon::MainLoop()
{
	double currentTime = Core::Time::GetAppTime();
	float dt = (float)(currentTime - mAppRuntime);

	mGameNetworker->Update(dt);

	mAppRuntime = currentTime;
}
