#pragma once

namespace Network
{
	class TCPSession;

	class CircleBuffer
	{
		/* <Circulary Buffer Structure>

		mBuffer			mCircleStart	mDataHead(Moving)		mDataTail(Moving)		mCircleEnd
		|----------------------|-----------------|-------------------------|-------------------|
		|-- (Circular Buffer) -|

		*/

	public :
		CircleBuffer();
		virtual ~CircleBuffer();

		void						Init(int bufferSize, int extraBufferSize);
		void						ClearAll();

		bool						Write(char* data, int size);
		char*						Read(IN OUT int* reqSize, bool bResize, bool bCircularMerge=false);

		bool						AddDataTail(int size);

		char*						GetEmpty(int* requiredSize);		//If Size 0, musch as possible;
		bool						ClearData(int size);
	
		int							GetDataSize();
		char*						GetDataHead()					{ return mCircleStart + mDataHead; };
		char*						GetDataTail()					{ return mCircleStart + mDataTail; };

		int							GetDataHeadPos()				{ return mDataHead; };
		int							GetDataTailPos()				{ return mDataTail; };

	protected:
		bool						DoWriteSeparate(char* data, int size);
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
