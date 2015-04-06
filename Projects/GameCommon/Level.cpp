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

	/*
#ifdef APP_SERVER
	mActorMap.InitHashTable(100000);
	mPlayerVec.reserve(5000);
#else 
	mActorMap.InitHashTable(95000);
	mPlayerVec.reserve(500);	
#endif
	*/
	mActorMap.InitHashTable(100000);
	mPlayerVec.reserve(5000);
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
			POSITION nextPos = pos;
			mActorMap.GetNext(nextPos);
			mActorMap.RemoveAtPos(pos);
			pos = nextPos;
			delete actor;
			continue;
		}

		actor->Update(dt);
		mActorMap.GetNext(pos);
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
		{
			Player* player = (Player*)actor;
			bool bInserted = false;

			for (UINT i = 0, n = mPlayerVec.size(); i < n; ++i) {
				if ( ! mPlayerVec[i] ) {
					bInserted = true;
					mPlayerVec[i] = player;
					player->SetPlayerIndex(i);
					break;
				}
			}

			if ( ! bInserted ) {
				player->SetPlayerIndex(mPlayerVec.size());
				mPlayerVec.push_back(player);
			}
		
			break;
		}
	default : 
		ASSERT(0);
		break;
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
		{
			Player* player = (Player*)actor;
			UINT index = player->GetPlayerIndex();
			ASSERT(mPlayerVec[index]);
			mPlayerVec[index] = NULL;
			break;
		}
	default : 
		ASSERT(0);
		break;
	};

	actor->OnDestroy();
}

Player*	Level::GetPlayer(UINT playerIndex)
{
	Player* player = NULL;
	if ( playerIndex < mPlayerVec.size() )
	{
		player = mPlayerVec[playerIndex];
	}

	return player;
}

