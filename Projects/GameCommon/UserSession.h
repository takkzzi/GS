#pragma once

namespace Game
{
	//class Network::Session;
	struct GamePacketBase;

	enum UserState 
	{
		USERSTATE_NONE,
		USERSTATE_CONNECTED,
		USERSTATE_LOBBY,
		USERSTATE_SHOP,
		USERSTATE_ENTRANCE,
		USERSTATE_GAME,
	};


	class UserSession
	{
	public :
		UserSession();
		~UserSession();

	public:
		void						Init(Network::Session* session);
		void						Destroy();
		bool						IsDestroyed()				{ return mUserState == USERSTATE_NONE; }

		GamePacketBase*				GetRecvPacket();
		bool						ClearRecvPacket(UINT buffSize);

	protected:
		void						ResetData();
		GamePacketBase*				DoPacketize(UINT packetMinSize);
		
		
	protected:
		Network::Session*			mSession;
		UserState					mUserState;

		UINT						mLastReadSize;
	};
}
