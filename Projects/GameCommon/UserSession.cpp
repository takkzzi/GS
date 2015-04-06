#include "pch.h"
#include "UserSession.h"
#include "GamePacketBase.h"


using namespace Game;

UserSession::UserSession()
	: mSession(NULL)
	, mUserState(USERSTATE_NONE)
{
}

UserSession::~UserSession()
{
}

void UserSession::Init(Session* session)
{
	mSession = session;
}

void UserSession::Destroy()
{
	ResetData();
}

void UserSession::ResetData()
{
	//Reset User Data;
	mSession = NULL;
	mUserState = USERSTATE_NONE;
}

GamePacketBase* UserSession::GetRecvPacket()
{
	return DoPacketize(sizeof(GamePacketBase));
}

bool UserSession::ClearRecvPacket(UINT buffSize)
{
	ASSERT(mSession);
	return mSession->ClearRecvBuffer(buffSize);
}

//Packetizing from RecvBuffer
GamePacketBase* UserSession::DoPacketize(UINT packetMinSize)
{
	ASSERT(mSession);
	GamePacketBase* basePacket = NULL;
	char* baseData = mSession->ReadRecvBuffer(packetMinSize);
	if ( baseData ) {
		basePacket = (GamePacketBase*)baseData;
		char* entireData = mSession->ReadRecvBuffer(basePacket->mSize);
		if ( entireData ) {
			return basePacket;
		}
	}
	return NULL;
}