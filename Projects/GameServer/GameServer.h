#pragma once

namespace Server 
{
	class GameServer : public GameCommon// , public Singleton<GameServer>
	{
	public :
		GameServer();
		virtual ~GameServer();

	public:
		virtual void Init();

	protected:

	};
}