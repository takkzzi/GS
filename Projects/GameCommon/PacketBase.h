#pragma once

#include "PacketProtocol.h"



namespace Game
{
#pragma pack (1)
	struct PacketBase
	{
		PacketBase() : mSize(sizeof(PacketBase)), mType(PacketProtocol::PT_Base) {
		}

		USHORT		mSize;
		USHORT		mType;
	};


	struct AlphabetPacket : public PacketBase 
	{
		AlphabetPacket() {
			mSize = sizeof(AlphabetPacket);
			mType = PacketProtocol::PT_Alphabet;

			memcpy(mData, ("ABCDEFGHIJKLMNOPQRSTUVWXYZ"), sizeof(mData));
		}

		char	mData[27];
	};

	struct ChatMsg : PacketBase 
	{
		ChatMsg() {
			mType = PacketProtocol::PT_ChatMsg;
		}

		USHORT	mChatSize;	
		TCHAR	mChatData[27];
	};

#pragma pack ()
}

