#pragma once

#include "SessionBuffer.h"

namespace Network
{
	class Networker;
	class Listener;
	
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
		SessionState			GetState()							{ return mState; }
		void					ResetState(bool bClearDataQ);

		bool					Connect(const CHAR* addr, USHORT port);
		bool					Disconnect(bool bReAccept=false);
		bool					PreAccept(SOCKET listenSock);		//Using AcceptEx()
		bool					Send();

		bool					PushSend(char* data, int dataLen);
		//Note : Returned Buffer Must be "Clear()" after Use.
		char*					PopRecv();

	//Start Event Callback
	public :
		//void					OnCompletionStatus(OverlappedData* overlapped, DWORD transferSize);
		void					OnAccept(SOCKET listenSock);
		void					OnSendComplete(OverlappedIoData* overlapIoData, DWORD sendSize);
		void					OnRecvComplete(OverlappedIoData* overlapIoData, DWORD recvSize);

	protected:
		void					OnConnect();
		void					OnDisconnect();
	//End Event Callback

	protected:
		void					PreReceive();		//Receive Using Overlapped

	protected:
		static	bool			msUseAcceptBuffer;
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
		BOOL					mIsAccepter;
		volatile	BOOL		mIsPendingSend;
		CriticalSection			mCriticalSec;
	};

}
