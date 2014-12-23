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
		bool						GetData(char** bufPtr, int* reqSize, bool bResize, bool bCircularMerge);

		bool						GetEmpty(char** bufPtr, int* size);		//If Size 0, musch as possible;
		bool						ClearData(int size);
		
		int							GetDataSize();

	protected:
		bool						DoPushSeparate(char* data, int size);
		bool						DoGetAndMergeData(char** bufPtr, int* reqSize, bool bResize);		//Use Only Separated Data.

		bool						IsCircularData()				{ return (mDataTail < mDataHead); }
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


	class SendBuffer : public CircularBuffer
	{
	public :
		//SendBuffer();
		//virtual ~SendBuffer();

		bool					OnIoComplete(char* dataPtr, DWORD completeSize);

	};

	class RecvBuffer : public CircularBuffer
	{
	public :
		//RecvBuffer();
		//virtual ~RecvBuffer();

		bool					OnIoComplete(char* dataPtr, DWORD completeSize);
	};

#pragma pack (1)
	struct PacketHeader 
	{
		USHORT	mPacketSize;
	};
#pragma pack (pop)
}