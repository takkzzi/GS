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
		Networker(bool bUseThreadUpdateSession, int ioThreadCount, int sessionReserveCount, int sessionLimitCount, int sendBufferSize, int recvBufferSize);
		virtual ~Networker(void);

	public:

		void					BeginListen(UINT16 port, bool bPreAccept);
		void					EndListen();

		Session*				GetSession(int id);
		Session*				GetNewSession();
		void					ClearSession(int id);
		int						GetSessionCount()				{ return mSessionVec.size(); }
		HANDLE					GetIocpHandle()					{ return mIocp; }
		
		bool					IsPreAccepter();
		SOCKET					GetListnerSocket();

		bool					IsThreadUpdatingSessions()		{ return mbThreadUpdateSessions; };
		void					UpdateSessions();

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
		std::vector<HANDLE>				mIoThreadHandles;
		bool							mbThreadUpdate;
		UINT							mIoThreadCount;
		class Listener*					mListener;
		bool							mbPreAccept;

		INT								mSessionLimitCount;
		INT								mSendBufferSize;
		INT								mRecvBufferSize;

		std::vector<Session*>			mSessionVec;
		volatile	bool				mbThreadUpdateSessions;
		HANDLE							mSessUpdateThread;
		CriticalSection					mCritiSect;
	};
}
