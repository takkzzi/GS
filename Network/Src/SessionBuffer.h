#pragma once

namespace Network
{
	class Session;

	class CircularBuffer
	{
	public :
		CircularBuffer();
		virtual ~CircularBuffer();

		void						Init(int size, int dataMaxSize);
		void						ClearAll();

		bool						Push(char* data, int size);
		int							GetData(char** bufPtr);		//Linear Whole Size
		bool						GetData(char** bufPtr, int reqSize, bool bCombineSeparate);

		bool						GetEmpty(char** bufPtr, int* size);
		bool						Clear(int size);
		
	protected:
		bool						DoPushSeparate(char* data, int size);
		bool						DoGetAndMergeData(char** bufPtr, int size);		//Use Only Separated Data.
		bool						IsUsingExtraBuffer()			{ return (mDataHead < mBufferStart); }

	protected:

		int							mBufferSize;
		int							mBufferExtraSize;	//front of Start Part : Combine Separate End and Start 

		//For Circular Buffering
		char*						mBuffer;
		char*						mBufferStart;
		char*						mBufferEnd;
		char*						mDataHead;		//Data Start : Moving
		char*						mDataTail;		//Data End : Moving

		CriticalSection				mCriticalSec;
	};


	class SessionBufferQueue
	{
	public:
		SessionBufferQueue();
		virtual ~SessionBufferQueue();

		//IOCP Event Callback
		virtual		bool			OnIoComplete(char* bufPtr, DWORD transferBytes)	=	0;

	protected:
		CriticalSection				mCriticalSec;
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