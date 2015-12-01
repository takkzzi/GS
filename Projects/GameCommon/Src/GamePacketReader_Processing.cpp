#include "pch.h"
#include "GamePacketReader.h"
#include "NetUser.h"
#include "GamePacketBase.h"


using namespace Game;

void GamePacketReader::BindHanlder()
{
	ZeroMemory(mHandlerArray, sizeof(mHandlerArray));
	mHandlerArray[PT_Alphabet] = &GamePacketReader::Alphabet;
	mHandlerArray[PT_ChatMsg] = &GamePacketReader::ChatMsg;
}

bool GamePacketReader::ProcessUserPacket(NetUser* user)
{
	if ( ! user )
		return false;

	GamePacketBase* packet = user->GetRecvPacket();
	if ( ! packet )
		return false;

	char* data = (char*)packet;

	USHORT protocol = packet->mType;
	if ( mHandlerArray[protocol] ) { 
		(this->*(this->mHandlerArray[protocol])) (user, data, (int)packet->mSize);	//Call Binded Func
		user->ClearRecvPacket(packet->mSize);
		return true;
	}
	else
		LOG_ERROR_A("Pakcket Handelr Missing !!!");

	return false;
}

void GamePacketReader::Alphabet(NetUser* user, char* data, int size)
{
	Game::AlphabetPacket* alphabet = (Game::AlphabetPacket*)data;
	Logger::LogDebugString(alphabet->mData);
}

void GamePacketReader::ChatMsg(NetUser* user, char* data, int size)
{
	Game::ChatMsg* chatMsg = (Game::ChatMsg*)data;
	Logger::LogDebugString(chatMsg->mChatData);
}

