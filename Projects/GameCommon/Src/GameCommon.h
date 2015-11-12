#pragma once

#include "Core/Singleton.h"

namespace Game
{
	class GameNetworker;
	class Level;
	class NetUserManager;
	class UserSession;
	class LocalPlayer;

	class GameCommon //: public Singleton<GameCommon>
	{
	//friend Singleton<GameCommon>;
	protected :
		GameCommon();
		virtual ~GameCommon();

	public :
		virtual void			Init();
		virtual void			Shutdown();

		void					StartServer(int reserveUserCount, int maxUserCount, int bufferSize, int port);
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


		UserSession*			mLocalUserSession;		//Client
		LocalPlayer*			mLocalPlayer;			//Client

	protected:
		double					mAppRuntime;

	};

}

//#define			TheGame			GameCommon::Instance()
