#pragma once

#include <vector>
#include <map>
//#include "Core/Threadsync.h"

class IOCPThread;

namespace Network
{
	struct OverlappedIoData;
	class TcpSession;
	class Listener;
	class NetEventDelegator;


	//Workin With IOCP
	class TcpNetworker
	{
	public:
		TcpNetworker(bool bUseThreadUpdateSession, int ioThreadCount, int sessionReserveCount, int sessionLimitCount, int sendBufferSize, int recvBufferSize);
		virtual ~TcpNetworker(void);

	public:

		bool					BeginListen(UINT16 port, bool bPreAccept);
		void					EndListen();

		HANDLE					GetIocpHandle()					{ return mIocp; }

		TcpSession*				GetSession(int id);
		TcpSession*				GetNewSession();
		int						GetSessionCount()				{ return mSessionVec.size(); }
		const std::vector<TcpSession*>*	GetSessionVec()			{ return &mSessionVec; };

		
		bool					IsPreAccept()					{ return (mListener && mbPreAccept); }
		SOCKET					GetListnerSocket();

		bool					IsThreadUpdatingSessions()		{ return mbThreadUpdateSessions; };
		void					UpdateSessions();

		void					SetEventDelegator(NetEventDelegator* eventDelegator);
		NetEventDelegator*		GetEventDelegator()				{ return mNetEventDelegator; };

	protected:
		void					BeginIo();
		void					EndIo();

		void					BeginSessionUpdate();
		void					EndSessionUpdate();

		TcpSession*				AddSession();

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

		std::vector<TcpSession*>		mSessionVec;
		volatile	bool				mbThreadUpdateSessions;
		HANDLE							mSessUpdateThread;
		CriticalSection					mCritiSect;
		NetEventDelegator*				mNetEventDelegator;
	};


	class NetEventDelegator
	{
	public :
		NetEventDelegator()	: mNetworker(NULL)	{};
		virtual ~NetEventDelegator()			{};

	public :

		virtual		void		SetNetworker(TcpNetworker* networker)	{ mNetworker = networker; }

		//Session Event
		virtual		void		OnConnect(TcpSession* session)			=	0;
		virtual		void		OnDisconnect(TcpSession* session)		=	0;
		virtual		void		OnReceived(TcpSession* session)		=	0;
		virtual		void		OnSend(TcpSession* session)			=	0;

	protected :
		TcpNetworker*				mNetworker;
	};
}
