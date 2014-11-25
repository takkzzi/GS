#pragma once
#include "Core/Thread.h"


namespace Network
{
	class Networker;

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
		void			OnAccept();

	private:
	
		UINT16			mPort;
		SOCKET			mSock;
		HANDLE			mEvent;
		Networker*		mNetworker;
	};
}
