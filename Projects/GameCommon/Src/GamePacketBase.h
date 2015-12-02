#pragma once

#include "GamePacketProtocol.h"

#pragma pack (1)

namespace Game
{
	//Header
	struct GamePacketBase
	{
		GamePacketBase() 
			: mSize(sizeof(GamePacketBase))
			, mType(GamePacketProtocol::PT_Base) 
			, mSequence(0)
		{
		}

		USHORT		mSize;
		USHORT		mType;
		UINT64		mSequence;
		//TODO:
		//USHORT	mUserId;
		//USHORT	mCRC;
	};

	//TEST
	struct AlphabetPacket : public GamePacketBase 
	{
		AlphabetPacket() {
			mSize = sizeof(AlphabetPacket);
			mType = GamePacketProtocol::PT_Alphabet;

			memcpy(mData, ("ABCDEFGHIJKLMNOPQRSTUVWXYZ\0"), sizeof(mData));
		}
		      
		char	mData[27];
	};

	//TEST
#define		MAX_CHAT_MSG			511

	struct ChatMsg : GamePacketBase 
	{
		ChatMsg(TCHAR* msgStr) {
			mType = GamePacketProtocol::PT_ChatMsg;
			mChatSize = min( MAX_CHAT_MSG, (USHORT)(_tcslen(msgStr) * sizeof(TCHAR)) );
			memcpy(mChatData, msgStr, mChatSize);
			
			//Adding Null Terminator 
			mChatData[mChatSize / sizeof(TCHAR)] = _T('\0'); // NULL; // _T('\0');
			mChatSize += sizeof(TCHAR);
			mSize += sizeof(mChatSize) + mChatSize;		//Not (mSize = sizeof(GamePacketBase)
		}

		USHORT	mChatSize;	
		TCHAR	mChatData[MAX_CHAT_MSG + 1];
	};


}

#pragma pack ()
