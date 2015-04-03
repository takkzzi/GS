#include "pch.h"
#include "GamePacketReader.h"


using namespace Game;

void GamePacketReader::BindHanlder()
{
	ZeroMemory(mHandlerArray, sizeof(mHandlerArray));
	mHandlerArray[PT_Alphabet] = &GamePacketReader::Alphabet;
	mHandlerArray[PT_ChatMsg] = &GamePacketReader::ChatMsg;
}

bool GamePacketReader::CallHandler(GamePacketBase* packet)
{
	if ( ! packet )
		return false;

	int protocol = (int)(packet->mType);
	if ( mHandlerArray[protocol] ) { //Call Binded Func
		(this->*(this->mHandlerArray[protocol])) ( (char*)packet, (int)packet->mSize );
		return true;
	}

	LOG_ERROR_A("Pakcket Handelr Missing !!!");
	return false;
}

void GamePacketReader::Alphabet(char* data, int size)
{
	
}

void GamePacketReader::ChatMsg(char* data, int size)
{
}

