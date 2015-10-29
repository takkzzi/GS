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
		virtual		void	Init(int reservUserCount, int maxUserCount, int bufferSize);
		virtual		void	ServerStart(UINT16 port);
		virtual		void	Update(float dt);

	protected:
		TcpNetworker*				mIocpNetworker;
		NetUserManager*				mUserManager;
		GamePacketReader*			mPacketReader;		//Buffer-Reading & Packetizing & Calling Packet Handler
	};
}

