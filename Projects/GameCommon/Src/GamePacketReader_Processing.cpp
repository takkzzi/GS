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

	mRecvDataSize += packet->mSize;

	//Check 'Packet Sequence'
	ASSERT((user->mLastPacketNumber + 1) == packet->mSequence);
	user->mLastPacketNumber = packet->mSequence;

	USHORT protocol = packet->mType;
	if ( mHandlerArray[protocol] ) { 
		(this->*(this->mHandlerArray[protocol])) (user, (char*)packet, (int)packet->mSize);	//Call Binded Func
		bool bCleared = user->ClearRecvPacket(packet->mSize);
		ASSERT(bCleared);
		return true;
	}
	else
		LOG_ERROR_A("Pakcket Handelr Missing !!!");

	return false;
}

void GamePacketReader::Alphabet(NetUser* user, char* data, int size)
{
	Game::AlphabetPacket* alphabet = (Game::AlphabetPacket*)data;
	user->SendData((char*)alphabet, alphabet->mSize);
	//Logger::LogDebugString(alphabet->mData);
}

void GamePacketReader::ChatMsg(NetUser* user, char* data, int size)
{
	Game::ChatMsg* chatMsg = (Game::ChatMsg*)data;
	user->SendData((char*)chatMsg, chatMsg->mSize);
	//Logger::LogDebugString(chatMsg->mChatData);
}

