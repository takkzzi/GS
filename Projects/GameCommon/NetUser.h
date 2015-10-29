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

		//TEST : State Transitions
		void						EnterGame();
		void						QuitGame();

	protected:
		void						ResetData();
		GamePacketBase*				DoPacketize(UINT packetMinSize);
		
		
	protected:
		Network::TcpSession*		mSession;
		UserState					mUserState;

		Level*						mLevel;
		Player*						mPlayer;
	};
}
