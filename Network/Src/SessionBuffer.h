#pragma once

namespace Network
{
	class Session;

	struct SessionBuffer		//For SessionDataQueue On Overlapped I/O
	{
		SessionBuffer(int bufferLength, int vecIndex) {
			buf = new char[bufferLength];
			len = 0;
			index = vecIndex;
		}

		~SessionBuffer() {
			delete[] buf;
		}

		void Clear() {
			len = 0;
		}

		char*		buf;
		int			len;
		int			index;
	};


	class SessionBufferQueue
	{
	public :
		SessionBufferQueue();
		virtual ~SessionBufferQueue();

		void						Init(int bufferCount, int bufferLength);
		void						Clear();

		void						Push(char* data, int len);
		SessionBuffer*				Pop(bool bClearBuffer);		//NO Delete
		SessionBuffer*				GetFront();
		virtual SessionBuffer*		GetEmpty();
		
		//IOCP Event Callback
		virtual		bool			OnIoComplete(SessionBuffer* buffer, DWORD transferBytes)	=	0;

	protected:
		
		int								mBufferLen;		//per buffer
		std::vector<SessionBuffer*>		mBufferVec;		//Empty Buffers
		std::deque<SessionBuffer*>		mBufferQ;		//Used Buffers

		CriticalSection					mCriticalSec;
	};


	//For Sending	////////////////////////////////////////////////////////////////////////////////
	class SendBufferQueue : public SessionBufferQueue
	{
	public :
		//virtual ~SendBufferQueue()	{};

		virtual		bool				OnIoComplete(SessionBuffer* buffer, DWORD transferBytes);
	};


	//For Receiving	////////////////////////////////////////////////////////////////////////////////
	class RecvBufferQueue : public SessionBufferQueue
	{
	public:
		//virtual ~RecvBufferQueue()	{};

		virtual		bool				OnIoComplete(SessionBuffer* buffer, DWORD transferBytes);
		virtual		SessionBuffer*		GetEmpty();		//For Pre-Recving
	};

}