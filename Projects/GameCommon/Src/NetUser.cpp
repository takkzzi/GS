#include "pch.h"
#include "NetUser.h"
#include "GamePacketBase.h"
#include "Level.h"
#include "Player.h"



using namespace Game;



NetUser::NetUser()
	: mSession(NULL)
	, mUserState(USERSTATE_NONE)
	, mLevel(NULL)
	, mPlayer(NULL)
{
}

NetUser::~NetUser()
{
	if ( ! IsDestroyed() )
		Destroy();
}

void NetUser::Init(TcpSession* session)
{
	mSession = session;
	mUserState = USERSTATE_CONNECTED;

	EnterGame(NULL);	//TEST
}

void NetUser::Destroy()
{
	if ( IsDestroyed() )
		return;

	QuitGame();		//TEST

	ResetData();
}

void NetUser::ResetData()
{
	//Reset User Data;
	mSession = NULL;
	mUserState = USERSTATE_NONE;
}

GamePacketBase* NetUser::GetRecvPacket()
{
	return DoPacketize();
}

bool NetUser::ClearRecvPacket(UINT buffSize)
{
	ASSERT(mSession);
	return mSession->ClearRecvBuffer(buffSize);
}

//Packetizing from RecvBuffer
GamePacketBase* NetUser::DoPacketize()
{
	ASSERT(mSession);
	GamePacketBase* basePacket = NULL;
	char* baseData = mSession->ReadRecvBuffer(sizeof(GamePacketBase));

	if ( baseData ) {
		basePacket = (GamePacketBase*)baseData;
		char* entireData = mSession->ReadRecvBuffer(basePacket->mSize);
		if ( entireData ) {
			return (GamePacketBase*)entireData;
		}
	}
	return NULL;
}


void NetUser::EnterGame(Level* level)
{
	ASSERT(mPlayer == NULL);

	mUserState = USERSTATE_GAME;
	mPlayer = LevelActorFactory::CreatePlayer(NULL);

	SetLevel(level);
}

void NetUser::QuitGame()
{
	mUserState = USERSTATE_NONE;

	if ( mPlayer ) {
		mPlayer->Destroy();
		mPlayer = NULL;
	}
}

void NetUser::SetLevel(Level* level)
{
	mLevel = level;
}

bool NetUser::SendData(char* data, int dataSize)
{
	return mSession->WriteToSend(data, dataSize);
}