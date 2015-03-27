#include "pch.h"
#include "Level.h"
#include "StaticObject.h"
#include "BreakableObject.h"
#include "Trigger.h"
#include "Player.h"



using namespace Game;

#define		ACTORTYPE_BIT_POS		48

UINT	LevelActorFactory::mActorIdSeq[ACTOR_MAX] = {0, 0, 0, 0, 0};


UINT LevelActorFactory::CreateActorId(ActorType actorType)
{
	UINT typePart = actorType << ACTORTYPE_BIT_POS;
	UINT subPart = mActorIdSeq[actorType]++;
	return (typePart | subPart);
}

UINT LevelActorFactory::GetActorSubId(Actor* actor)
{
	return ( actor->GetId() % (1 << ACTORTYPE_BIT_POS));
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
		ASSERT(subId >= 0 && subId < mPlayerVec.size());
		mPlayerVec[((Player*)actor)->GetPlayerIndex()] = NULL;
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
	UINT subId = LevelActorFactory::GetActorSubId(actor);

	switch (actor->mActorType) {
	case ACTOR_STATIC : 
		break;
	case ACTOR_BREAKABLE :
		break;
	case ACTOR_TRIGGER :
		break;
	case ACTOR_PLAYER :
		ASSERT(subId >= 0 && subId < mPlayerVec.size());
		mPlayerVec[subId] = NULL;
		break;
	default : 
		ASSERT(0);
	}
}

Player*	Level::GetPlayer(UINT playerIndex, bool bCreate)
{
	Pending 
	if ( bCreate ) {
		if ( playerIndex >= mPlayerVec.size()) {
			Player* newPlayer = LevelActorFactory::CreatePlayer();
			newPlayer->SetPlayerIndex(playerIndex);
			return newPlayer;
		}
		else if ( ! mPlayerVec[playerIndex] ) {
			Player* newPlayer = LevelActorFactory::CreatePlayer();
			mPlayerVec[playerIndex] = newPlayer;
			newPlayer->SetPlayerIndex(playerIndex);
			return newPlayer;
		}
	}

	return mPlayerVec[playerIndex];
}

