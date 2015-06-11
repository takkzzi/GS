#pragma once

namespace Game
{
	class GamePacketReader;
	class NetUserManager;


	class GameNetworker
	{
	public:
		GameNetworker();
		virtual ~GameNetworker();

	public :
		virtual		void	Update(float dt);

	protected:
		Networker*					mIocpNetworker;
		NetUserManager*				mUserManager;
		GamePacketReader*			mPackeReader;		//Buffer-Reading & Packetizing & Calling Packet Handler
	};
}

