#pragma once

#include "SessionDataQueue.h"

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

		int						GetId()		{ return mId; }
		void					SetState(SessionState state);
		bool					IsState(SessionState state)			{ return (mState == state); }

		bool					Connect(const CHAR* addr, USHORT port);
		bool					PreAccept(SOCKET listenSock);
		bool					Disconnect(bool bAccept=false);
		bool					Send(char* data, int dataLen);

	//Start Event Callback
	public :
		void					OnCompletionStatus(OverlappedData* overlapped, DWORD transferSize);
		virtual void			OnAccept(SOCKET listenSock);

	protected:
		void					OnConnect();
		virtual void			OnSendComplete(OverlappedData* overlapped, DWORD sendSize);
		virtual void			OnRecvComplete(OverlappedData* overlapped, DWORD recvSize);
		virtual void			OnDisconnect();
	//End Event Callback

	protected:
		void					PreReceive();	

	protected:
		Networker*				mNetworker;
		int						mId;
		SessionState			mState;
		
		SOCKET					mSock;
		HANDLE					mEvent;
		SOCKADDR_IN				mRemoteAddr;

		OverlappedData*			mAcceptData;
		SessionDataQueue		mSendBuffer;
		SessionDataQueue		mRecvBuffer;

		SOCKET					mListenSock;
		BOOL					mIsAccepter;
	};

}
