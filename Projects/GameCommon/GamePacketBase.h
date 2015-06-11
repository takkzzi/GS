#pragma once

#include "GamePacketProtocol.h"

#pragma pack (1)

namespace Game
{

	struct GamePacketBase
	{
		GamePacketBase() : mSize(sizeof(GamePacketBase)), mType(GamePacketProtocol::PT_Base) {
		}

		USHORT		mSize;
		USHORT		mType;
	};

	//TEST
	struct AlphabetPacket : public GamePacketBase 
	{
		AlphabetPacket() {
			mSize = sizeof(AlphabetPacket);
			mType = GamePacketProtocol::PT_Alphabet;

			memcpy(mData, ("ABCDEFGHIJKLMNOPQRSTUVWXYZ"), sizeof(mData));
		}

		char	mData[27];
	};

	//TEST
#define		MAX_CHAT_MSG			100

	struct ChatMsg : GamePacketBase 
	{
		ChatMsg(TCHAR* msgStr) {
			mType = GamePacketProtocol::PT_ChatMsg;
			mChatSize = min( MAX_CHAT_MSG, (USHORT)(_tcslen(msgStr) * sizeof(TCHAR)) );
			memcpy(mChatData, msgStr, mChatSize);
			mChatData[mChatSize] = NULL;
		}

		USHORT	mChatSize;	
		TCHAR	mChatData[MAX_CHAT_MSG + 1];
	};


}

#pragma pack ()
