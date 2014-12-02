#pragma once


namespace Network
{
	class Networker;
	class Listener;

	enum OverlappedIoType {
		IO_ACCEPT,
		IO_SEND,
		IO_RECV,
	};

	struct Overlapped {
		Overlapped(void* ownSession, OverlappedIoType type, int bufLen) 
		{ 
			ZeroMemory(&ov, sizeof(WSAOVERLAPPED));
			iotype = type;
			wsaBuf.buf = new CHAR[bufLen];
			wsaBuf.len = bufLen;
			owner = ownSession;
		}

		~Overlapped() {
			delete[] wsaBuf.buf;
		}

		void Reset()
		{
			ZeroMemory(&ov, sizeof(WSAOVERLAPPED));
			ZeroMemory(wsaBuf.buf, wsaBuf.len);
		}

		WSAOVERLAPPED		ov;
		OverlappedIoType	iotype;		
		WSABUF				wsaBuf;
		void*				owner;
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
		Overlapped*				mOverlappedSend;
		Overlapped*				mOverlappedRecv;
		
		SOCKET					mListenSock;
		BOOL					mIsAccepter;
		WSAOVERLAPPED			mAcceptOverlapped;		//For AcceptEx()

		//TODO : Implement 'Send Buffer', 'Recv Buffer'
		BYTE*					mSendBuffer;
		BYTE*					mRecvBuffer;
	};

}
