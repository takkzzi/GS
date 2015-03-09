#include "pch.h"
#include "PacketProcessor.h"
#include "PacketBase.h"
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

PacketProcessor::PacketProcessor(void)
{
}


PacketProcessor::~PacketProcessor(void)
{
}

void PacketProcessor::Init()
{
	BindHanlder();
}

bool PacketProcessor::Process(Network::Session* session)
{
	if ( ! session || ! session->IsState(SESSIONSTATE_CONNECTED) )
		return false;

	//Packetizing from RecvBuffer & Packet Processing
	PacketBase* basePacket = NULL;
	char* baseData = session->ReadRecvBuffer(sizeof(PacketBase));
	if ( baseData ) {
		basePacket = (PacketBase*)baseData;
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



