#include "pch.h"
#include "GameCommon.h"
#include "GameNetworker.h"
#include "Level.h"

using namespace Game;

GameCommon::GameCommon()
	: mbInit(false)
	, mLevel(NULL)
	, mGameNetworker(NULL)
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

	mLevel = new Level();
	mGameNetworker = new GameNetworker();
	
	mAppRuntime = Core::Time::GetAppTime();

	mbInit = true;
}

void GameCommon::Shutdown()
{
	if ( ! mbInit)
		return;

	SAFE_DELETE(mGameNetworker);
	SAFE_DELETE(mLevel);

	NetworkSystem::Shutdown();
	CoreSystem::Shutdown();

	mbInit = false;
}

void  GameCommon::MainLoop()
{
	double currentTime = Core::Time::GetAppTime();
	float dt = (float)(currentTime - mAppRuntime);

	mGameNetworker->Update(dt);
	mLevel->Update(dt);

	mAppRuntime = currentTime;
}
