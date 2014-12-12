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

		int						GetId()		{ return mId; }
		void					SetState(SessionState state);
		bool					IsState(SessionState state)			{ return (mState == state); }

		bool					Connect(const CHAR* addr, USHORT port);
		bool					PreAccept(SOCKET listenSock);
		bool					Disconnect(bool bAccept=false);
		bool					Send(char* data, int dataLen);

	//Start Event Callback
	public :
		void					OnCompletionStatus(Overlapped* overlapped, DWORD transferSize);
		virtual void			OnAccept(SOCKET listenSock);

	protected:
		void					OnConnect();
		virtual void			OnSendComplete(Overlapped* overlapped, DWORD sendSize);
		virtual void			OnRecvComplete(Overlapped* overlapped, DWORD recvSize);
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

		BufferItem*				mAcceptBufferItem;
		SessionBuffer*			mSendBuffer;
		SessionBuffer*			mRecvBuffer;

		SOCKET					mListenSock;
		BOOL					mIsAccepter;
	};

}
