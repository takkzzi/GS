#pragma once


namespace Network
{
	class Networker;
	class Listener;

	enum OverlappedIoType {
		IO_SEND,
		IO_RECV,
	};

	struct Overlapped {
		Overlapped(OverlappedIoType type, int bufLen) 
		{ 
			ZeroMemory(&ov, sizeof(WSAOVERLAPPED));
			iotype = type;
			wsaBuf.buf = new CHAR[bufLen];
			wsaBuf.len = bufLen;
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
		//SOCKET				sock;
	};

	enum SessionState {
		SESSIONSTATE_NONE,
		SESSIONSTATE_LISTENING,
		SESSIONSTATE_CONNECTED,
	};

	class Session : public IOKey
	{
	public:
		Session(Networker* networker, int id, int sendBufferSize, int recvBufferSize);
		~Session(void);

	public:

		bool					Connect(const CHAR* addr, USHORT port);
		bool					Disconnect();
		bool					Send(BYTE* data, int dataLen);

		virtual void			OnAccept(SOCKET listenSock);
		virtual void			OnSendComplete(int sendSize);
		virtual void			OnRecvComplete(int recvSize);
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

		Overlapped*				mOverlappedSend;
		Overlapped*				mOverlappedRecv;
		
		//TODO : Implement 'Send Buffer', 'Recv Buffer'
		//NetBuffer				mSendBuffer;
		//NetBuffer				mRecvBuffer;
	};

}
