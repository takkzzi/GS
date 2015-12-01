#pragma once

namespace Game
{
	//class Network::Session;
	struct GamePacketBase;
	class Level;
	class Player;


	enum UserState 
	{
		USERSTATE_NONE,
		USERSTATE_CONNECTED,
		USERSTATE_LOBBY,
		USERSTATE_SHOP,
		USERSTATE_ENTRANCE,
		USERSTATE_GAME,
	};


	class NetUser
	{
	public :
		NetUser();
		~NetUser();

	public:
		void						Init(Network::TcpSession* session);
		void						Destroy();
		bool						IsDestroyed()				{ return mUserState == USERSTATE_NONE; }
		UserState					GetState()					{ return mUserState; }

		GamePacketBase*				GetRecvPacket();
		bool						ClearRecvPacket(UINT buffSize);

		//State Transitions
		void						EnterGame(Level* level);
		void						QuitGame();

		void						SetLevel(Level* level);

		//Networking
		bool						SendData(char* data, int dataSize);

	protected:
		void						ResetData();
		GamePacketBase*				DoPacketize();
		
		
	protected:
		Network::TcpSession*		mSession;
		UserState					mUserState;

		Level*						mLevel;
		Player*						mPlayer;
	};
}
