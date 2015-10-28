#pragma once

#include "CircleBuffer.h"

namespace Network
{
	class TcpNetworker;
	class Listener;
	class TcpSession;
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
		void Init(OverlappedIoType overlappedIoType, TcpSession* ownSession) {
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
		TcpSession*			session;
		char*				bufPtr;
	};

	enum SessionState {
		SESSIONSTATE_NONE,
		SESSIONSTATE_ACCEPTING,
		SESSIONSTATE_CONNECTED,
		SESSIONSTATE_DISCONNECTING,
	};


	class TcpSession
	{
		friend TcpNetworker;

	protected:
		TcpSession(TcpNetworker* networker, int id, int sendBufferSize, int recvBufferSize, int maxPacketSize=1024);
		virtual ~TcpSession(void);

	public:
		static TcpSession*			Create(int sendBufferSize, int recevBufferSize, int maxPacketSize = 1024);
		static void					Destroy(TcpSession* session);

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
		
		char*					ReadRecvBuffer(int bufSize);
		bool					ClearRecvBuffer(int bufSize);

		bool					WriteToSend(char* data, int dataSize);

		CircleBuffer*			GetReadBuffer()	{ return &mRecvBuffer; }
		CircleBuffer*			GetSendBuffer() { return &mSendBuffer; }

	//Start Event Callback
	public :
		void					OnAccept(SOCKET listenSock);
		void					OnSendComplete(OverlappedIoData* overlapIoData, DWORD sendSize);
		void					OnRecvComplete(OverlappedIoData* overlapIoData, DWORD recvSize);
		void					OnDisconnect();

	protected:
		void					OnConnect();
		void					Update();
		bool					Send();

	protected:
		bool					StartReceive();		//Receive Using Overlapped
		void					SetKeepAliveOpt();

	protected:

		static	TcpNetworker*	msDefaultNetworker;
		TcpNetworker*			mNetworker;
		int						mId;
		volatile	SessionState	mState;
		
		SOCKET					mSock;
		HANDLE					mEvent;
		SOCKADDR_IN				mRemoteAddr;

		OverlappedIoData		mAcceptIoData;
		OverlappedIoData		mSendIoData;
		OverlappedIoData		mRecvIoData;

		char*					mAcceptBuffer;
		CircleBuffer			mSendBuffer;
		CircleBuffer			mRecvBuffer;

		SOCKET					mListenSock;
		bool					mIsAccepter;
		volatile	bool		mbSendPending;
		volatile	bool		mbRecvStarted;
		volatile	bool		mbRecvLock;

		SessionEventObject*		mEventObj;

		CriticalSection			mCritiSect;

	};
	
}
