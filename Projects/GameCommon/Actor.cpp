#include "pch.h"
#include "Actor.h"
#include "Level.h"


using namespace Game;

Actor::Actor() 
	: mActorType(ACTOR_NONE)
	, mActorId(0)
	, mDeleteTimer(0.f)
	, mLevel(NULL)
{
}

Actor::~Actor()
{
}

void Actor::Destroy(float time)
{
	if ( time <= 0.f ) {
		DestroyImmediate();
	}
	else {
		mDeleteTimer = time;
	}
}

void Actor::DestroyImmediate()
{
	OnDestroy();
}

bool Actor::UpdateDestroy(float dt)
{
	if ( mDeleteTimer > 0.0f ) {
		mDeleteTimer -= dt;
		if ( mDeleteTimer <= 0.f) {
			DestroyImmediate();
			return true;
		}
	}
	return false;
}

