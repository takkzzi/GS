#pragma once
#include "GamePacketBase.h"


namespace Game
{
	struct		GamePacketBase;	
	class		NetUser;


	class GamePacketReader
	{
	public:
		GamePacketReader(void);
		virtual ~GamePacketReader(void);
		
	public:
		void			Init();
		bool			ProcessUserPacket(NetUser* user);

	protected:
		void			BindHanlder();

	protected :
		//** Start Packet Handler 

		void			Alphabet(NetUser* user, char* data, int size);
		void			ChatMsg(NetUser* user, char* data, int size);

		//** End Packet Handler

	protected:

		typedef		void(GamePacketReader::*PacketHandler)(NetUser*, char*, int);	//char:Data, int:Size
		PacketHandler	mHandlerArray[PT_Max];
	};
}
