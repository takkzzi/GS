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
	public:
		Session(Networker* networker, int id, int sendBufferSize, int recvBufferSize);
		~Session(void);

	public:
		void					Init();
		int						GetId()		{ return mId; }
		void					SetState(SessionState state);
		bool					IsState(SessionState state)			{ return (mState == state); }
		SessionState			GetState()							{ return mState; }
		void					ResetState(bool bClearDataQ);

		bool					Connect(const CHAR* addr, USHORT port);
		bool					Disconnect();
		bool					StartAccept(SOCKET listenSock);		//Using AcceptEx()
		bool					Send();

		bool					WriteData(char* data, int dataLen);
		PacketBase*				ReadData();
		bool					ClearRecv(int bufSize);

		void					Update();
		
		//TEST
		void					LogPacket(char* prefix, AlphabetPacket* packet);

	//Start Event Callback
	public :
		void					OnAccept(SOCKET listenSock);
		void					OnSendComplete(OverlappedIoData* overlapIoData, DWORD sendSize);
		void					OnRecvComplete(OverlappedIoData* overlapIoData, DWORD recvSize);

	protected:
		void					OnConnect();
	//End Event Callback

	protected:
		bool					StartReceive();		//Receive Using Overlapped

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
		CircularBuffer			mSendBuffer;
		CircularBuffer			mRecvBuffer;

		SOCKET					mListenSock;
		bool					mIsAccepter;
		volatile	bool		mbSendCompleted;
		volatile	bool		mbRecvCompleted;
		volatile	bool		mbRecvLock;
		CriticalSection			mCritiSect;

		volatile INT64			mRecvLockCount;
		volatile INT64			mRecvUnlockCount;
	};


	class SessionEventObject
	{
	public :
		SessionEventObject();
		virtual ~SessionEventObject();

	public :
		virtual		void		Init(Session* session);

		//Session Event
		virtual		void		OnConnect()			=	0;
		virtual		void		OnDisconnect()		=	0;
		virtual		void		OnReceived()		=	0;
		virtual		void		OnSend()			=	0;
		
	};
	
}
