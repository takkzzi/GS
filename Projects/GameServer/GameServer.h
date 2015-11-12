#pragma once

class Game::GameNetworker;

namespace Server 
{
	class GameServer
	{
	public :
		GameServer();
		~GameServer();

	public:
		void					Init();
		void					Shutdown();
		void					BeginServer(int port);
		void					MainLoop();

	protected:

		GameNetworker*			mGameNetworker;
		double					mAppRuntime;

	};
}