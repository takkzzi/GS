#include "pch.h"
#include "PacketProcessor.h"


using namespace Game;

void PacketProcessor::BindHanlder()
{
	ZeroMemory(mHandlerArray, sizeof(mHandlerArray));
	mHandlerArray[PT_Alphabet] = &PacketProcessor::Alphabet;
	mHandlerArray[PT_ChatMsg] = &PacketProcessor::ChatMsg;
}

bool PacketProcessor::CallHandler(PacketBase* packet)
{
	int protocol = (int)(packet->mType);
	if ( mHandlerArray[protocol] ) {
		(this->*(this->mHandlerArray[protocol])) ( (char*)packet, (int)packet->mSize );
		return true;
	}

	LOG_ERROR_A("Pakcket Handelr Missing !!!");
	return false;
}

void PacketProcessor::Alphabet(char* data, int size)
{
	
}

void PacketProcessor::ChatMsg(char* data, int size)
{
}

