#pragma once

namespace Network
{
	class Session;

	class SessionBuffer
	{
		/* <Circulary Buffer Structure>

		mBuffer			mBufferStart	mDataHead(Moving)		mDataTail(Moving)		mBufferEnd
		|----------------------|-----------------|-------------------------|-------------------|
		|-- (Circular Buffer) -|

		*/

	public :
		SessionBuffer();
		virtual ~SessionBuffer();

		void						Init(int size, int dataMaxSize);
		void						ClearAll();

		bool						Write(char* data, size_t size);
		char*						Read(IN OUT int* reqSize, bool bResize, bool bCircularMerge);

		bool						AddDataTail(size_t size);

		char*						GetEmpty(int* size);		//If Size 0, musch as possible;
		bool						ClearData(int size);
	
		size_t						GetDataSize();
		char*						GetDataHead()					{ return mCircleStart + mDataHead; };
		char*						GetDataTail()					{ return mCircleStart + mDataTail; };

		int							GetDataHeadPos()				{ return mDataHead; };
		int							GetDataTailPos()				{ return mDataTail; };

	protected:
		bool						DoWriteSeparate(char* data, size_t size);
		char*						DoReadAndMergeData(int* reqSize, bool bResize);		//Use Only Separated Data.

		bool						IsCircularData()				{ return (0 < mDataHead) && (mDataTail < mDataHead); }
		bool						IsUsingExtraBuffer()			{ return (mDataHead < 0); }
		
	protected:

		int							mBufferSize;
		int							mBufferExtraSize;	//front of CircleStart Part : Combine Separate End and Start 

		//For Circular Buffering
		char*						mBuffer;
		char*						mCircleStart;
		char*						mCircleEnd;
		int							mDataHead;		//Data Start : Moving
		int							mDataTail;		//Data End : Moving

		CriticalSection				mCritiSect;
	};
}
