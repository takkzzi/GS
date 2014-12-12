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

	struct Overlapped 
	{
		Overlapped(Session* ownSession, OverlappedIoType type) 
		{ 
			ZeroMemory(&ov, sizeof(WSAOVERLAPPED));
			ioType = type;
			session = ownSession;
		}

		WSAOVERLAPPED				ov;
		OverlappedIoType			ioType;
		Session*					session;
	};

	struct BufferItem
	{
		BufferItem(Session* ownSession, OverlappedIoType ioType, int bufferSize, int index) {
			mOverlapped = new Overlapped(ownSession, ioType);
			mWsaBuf.buf = new char[bufferSize];
			mWsaBuf.len = 0;
			mIndex = index;
		}

		~BufferItem() {
			delete[]	mWsaBuf.buf;
			delete		mOverlapped;
		}

		void Push(char* data) {
			int dataLen = *(UINT16*)data;
			::CopyMemory(mWsaBuf.buf, data, dataLen);
			mWsaBuf.len = dataLen;
		}

		void Clear() {
			mWsaBuf.len = 0;
		}

		Overlapped*		mOverlapped;
		WSABUF			mWsaBuf;
		int				mIndex;
	};


	class SessionBuffer
	{
	public :
		SessionBuffer();
		virtual ~SessionBuffer();

		void						Init(Session* sess, OverlappedIoType ioType, int bufferCount, int bufferSize);
		void						Clear();

		bool						PushCopy(char* data);
		BufferItem*					GetEmptyBuffer();

		void						OnSend(Overlapped* overlapped, int byteCount);
		void						OnRecv(Overlapped* overlapped, int byteCount);

		BufferItem*					GetFilledBuffer();
		void						ClearBuffer(BufferItem* item);

	protected:
		
		Session*					mSession;
		OverlappedIoType			mIOType;
		int							mBufferSize;

		std::vector<BufferItem*>	mBuffers;
	};

}