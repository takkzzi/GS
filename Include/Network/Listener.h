#pragma once
#include "Core/Thread.h"


namespace Network
{
	class IOCP;

	class Listener : public Core::Thread
	{
	public:
		Listener(IOCP* iocp, UINT16 port);
		~Listener(void);

		virtual bool		Begin(bool bSuspend=false);
		virtual void		End(bool bForceTerminate=false);

		virtual DWORD		Run();
		virtual void		OnEnd(bool bTerminated=false);

	private:
		void			OnAccept();

	private:
	
		UINT16			mPort;
		SOCKET			mSock;
		HANDLE			mEvent;
		IOCP*			mIocp;
	};
}
