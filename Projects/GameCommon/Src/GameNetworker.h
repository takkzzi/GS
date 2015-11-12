#pragma once

namespace Game
{
	class GamePacketReader;
	class NetUserManager;

	//일단 서버용
	class GameNetworker
	{
	public:
		GameNetworker();
		virtual ~GameNetworker();

	public :
		virtual		void	Init(int reservSessionCount, int maxSessionCount, int bufferSize);

		virtual		void	BeginListen(UINT16 port);

		virtual		void	Update(float dt);

	protected:
		bool						mbInit;
		TcpNetworker*				mTcpNetworker;
		NetUserManager*				mUserManager;
		GamePacketReader*			mPacketReader;		//Buffer-Reading & Packetizing & Calling Packet Handler
	};
}

