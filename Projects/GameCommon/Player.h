#pragma once

#include "Actor.h"

namespace Game 
{
	class Player : public Actor
	{
	public :
		Player();
		virtual ~Player();

	protected :
		virtual			void			OnCreate();
		virtual			void			OnDestroy();
		virtual			void			Update(float dt);

	public :
		UINT		SetPlayerIndex(UINT index)		{ mPlayerIndex = index; }
		UINT		GetPlayerIndex()				{ return mPlayerIndex; }

	protected :

		UINT		mPlayerIndex;
	};

}
