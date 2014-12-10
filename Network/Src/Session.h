#pragma once

#include "SessionBuffer.h"

namespace Network
{
	class Networker;
	class Listener;
	
	enum SessionState {
		SESSIONSTATE_NONE,
		SESSIONSTATE_ACCEPTING,
		SESSIONSTATE_CONNECTED,
	};


	class Session
	{
	public:
		Session(Networker* networker, int id, int sendBufferSize, int recvBufferSize);
		~Session(void);

	public:

		bool					Connect(const CHAR* addr, USHORT port);
		bool					Accept(SOCKET listenSock);
		bool					Disconnect(bool bAccept=false);
		bool					Send(BYTE* data, int dataLen);

		void					OnCompletionStatus(Overlapped* overlapped, DWORD transferSize);
		
	public :
		virtual void			OnAccept(SOCKET listenSock);

	protected:
		void					OnConnect();
		virtual void			OnSendComplete(DWORD sendSize);
		virtual void			OnRecvComplete(DWORD recvSize);
		virtual void			OnDisconnect();

	public:
		int						GetId()		{ return mId; }
		void					SetState(SessionState state);
		bool					IsState(SessionState state)			{ return (mState == state); }

	protected:
		Networker*				mNetworker;
		int						mId;
		SessionState			mState;
		
		SOCKET					mSock;
		HANDLE					mEvent;
		SOCKADDR_IN				mRemoteAddr;

		Overlapped*				mOverlappedAccept;
		SessionBuffer*			mSendBuffer;
		SessionBuffer*			mRecvBuffer;

		SOCKET					mListenSock;
		BOOL					mIsAccepter;
	};

}
