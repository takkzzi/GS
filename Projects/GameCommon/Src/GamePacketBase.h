#pragma once

#include "GamePacketProtocol.h"

#pragma pack (1)

namespace Game
{
	//Header
	struct GamePacketBase
	{
		GamePacketBase() : mSize(sizeof(GamePacketBase)), mType(GamePacketProtocol::PT_Base) {
		}

		USHORT		mSize;
		USHORT		mType;
		//TODO :
		//USHORT	mUserId;
		//USHORT	mCRC;
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
#define		MAX_CHAT_MSG			512

	struct ChatMsg : GamePacketBase 
	{
		널 터미네이터를 잘 붙여줘야 함 !!!
		ChatMsg(TCHAR* msgStr) {
			mType = GamePacketProtocol::PT_ChatMsg;
			mChatSize = min( MAX_CHAT_MSG, (USHORT)(_tcslen(msgStr) * sizeof(TCHAR)) );
			memcpy(mChatData, msgStr, mChatSize);
			mChatData[mChatSize/sizeof(TCHAR)] = _T('\0');// NULL;

			mSize += sizeof(mChatSize) + mChatSize;
		}

		USHORT	mChatSize;	
		TCHAR	mChatData[MAX_CHAT_MSG + 1];
	};


}

#pragma pack ()
