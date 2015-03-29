#include "pch.h"
#include "Level.h"
#include "StaticObject.h"
#include "BreakableObject.h"
#include "Trigger.h"
#include "Player.h"



using namespace Game;

#define		ACTORTYPE_BIT_POS		48

UINT64	LevelActorFactory::mActorIdSeq[ACTOR_MAX] = {0, 0, 0, 0, 0};


UINT64 LevelActorFactory::CreateActorId(ActorType actorType)
{
	UINT64 typePart = (UINT64)actorType << ACTORTYPE_BIT_POS;
	UINT64 subPart = mActorIdSeq[actorType]++;
	return (typePart | subPart);
}

UINT LevelActorFactory::GetActorSubId(Actor* actor)
{
	return (UINT)( actor->GetId() % ((UINT64)1 << ACTORTYPE_BIT_POS));
}

Actor* LevelActorFactory::CreateActor(ActorType actorType)
{
	Actor* actor = NULL;

	switch (actorType) {
	case ACTOR_STATIC : 
		actor = new StaticObject();
		break;
	case ACTOR_BREAKABLE :
		actor = new BreakableObject();
		break;
	case ACTOR_TRIGGER :
		actor = new Trigger();
		break;
	case ACTOR_PLAYER :
		actor = new Player();
		break;
	default : 
		ASSERT(0);
	}
	actor->mActorId = CreateActorId(actorType);
	TheGame->GetLevel()->OnCreateActor(actor);
	return actor;
}

StaticObject* LevelActorFactory::CreateStaticObject()
{
	return (StaticObject*)CreateActor(ACTOR_STATIC);
}

BreakableObject* LevelActorFactory::CreateBreakable()
{
	return (BreakableObject*)CreateActor(ACTOR_BREAKABLE);
}

Trigger* LevelActorFactory::CreateTrigger()
{
	return (Trigger*)CreateActor(ACTOR_TRIGGER);
}

Player* LevelActorFactory::CreatePlayer()
{
	return (Player*)CreateActor(ACTOR_PLAYER);
}


Level::Level()
{
	mStaticVec.reserve(1000);
	mBreakableVec.reserve(1000);
	mTriggerVec.reserve(1000);

#ifdef APP_SERVER
	mActorMap.InitHashTable(100000);
	mPlayerVec.reserve(5000);
#else 
	mActorMap.InitHashTable(95000);
	mPlayerVec.reserve(500);	
#endif
}

Level::~Level()
{
}

void Level::Update(float dt)
{
	//mTerrain->Update(dt);

	POSITION pos = mActorMap.GetStartPosition();
	while(pos) {
		Actor* actor = mActorMap.GetValueAt(pos);
		if ( actor->UpdateDestroy(dt) ) {
			OnDestroyActor(actor);
			POSITION nextPos = mActorMap.GetNext(pos);
			mActorMap.RemoveAtPos(pos);
			pos = nextPos;
			delete actor;
			continue;
		}

		actor->Update(dt);
	}
}

void Level::OnCreateActor(Actor* actor)
{
	UINT subId = LevelActorFactory::GetActorSubId(actor);

	switch (actor->mActorType) {
	case ACTOR_STATIC : 
		break;
	case ACTOR_BREAKABLE :
		break;
	case ACTOR_TRIGGER :
		break;
	case ACTOR_PLAYER :
		
		Player* player = (Player*)actor;

		ASSERT(player->GetPlayerIndex() <= mPlayerVec.size());

		if ( player->GetPlayerIndex() < mPlayerVec.size() ) {
			ASSERT(mPlayerVec[player->GetPlayerIndex() == NULL]);
			mPlayerVec[player->GetPlayerIndex()] = player;
		}
		else {
			mPlayerVec.push_back(player);
		}
		
		break;
	default : 
		
		ASSERT(0);
	}

	mActorMap.SetAt(actor->GetId(), actor);

	actor->mLevel = this;
	actor->OnCreate();
}

void Level::OnDestroyActor(Actor* actor)
{
	switch (actor->mActorType) {
	case ACTOR_STATIC : 
		break;
	case ACTOR_BREAKABLE :
		break;
	case ACTOR_TRIGGER :
		break;
	case ACTOR_PLAYER :
		Player* player = (Player*)actor;
		UINT index = player->GetPlayerIndex();
		ASSERT(mPlayerVec[index]);
		mPlayerVec[index] = NULL;
		break;
	default : 
		ASSERT(0);
	}

	actor->OnDestroy();
}

Player*	Level::GetPlayer(UINT playerIndex, bool bCreate)
{
	Player* player = NULL;
	if ( playerIndex < mPlayerVec.size() )
	{
		player = mPlayerVec[playerIndex];
	}

	if ( ! player && bCreate )
	{
		if ( playerIndex = mPlayerVec.size()) {
			player = LevelActorFactory::CreatePlayer();
			player->SetPlayerIndex(playerIndex);
		}
		else if ( ! mPlayerVec[playerIndex] ) {
			player = LevelActorFactory::CreatePlayer();
			player->SetPlayerIndex(playerIndex);
		}
		else
			ASSERT(0);
	}

	return player;
}

