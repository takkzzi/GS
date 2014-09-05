#pragma once
#include "Core/Thread.h"

#ifdef __cpluscplus
extern "C" {
#endif // __cpluscplus

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

#ifdef __cpluscplus
}
#endif // __cpluscplus