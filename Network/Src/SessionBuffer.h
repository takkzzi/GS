#pragma once

namespace Network
{
	class Session;

	enum OverlappedIoType 
	{
		IO_ACCEPT,
		IO_SEND,
		IO_RECV,
	};

	struct Overlapped 
	{
		Overlapped(Session* ownSession, OverlappedIoType type) 
		{ 
			ZeroMemory(&mOverLapped, sizeof(WSAOVERLAPPED));
			mIoType = type;
			mSession = ownSession;
		}

		WSAOVERLAPPED				mOverLapped;
		OverlappedIoType			mIoType;
		Session*					mSession;
	};


	class SessionBuffer
	{
	public :
		struct BufferItem
		{
			BufferItem(int bufferSize) {
				mWsaBuf.buf = new char[bufferSize];
				mWsaBuf.len = 0;
				mOverlapped = NULL;
			}

			~BufferItem() {
				delete[]	mWsaBuf.buf;
				delete		mOverlapped;
			}

			void CreateOverlapped() {
			}

			WSABUF			mWsaBuf;
			Overlapped*		mOverlapped;
		};

	public :
		SessionBuffer();
		virtual ~SessionBuffer();

		virtual		void			Init(Session* sess, int bufferCount, int bufferSize);
		virtual		int				Clear();

		virtual		bool			Push(char* data);
		virtual		char*			GetBuffer(int idx);
		virtual		bool			RemoveData(int idx);
		BufferItem*					GetEmptyBuffer();

	protected:
		
		Session*					mSession;
		int							mBufferSize;

		std::vector<char*>			mBuffers;
		std::list<char*>			mBufferIndexList;
	};

}