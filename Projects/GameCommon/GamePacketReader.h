#pragma once
#include "GamePacketBase.h"


namespace Game
{
	class		Network::Session;
	struct		GamePacketBase;	

	class GamePacketReader
	{
	public:
		GamePacketReader(void);
		virtual ~GamePacketReader(void);
		
	public:
		void			Init();
		bool			ReadPacket(Network::Session* session);

	protected:
		void			BindHanlder();
		bool			CallHandler(GamePacketBase* packet);

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
