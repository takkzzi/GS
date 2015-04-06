#pragma once
#include "GameObject.h"


namespace Game 
{
	enum ActorType {
		ACTOR_NONE,
		ACTOR_STATIC,
		ACTOR_BREAKABLE,
		ACTOR_TRIGGER,
		ACTOR_PLAYER,
		ACTOR_MAX,
	};

	class LevelActorFactory;
	class Level;

	class Actor : public GameObject
	{
		friend	LevelActorFactory;
		friend	Level;

	public :

		Actor();
		virtual ~Actor();

	public :			
						void		Destroy(float time=0.f);
						

						UINT64		GetId()					{ return mActorId; }

	protected :
		void						DoDestroy();

		virtual			void		OnCreate()				{};
		virtual			void		OnDestroy()				{};
		virtual			void		Update(float dt)		{};
		
	private :
						bool		UpdateDestroy(float dt);

	protected:

		ActorType			mActorType;
		UINT64				mActorId;
		Level*				mLevel;

		float				mDeleteTimer;
		bool				mDestroyed;
	};
}
