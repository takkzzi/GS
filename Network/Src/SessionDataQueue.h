#pragma once

namespace Network
{
	class Session;

	enum OverlappedIoType 
	{
		IO_NONE,
		IO_ACCEPT,
		IO_SEND,
		IO_RECV,
	};

	struct OverlappedData	//For SessionDataQueue On Overlapped I/O
	{
		OverlappedData(Session* ownSession, OverlappedIoType io, int bufferSize, int arrayIdx)
		{
			::ZeroMemory(&ov, sizeof(WSAOVERLAPPED));
			ioType = io;
			wsaBuf.buf = new char[bufferSize];
			wsaBuf.len = 0;
			session = ownSession;
			index = arrayIdx;
		}

		~OverlappedData() {
			delete[]	wsaBuf.buf;
		}

		void Push(char* data, int length) {
			::CopyMemory(wsaBuf.buf, data, length);
			wsaBuf.len = length;
		}

		void Clear() {
			wsaBuf.len = 0;
		}

		WSAOVERLAPPED				ov;
		OverlappedIoType			ioType;
		WSABUF						wsaBuf;
		Session*					session;
		int							index;
	};


	class SessionDataQueue
	{
	public :
		SessionDataQueue();
		virtual ~SessionDataQueue();

		void						Init(Session* sess, OverlappedIoType ioType, int dataCount, int dataBufferSize);
		void						Clear();

		void						Push(char* data, int length);
		OverlappedData*				Pop();		//NO Delete
		OverlappedData*				GetEmptyData();

		//Net Event Callbacks
		void						OnSendComplete(OverlappedData* olData, int byteCount);
		void						OnRecvComplete(OverlappedData* olData, int byteCount);
		

	protected:
		
		Session*						mSession;
		OverlappedIoType				mIOType;
		int								mDataBufferSize;

		std::vector<OverlappedData*>	mDataVec;
		std::deque<OverlappedData*>		mDataQueue;
	};

}