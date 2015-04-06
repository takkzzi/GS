#include "pch.h"
#include "UserSession.h"
#include "GamePacketBase.h"
#include "Level.h"
#include "Player.h"



using namespace Game;



UserSession::UserSession()
	: mSession(NULL)
	, mUserState(USERSTATE_NONE)
	, mLevel(NULL)
	, mPlayer(NULL)
{
	mLevel = TheGame->GetLevel();
}

UserSession::~UserSession()
{
	if ( ! IsDestroyed() )
		Destroy();
}

void UserSession::Init(Session* session)
{
	Logger::Log("UserSession", "UserSession Init() [id:%d]", session->GetId());

	mSession = session;
	mUserState = USERSTATE_CONNECTED;

	//TEST
	EnterGame();
}

void UserSession::Destroy()
{
	if ( IsDestroyed() )
		return;

	Logger::Log("UserSession", "UserSession Destroy() [id:%d]", mSession->GetId());

	//TEST
	QuitGame();

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


void UserSession::EnterGame()
{
	ASSERT(mPlayer == NULL);

	mUserState = USERSTATE_GAME;
	mPlayer = LevelActorFactory::CreatePlayer();
}

void UserSession::QuitGame()
{
	mUserState = USERSTATE_NONE;

	if ( mPlayer ) {
		mPlayer->Destroy();
		mPlayer = NULL;
	}
}
