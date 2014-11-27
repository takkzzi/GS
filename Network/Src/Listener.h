#pragma once
#include "Core/Thread.h"


namespace Network
{
	class Networker;

	/*
	class Listener : public Core::Thread
	{
	public:
		Listener(Networker* iocp, UINT16 port);
		~Listener(void);

		virtual bool		Begin(bool bSuspend=false);
		virtual bool		End();

		virtual DWORD		ThreadTick();
		virtual void		OnEnd(bool bTerminated=false);

	private:
		void				OnAccept();

	private:
	
		UINT16			mPort;
		SOCKET			mSock;
		HANDLE			mEvents[2];	//0 == Listen, 1 == End (Thread Return)
		Networker*		mNetworker;
	};
	*/

	class Listener : public IOKey
	{
	public:
		Listener(Networker* iocp, UINT16 port);
		~Listener(void);

		bool				Begin();
		bool				End();
		void				OnAccept();

	private:
	
		bool			mbBegan;
		UINT16			mPort;
		SOCKET			mSock;
		Networker*		mNetworker;
	};
	
}
