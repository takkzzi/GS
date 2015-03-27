#pragma once

namespace Game
{
	class GamePacketReader;
	class Level;
	
	class GameNetworker
	{
	public:
		GameNetworker();
		virtual ~GameNetworker();

	public :
		virtual		void	Update(float dt);

	protected:
		Networker*					mIocpNetworker;
		GamePacketReader*			mPackeReader;		//Buffer-Reading & Packetizing & Calling Packet Handler
		Level*						mLevel;
	};
}

