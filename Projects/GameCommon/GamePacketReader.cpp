#include "pch.h"
#include "GamePacketReader.h"
#include "GamePacketBase.h"
//#include "Network\Session.h"

using namespace Game;


/*
void LogPacket(char* prefix, AlphabetPacket* packet)
{
	ASSERT(sizeof(*packet) == sizeof(AlphabetPacket));

	char categ[64];
	sprintf_s(categ, "[%d]%s", mId, prefix );

	char logMsg[32];
	size_t msgSize = sizeof(packet->mData);
	memcpy(logMsg, packet->mData, msgSize);
	logMsg[msgSize] = '\0';

	Logger::Log(categ, "%s %s(Num:%d) (Head%d, Tail%d)", categ, logMsg, packet->mPacketId, mRecvBuffer.GetDataHeadPos(), mRecvBuffer.GetDataTailPos());
}
*/

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

bool GamePacketReader::ReadPacket(Network::Session* session)
{
	if ( ! session || ! session->IsState(SESSIONSTATE_CONNECTED) )
		return false;

	//Packetizing from RecvBuffer & Packet Processing
	GamePacketBase* basePacket = NULL;
	char* baseData = session->ReadRecvBuffer(sizeof(GamePacketBase));
	if ( baseData ) {
		basePacket = (GamePacketBase*)baseData;
		char* entireData = session->ReadRecvBuffer(basePacket->mSize);
		if ( entireData ) 
		{
			CallHandler(basePacket);

			session->ClearRecvBuffer(basePacket->mSize);
			return true;
		}
	}

	return false;
}



