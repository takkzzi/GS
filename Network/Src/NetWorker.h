#pragma once

#include <vector>
#include <map>

class IOCPThread;

namespace Network
{
	using namespace Core;

	struct OverlappedIoData;
	class Session;
	class Listener;

	//IOCP Only
	class Networker
	{
	public:
		Networker(int threadCount, int reserveSessionCount, int sessionLimitCount, int sendBufferSize, int recvBufferSize);
		virtual ~Networker(void);

	public:

		void					BeginListen(UINT16 port, bool bPreAccept);
		void					EndListen();

		Session*				GetSession(int id);
		Session*				GetNewSession();
		void					ClearSession(int id);
		int						GetSessionCount()				{ return mSessionVec.size(); }
		HANDLE					GetIocpHandle()					{ return mIocp; }

		void					OnEndIoThread();
		void					Update();

	protected:
		void					BeginIo();
		void					EndIo();

		Session*				AddSession();

		void					PreacceptAll();
		void					DeleteAllSessions();

	private:

		HANDLE							mIocp;
		UINT							mThreadCount;
		UINT							mWorkingcCount;
		class Listener*					mListener;
		bool							mbPreAccept;

		INT								mSessionLimitCount;
		INT								mSendBufferSize;
		INT								mRecvBufferSize;

		std::vector<Session*>			mSessionVec;
		CriticalSection					mCriticalSec;
	};
}
