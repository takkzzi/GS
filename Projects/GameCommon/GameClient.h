#include "GameCommon.h"

namespace Game
{
	class GameClient : public GameCommon//, public Singleton<GameClient>
	{
	public:
		GameClient();
		virtual ~GameClient();

	public :
		virtual void		Init();
	};
}