#pragma once

namespace Network
{
	class Session;

	class SessionBufferQueue
	{
	public :
		SessionBufferQueue();
		virtual ~SessionBufferQueue();

		void						Init(int bufferCount, int bufferLength);
		void						ClearAll();

		bool						Push(char* data, int size);
		bool						GetData(char** bufPtr, int* size);
		bool						GetEmpty(char** bufPtr, int* size);
		bool						ClearData(int len);
		
		//IOCP Event Callback
		virtual		bool			OnIoComplete(char* bufPtr, DWORD transferBytes)	=	0;

	protected:
		
		int								mBufferLen;

		//For Ring Buffering
		char*							mBuffer;
		char*							mBufferStart;
		char*							mBufferEnd;
		char*							mDataHead;		//Data Start : Moving
		char*							mDataTail;		//Data End : Moving

		CriticalSection					mCriticalSec;
	};


	//For Sending	////////////////////////////////////////////////////////////////////////////////
	class SendBufferQueue : public SessionBufferQueue
	{
	public :

		virtual		bool				OnIoComplete(char* bufPtr, DWORD transferBytes);
	};


	//For Receiving	////////////////////////////////////////////////////////////////////////////////
	class RecvBufferQueue : public SessionBufferQueue
	{
	public:

		virtual		bool				OnIoComplete(char* bufPtr, DWORD transferBytes);
	};

}