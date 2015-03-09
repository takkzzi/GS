#pragma once

#include "Core/Singleton.h"

namespace Game
{
	class GameNetworker;

	class GameCommon : public Singleton<GameCommon>
	{
	protected :
		GameCommon();
		virtual ~GameCommon();

	public :
		virtual void			Init();
		virtual void			Shutdown();
		virtual void			MainLoop();

	protected:
		bool					mbInit;

		/** Networking */
		GameNetworker*			mGameNetworker;

		/** To Do */
		/** Scene Handling */
		//GameLevel*			mLevel;
		//ResourceManager*		mResourceManager;

		/** Rendering */
		//Renderer*				mRenderer;


	protected:
		double					mAppRuntime;

	};

#define			TheGame			Game::Instance()

}
