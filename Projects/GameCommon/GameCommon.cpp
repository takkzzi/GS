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

//TEST start
#define			RESERVE_USER_COUNT		10000
#define			MAX_USER_COUNT			30000
#define			BUFFER_SIZE				1024

#define			PORT					42999

	mGameNetworker->Init(RESERVE_USER_COUNT, MAX_USER_COUNT, BUFFER_SIZE);
	mGameNetworker->ServerStart(PORT);
	//TEST end
}

void GameCommon::Shutdown()
{
	if ( ! mbInit)
		return;

	SAFE_DELETE(mGameNetworker);
	SAFE_DELETE(mLevel);	

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
