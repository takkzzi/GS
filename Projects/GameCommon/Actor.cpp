#include "pch.h"
#include "Actor.h"
#include "Level.h"


using namespace Game;

Actor::Actor() 
	: mActorType(ACTOR_NONE)
	, mActorId(0)
	, mLevel(NULL)
	, mDeleteTimer(0.f)
	, mDestroyed(false)
{
}

Actor::~Actor()
{
}

void Actor::Destroy(float time)
{
	if ( time <= 0.f ) {
		DoDestroy();
	}
	else {
		mDeleteTimer = time;
	}
}

void Actor::DoDestroy()
{	
	OnDestroy();
	mDestroyed = true;
}

bool Actor::UpdateDestroy(float dt)
{
	if ( mDeleteTimer > 0.0f ) {
		mDeleteTimer -= dt;
		if ( mDeleteTimer <= 0.f) {
			DoDestroy();
		}
	}
	return mDestroyed;
}

