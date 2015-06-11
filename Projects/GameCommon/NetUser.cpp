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
	mLevel = TheGame->GetLevel();
}

NetUser::~NetUser()
{
	if ( ! IsDestroyed() )
		Destroy();
}

void NetUser::Init(Session* session)
{
	//Logger::Log("NetUser", "UserSession Init() [id:%d]", session->GetId());

	mSession = session;
	mUserState = USERSTATE_CONNECTED;

	//TEST
	EnterGame();
}

void NetUser::Destroy()
{
	if ( IsDestroyed() )
		return;

	//Logger::Log("NetUser", "UserSession Destroy() [id:%d]", mSession->GetId());

	//TEST
	QuitGame();

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
	return DoPacketize(sizeof(GamePacketBase));
}

bool NetUser::ClearRecvPacket(UINT buffSize)
{
	ASSERT(mSession);
	return mSession->ClearRecvBuffer(buffSize);
}

//Packetizing from RecvBuffer
GamePacketBase* NetUser::DoPacketize(UINT packetMinSize)
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


void NetUser::EnterGame()
{
	ASSERT(mPlayer == NULL);

	mUserState = USERSTATE_GAME;
	mPlayer = LevelActorFactory::CreatePlayer();
}

void NetUser::QuitGame()
{
	mUserState = USERSTATE_NONE;

	if ( mPlayer ) {
		mPlayer->Destroy();
		mPlayer = NULL;
	}
}
