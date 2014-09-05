#pragma once

#include <vector>
#include <map>

#ifdef __cpluscplus
extern "C" {
#endif // __cpluscplus

namespace Network
{
	using namespace Core;

	class Session;
	class Listener;

	class IOCP
	{
	public:
		IOCP(int threadCount, int maxSession, bool bPrecacheSession, int sendBufferSize, int recvBufferSize);
		virtual ~IOCP(void);

	public:

		void			BeginListen(UINT16 port);
		void			EndListen();

		Session*		GetNewSession();
		Session*		GetSession(int id);
		void			ClearSession(int id);
		
		HANDLE			GetIocpHandle()					{ return mIocp; }

		void			Update();

	protected:
		void			BeginIo(int threadCount);
		void			EndIo();
		void			DeleteAllSessions();

	private:

		HANDLE							mIocp;
		class Listener*					mListener;

		int								mSessionCount;
		int								mSendBufferSize;
		int								mRecvBufferSize;

		std::vector<Core::Thread*>				mThreadVec;
		std::vector<Session*>					mSessionVec;
		std::map<int, Session*>					mSessionMap;
	};
}

#ifdef __cpluscplus
}
#endif // __cpluscplus