#pragma once

#ifdef __cpluscplus
extern "C" {
#endif // __cpluscplus

namespace Network
{
	class IOCP;
	class Listener;

	enum OverlappedIoType {
		IO_SEND,
		IO_RECV,
	};

	struct Overlapped {
		Overlapped(OverlappedIoType type, SOCKET s, int bufLen) { 
			ZeroMemory(&ov, sizeof(WSAOVERLAPPED));
			iotype = type;
			wsaBuf.buf = new CHAR[bufLen];
			wsaBuf.len = bufLen;
		}
		~Overlapped() {
			delete wsaBuf.buf;
		}

		WSAOVERLAPPED		ov;
		OverlappedIoType	iotype;		
		WSABUF				wsaBuf;
		SOCKET				sock;
	};

	enum SessionState {
		SESSION_NONE,
		//SESSOIN_OCCUPIED,
		SESSION_CONNECTED,
		//SESSION_DISCONNECTING,
	};

	class Session
	{
	public:
		Session(int id, int sendBufferSize, int recvBufferSize);
		~Session(void);

	public:

		bool					Connect(CHAR* addr, USHORT port);
		bool					Disconnect();
		bool					Send(BYTE* data, int dataLen);

		virtual void			OnAccept(IOCP* iocp, SOCKET listenSock);
		virtual void			OnSendComplete(int sendSize);
		virtual void			OnRecvComplete(int recvSize);
		//virtual void			OnDisconnect();

	public:
		int						GetId()		{ return mId; }
		void					SetState(SessionState state);
		bool					IsState(SessionState state)			{ return (mState == state); }

	protected:
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

#ifdef __cpluscplus
}
#endif // __cpluscplus