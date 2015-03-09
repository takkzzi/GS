#pragma once
#include "PacketBase.h"


namespace Game
{
	class		Network::Session;
	struct		PacketBase;	

	class PacketProcessor
	{
	public:
		PacketProcessor(void);
		virtual ~PacketProcessor(void);
		
	public:
		void			Init();
		bool			Process(Network::Session* session);

	protected:
		void			BindHanlder();
		bool			CallHandler(PacketBase* packet);

	protected :
		//** Start Packet Handler 

		void			Alphabet(char* data, int size);
		void			ChatMsg(char* data, int size);

		//** End Packet Handler

	protected:

		typedef		void(PacketProcessor::*PacketHandler)(char*, int);		//char:Data, int:Size
		PacketHandler	mHandlerArray[PT_Max];
	};
}
