#pragma once
#include "Core/Thread.h"


namespace Network
{
	class Networker;


	class Listener 
	{
	public :

		Listener(Networker* networker, UINT16 port) 
			: mNetworker(networker), mPort(port), mSock(INVALID_SOCKET)		
		{};

		virtual ~Listener()
		{};

		virtual bool			BeginListen()	=	0;
		virtual bool			EndListen();

		SOCKET					GetSocket()		{ return mSock; }

	protected:
		UINT16					mPort;
		SOCKET					mSock;
		Networker*				mNetworker;

	};


	//Using Event-Select (Separate Thread)
	class SelectListener : public Listener, public Core::Thread
	{
	public:
		SelectListener(Networker* networker, UINT16 port);
		~SelectListener(void);

		virtual bool		BeginListen();
		virtual bool		EndListen();

		virtual DWORD		ThreadTick();
		virtual void		OnEnd(bool bTerminated=false);

	private:
		void				OnAccept();

	private:
		HANDLE				mEvents[2];	//0 == Listen, 1 == End (Thread Return)
	};


	//Using IOCP
	class IocpListener : public Listener
	{
	public:
		IocpListener(Networker* networker, UINT16 port);
		~IocpListener(void);

	private:
		virtual bool		BeginListen();
		virtual	bool		EndListen();
		
	protected:
	};
	
}
