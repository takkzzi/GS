#pragma once

#include "Core/Singleton.h"

namespace Game
{
	class GameNetworker;
	class Level;


	class GameCommon : public Singleton<GameCommon>
	{
	friend Singleton<GameCommon>;
	protected :
		GameCommon();
		virtual ~GameCommon();

	public :
		virtual void			Init();
		virtual void			Shutdown();
		virtual void			MainLoop();

		Level*					GetLevel()			{ return mLevel; };

	protected:
		bool					mbInit;

		/** Scene Handling */
		Level*					mLevel;
		//ResourceManager*		mResourceManager;

		/** Rendering */
		//Renderer*				mRenderer;

		/** Networking */
		GameNetworker*			mGameNetworker;

	protected:
		double					mAppRuntime;

	};

}

#define			TheGame			GameCommon::Instance()
