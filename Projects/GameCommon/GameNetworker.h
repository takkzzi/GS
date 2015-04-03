#pragma once

namespace Game
{
	class GamePacketReader;
	class UserSessionManager;


	class GameNetworker
	{
	public:
		GameNetworker();
		virtual ~GameNetworker();

	public :
		virtual		void	Update(float dt);

	protected:
		Networker*					mIocpNetworker;
		UserSessionManager*			mUserSessionMgr;
		GamePacketReader*			mPackeReader;		//Buffer-Reading & Packetizing & Calling Packet Handler
	};
}

