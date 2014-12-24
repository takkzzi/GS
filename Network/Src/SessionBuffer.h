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

		bool						PushData(char* data, int size);
		bool						ReserveData(int size);
		int							GetData(char** bufPtr);		//Linear Whole Size
		bool						GetData(char** bufPtr, int* reqSize, bool bResize, bool bCircularMerge);

		bool						GetEmpty(char** bufPtr, int* size);		//If Size 0, musch as possible;
		bool						ClearData(int size);
		
		int							GetDataSize();
		char*						GetDataHead()					{ return mDataHead; };
		char*						GetDataTail()					{ return mDataTail; };

	protected:
		bool						DoPushSeparate(char* data, int size);
		bool						DoGetAndMergeData(char** bufPtr, int* reqSize, bool bResize);		//Use Only Separated Data.

		bool						IsCircularData()				{ return (mBufferStart < mDataHead) && (mDataTail < mDataHead); }
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
			memcpy(mData, "abcdefghijklmnopqrstuvwxyz", 26);
		}

		USHORT		mPacketId;
		char		mData[26];
	};
#pragma pack()
}
