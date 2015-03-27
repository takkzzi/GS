#pragma once

#include "GamePacketProtocol.h"



namespace Game
{
#pragma pack (1)
	struct GamePacketBase
	{
		GamePacketBase() : mSize(sizeof(GamePacketBase)), mType(GamePacketProtocol::PT_Base) {
		}

		USHORT		mSize;
		USHORT		mType;
	};


	struct AlphabetPacket : public GamePacketBase 
	{
		AlphabetPacket() {
			mSize = sizeof(AlphabetPacket);
			mType = GamePacketProtocol::PT_Alphabet;

			memcpy(mData, ("ABCDEFGHIJKLMNOPQRSTUVWXYZ"), sizeof(mData));
		}

		char	mData[27];
	};

	struct ChatMsg : GamePacketBase 
	{
		ChatMsg() {
			mType = GamePacketProtocol::PT_ChatMsg;
		}

		USHORT	mChatSize;	
		TCHAR	mChatData[27];
	};

#pragma pack ()
}

