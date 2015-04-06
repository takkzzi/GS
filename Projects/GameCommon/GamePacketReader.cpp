#include "pch.h"
#include "GamePacketReader.h"
#include "GamePacketBase.h"
//#include "Network\Session.h"

using namespace Game;



GamePacketReader::GamePacketReader(void)
{
}

GamePacketReader::~GamePacketReader(void)
{
}

void GamePacketReader::Init()
{
	BindHanlder();
}
