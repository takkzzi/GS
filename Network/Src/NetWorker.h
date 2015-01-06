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
	
	//Workin With IOCP
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

		bool					IsPreAccepter();
		SOCKET					GetListnerSocket();

		bool					IsUpdatingSession()				{ return mbUpdateSessions; };
		bool					UpdateSessions();

	protected:
		void					BeginIo();
		void					EndIo();

		void					BeginSessionUpdate();
		void					EndSessionUpdate();

		Session*				AddSession();

		void					StartAcceptAll();
		void					DeleteAllSessions();

	private:

		HANDLE							mIocp;
		UINT							mThreadCount;
		UINT							mIoWorkingCount;
		class Listener*					mListener;
		bool							mbPreAccept;

		INT								mSessionLimitCount;
		INT								mSendBufferSize;
		INT								mRecvBufferSize;

		std::vector<Session*>			mSessionVec;
		volatile	bool				mbUpdateSessions;
		HANDLE							mSessUpdateThread;
		CriticalSection					mCriticalSec;
	};
}
