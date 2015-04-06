#pragma once
#include "GamePacketBase.h"


namespace Game
{
	struct		GamePacketBase;	
	class		UserSession;


	class GamePacketReader
	{
	public:
		GamePacketReader(void);
		virtual ~GamePacketReader(void);
		
	public:
		void			Init();
		bool			ProcessPacket(GamePacketBase* packet, UserSession* user);

	protected:
		void			BindHanlder();

	protected :
		//** Start Packet Handler 

		void			Alphabet(char* data, int size);
		void			ChatMsg(char* data, int size);

		//** End Packet Handler

	protected:

		typedef		void(GamePacketReader::*PacketHandler)(char*, int);		//char:Data, int:Size
		PacketHandler	mHandlerArray[PT_Max];
	};
}
