#pragma once

#include "atlcoll.h"

namespace Game
{
	class StaticObject;
	class BreakableObject;
	class Trigger;
	class Player;


	class LevelActorFactory
	{
	public:
		LevelActorFactory()		{};
		~LevelActorFactory()	{};


	public:

		static	Actor*				CreateActor(ActorType actorType);
		static	StaticObject*		CreateStaticObject();
		static	BreakableObject*	CreateBreakable();
		static	Trigger*			CreateTrigger();
		static	Player*				CreatePlayer();

		static	UINT				GetActorSubId(Actor* actor);

	protected:
		static	UINT64				CreateActorId(ActorType actorType);

	protected:
		static	UINT64				mActorIdSeq[ACTOR_MAX];

	protected:

	};


	class Level
	{
	public:
		Level();
		virtual	~Level();

	public :
		void					Update(float dt);

		Actor*					GetActor(ActorType type, UINT64 actorId);
		Player*					GetPlayer(UINT index);

	//Actor Events
		void					OnCreateActor(Actor* actor);
		void					OnDestroyActor(Actor* actor);

	protected :

		CAtlMap<UINT64, Actor*>			mActorMap;
		std::vector<StaticObject*>		mStaticVec;
		std::vector<BreakableObject*>	mBreakableVec;
		std::vector<Trigger*>			mTriggerVec;
		std::vector<Player*>			mPlayerVec;
		
	protected:
		//Terrain*						mTerrain;
	};
}
