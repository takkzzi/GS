#pragma once

#include <vector>
#include <map>


enum NetMethod {
	//NET_EVENTSELECT,
	NET_IOCP,
};

class IOCPThread;

namespace Network
{
	using namespace Core;

	class Session;
	class Listener;

	//IOCP Only
	class Networker
	{
	public:
		Networker(int threadCount, int reserveSessionCount, int sessionLimitCount, int sendBufferSize, int recvBufferSize);
		virtual ~Networker(void);

	public:

		void			BeginListen(UINT16 port, bool bPreAccept);
		void			EndListen();

		Session*		GetNewSession();
		Session*		GetSession(int id);
		void			ClearSession(int id);
		int				GetSessionCount()				{ return mSessionVec.size(); }
		HANDLE			GetIocpHandle()					{ return mIocp; }

		void			UpdateSend();
		void			UpdateRecv();

	protected:
		void			BeginIo(int threadCount);
		void			EndIo();

		void			PreacceptAll();
		void			DeleteAllSessions();

	private:

		HANDLE							mIocp;
		class Listener*					mListener;
		bool							mbPreAccept;

		int								mSessionLimitCount;
		int								mSendBufferSize;
		int								mRecvBufferSize;

		std::vector<Core::Thread*>				mThreadVec;
		std::vector<Session*>					mSessionVec;

		CriticalSection					mCriticalSec;
	};
}
