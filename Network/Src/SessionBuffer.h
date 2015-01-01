#pragma once

namespace Network
{
	class Session;

	/* 
	mBuffer			mBufferStart	mDataHead(Moving)		mDataTail(Moving)		mBufferEnd
	|----------------------|-----------------|-------------------------|-------------------|
	|--- (Extra Buffer) ---|
	*/

	class CircularBuffer
	{
	public :
		CircularBuffer();
		virtual ~CircularBuffer();

		void						Init(int size, int dataMaxSize);
		void						ClearAll();

		bool						Write(char* data, size_t size);	//For Sending
		bool						Write(size_t size);				//For Recv Complete (No memcpy)

		int							Read(char** bufPtr);			//Linear Whole Size
		bool						Read(char** bufPtr, int* reqSize, bool bResize, bool bCircularMerge);

		bool						GetEmpty(char** bufPtr, int* size);		//If Size 0, musch as possible;
		bool						ClearData(int size);
		
		size_t						GetDataSize();
		char*						GetDataHead()					{ return mCircletart + mDataHead; };
		char*						GetDataTail()					{ return mCircleStart + mDataTail; };

		int							GetDataHeadPos()				{ return mDataHead; };
		int							GetDataTailPos()				{ return mDataTail; };

	protected:
		bool						DoWriteSeparate(char* data, size_t size);
		bool						DoGetAndMergeData(char** bufPtr, int* reqSize, bool bResize);		//Use Only Separated Data.

		bool						IsCircularData()				{ return (0 < mDataHead) && (mDataTail <= mDataHead); }
		bool						IsUsingExtraBuffer()			{ return (mDataHead < 0); }
		
	protected:

		int							mBufferSize;
		int							mBufferExtraSize;	//front of Start Part : Combine Separate End and Start 

		//For Circular Buffering
		char*						mBuffer;
		char*						mCircleStart;
		char*						mCircleEnd;
		int							mDataHead;		//Data Start : Moving
		int							mDataTail;		//Data End : Moving

		CriticalSection				mCriticalSec;
	};

	/*
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
	*/

	//추후 Game Module 로 옮기자
#pragma pack (1)
	struct PacketBase
	{
		USHORT	mPacketSize;
	};
#pragma pack()

#pragma pack (1)
	struct AlphabetPacket : public PacketBase
	{
		AlphabetPacket() {
			memcpy(mData, "abcdefghijklmnopqrstuvwxyz", sizeof(mData));
		}

		UINT		mPacketId;
		char		mData[4];
	};
#pragma pack()
}
