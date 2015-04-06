#pragma once

#include "SessionBuffer.h"

namespace Network
{
	class Networker;
	class Listener;
	class SessionEventObject;


	enum OverlappedIoType 
	{
		IO_NONE,
		IO_ACCEPT,
		IO_SEND,
		IO_RECV,
	};

	struct OverlappedIoData 
	{
		void Init(OverlappedIoType overlappedIoType, Session* ownSession) {
			::ZeroMemory(&ov, sizeof(WSAOVERLAPPED));
			ioType = overlappedIoType;
			session = ownSession;
		}

		void Reset(char* buf) {
			::ZeroMemory(&ov, sizeof(WSAOVERLAPPED));
			bufPtr = buf;
		}

		WSAOVERLAPPED		ov;
		OverlappedIoType	ioType;
		Session*			session;
		char*				bufPtr;
	};

	enum SessionState {
		SESSIONSTATE_NONE,
		SESSIONSTATE_ACCEPTING,
		SESSIONSTATE_CONNECTED,
		SESSIONSTATE_DISCONNECTING,
	};


	class Session
	{
		friend Networker;

	public:
		Session(Networker* networker, int id, int sendBufferSize, int recvBufferSize);
		~Session(void);

	public:
		void					Init();
		int						GetId()		{ return mId; }
		void					SetState(SessionState state);
		bool					IsState(SessionState state)			{ return (mState == state); }
		SessionState			GetState()							{ return mState; }
		bool					IsConnected()						{ return IsState(SESSIONSTATE_CONNECTED); }

		void					ResetState(bool bClearDataQ);

		bool					Connect(const CHAR* addr, USHORT port);
		bool					Disconnect();
		bool					StartAccept(SOCKET listenSock);		//Using AcceptEx()
		bool					Send();

		bool					WriteSendBuffer(char* data, int dataLen);
		char*					ReadRecvBuffer(int bufSize);
		bool					ClearRecvBuffer(int bufSize);

	//Start Event Callback
	public :
		void					OnAccept(SOCKET listenSock);
		void					OnSendComplete(OverlappedIoData* overlapIoData, DWORD sendSize);
		void					OnRecvComplete(OverlappedIoData* overlapIoData, DWORD recvSize);
		void					OnDisconnect();

	protected:
		void					Update();
		void					OnConnect();
	//End Event Callback

	protected:
		bool					StartReceive();		//Receive Using Overlapped
		void					SetKeepAliveOpt();

	protected:

		Networker*				mNetworker;
		int						mId;
		SessionState			mState;
		
		SOCKET					mSock;
		HANDLE					mEvent;
		SOCKADDR_IN				mRemoteAddr;

		OverlappedIoData		mAcceptIoData;
		OverlappedIoData		mSendIoData;
		OverlappedIoData		mRecvIoData;

		char*					mAcceptBuffer;
		SessionBuffer			mSendBuffer;
		SessionBuffer			mRecvBuffer;

		SOCKET					mListenSock;
		bool					mIsAccepter;
		volatile	bool		mbSendPending;
		volatile	bool		mbRecvStarted;
		volatile	bool		mbRecvLock;

		SessionEventObject*		mEventObj;

		CriticalSection			mCritiSect;

	};
	
}
