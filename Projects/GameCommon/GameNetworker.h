#pragma once

namespace Game
{
	class PacketProcessor;

	
	class GameNetworker
	{
	public:
		GameNetworker(void);
		virtual ~GameNetworker(void);

		
	public :
		virtual		void	Update(float dt);

	protected:
		Networker*				mIocpNetworker;
		PacketProcessor*		mPackeProcessor;			//Buffer-Reading & Packetizing & Calling Packet Handler
	};
}

