#include "pch.h"
#include "GamePacketReader.h"
#include "GamePacketBase.h"
//#include "Network\Session.h"

using namespace Game;



GamePacketReader::GamePacketReader(void)
	: mRecvDataSize(0)
	, mSentDataSize(0)
	, mElapsedTime(0.0f)
{
}

GamePacketReader::~GamePacketReader(void)
{
}

void GamePacketReader::Init()
{
	BindHanlder();
}

void GamePacketReader::Update(float dt)
{
	mElapsedTime += dt;
	if (mElapsedTime >= 1.0f)
	{
		float kbSize = (float)mRecvDataSize / 1024.f;
		Logger::LogDebugString("Recieved Data : %g KB/S", kbSize);
		mRecvDataSize = 0;
		mElapsedTime = 0.0f;
	}
}
