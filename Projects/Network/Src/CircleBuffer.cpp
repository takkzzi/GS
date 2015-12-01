#include "PCH.h"
#include "CircleBuffer.h"

using namespace Network;

#define		CS_LOCK					mCritiSect.Enter();
#define		CS_UNLOCK				mCritiSect.Leave();
#define		CIRCULAR_DATATAIL_MAX	(mDataHead - 1)

CircleBuffer::CircleBuffer()
	: mBufferSize(0)
	, mBufferExtraSize(0)
	, mBuffer(NULL)
	, mCircleStart(0)
	, mCircleEnd(0)
	, mDataHead(0)
	, mDataTail(0)
{
}

CircleBuffer::~CircleBuffer()
{
	CS_LOCK
	delete[] mBuffer;
	CS_UNLOCK
}

void CircleBuffer::Init(int bufferSize, int extraBufferSize)
{
	CS_LOCK

	int newBufferTotal = extraBufferSize + bufferSize;
	int oldBufferTotal = mBufferSize + mBufferExtraSize;
	
	//Ring buffer
	if(mBuffer && (oldBufferTotal != newBufferTotal)) {
		delete[] mBuffer;
		mBuffer = NULL;
	}
	
	if ( !mBuffer) {
		mBuffer = new char[newBufferTotal];
	}

	mBufferSize = bufferSize;
	mBufferExtraSize = extraBufferSize;

	mCircleStart = mBuffer + extraBufferSize;
	mCircleEnd = mCircleStart + mBufferSize;
	
	CS_UNLOCK

	ClearAll();
}

void CircleBuffer::ClearAll()
{
	CS_LOCK
	mDataHead = mDataTail = 0;
	CS_UNLOCK
}

bool CircleBuffer::Write(char* data, int dataSize)
{	
	CS_LOCK

	bool bLinear = ! IsCircularData();
	char* emptyStart = GetDataTail(); // mCircleStart + mDataTail;
	char* emptyEnd = bLinear ? mCircleEnd : (mCircleStart + CIRCULAR_DATATAIL_MAX);

	bool bEnough = (emptyEnd - emptyStart) >= dataSize;

	if ( bEnough ) {
		memcpy(emptyStart, data, dataSize);
		mDataTail += dataSize;
	}
	else if ( bLinear && ! IsUsingExtraBuffer() ) {
		bEnough = DoWriteSeparate(data, dataSize);
	}

	CS_UNLOCK

	return bEnough;
}

bool CircleBuffer::DoWriteSeparate(char* data, int size)
{
	ASSERT(!IsCircularData());

	int tail_end = (mCircleEnd - GetDataTail());
	int start_head = CIRCULAR_DATATAIL_MAX;

	bool bEnough = ( tail_end + start_head ) >= size;

	if ( bEnough ) {   //Separate Tail-End, Start-Head
		int start_tail = (size - tail_end);
		memcpy(GetDataTail(), data, tail_end);
		memcpy(mCircleStart, (data + tail_end), start_tail);
		mDataTail = start_tail;
	}
	return bEnough;
}

char* CircleBuffer::Read(IN OUT int* requestSize, bool canBeResized, bool bCircularMerge)
{
	CS_LOCK
	
	char* resultBuf = NULL;
	bool bLinear = ! IsCircularData();
	char* continousDataEnd = bLinear ? GetDataTail() : mCircleEnd;

	size_t dataSize = (continousDataEnd - GetDataHead());
	bool bDataEnough = dataSize >= (size_t)*requestSize;

	if ( bDataEnough ) {
		resultBuf = (mCircleStart + mDataHead);
	}
	else if (canBeResized && (dataSize > 0) ) {
		resultBuf = (mCircleStart + mDataHead);
		*requestSize = dataSize;
	}
	
	if ( (resultBuf == NULL) && ! bLinear && bCircularMerge && ! IsUsingExtraBuffer() ) {    
		resultBuf = DoReadAndMergeData(requestSize, canBeResized);	//Separate End-Start. Merge it! (Make it Linear by Using Extra Buffer)
	}

	CS_UNLOCK

	return resultBuf;
}

//Use Only Separated Data. Make buffer Linear
char* CircleBuffer::DoReadAndMergeData(int* reqSize, bool canBeResized)
{
	int head_end_size = (mCircleEnd - (mCircleStart + mDataHead));
	bool bResult = false;

	char* resultBuf = NULL;
	if ( head_end_size <= mBufferExtraSize ) {

		int start_tail_size = (mDataTail);
		int dataFullSize = (head_end_size + start_tail_size);

		bool bDataEnough = dataFullSize >= (*reqSize);

		char* newHeadPos = (mCircleStart - head_end_size);
		memcpy(newHeadPos, mCircleStart + mDataHead, head_end_size);

		if ( bDataEnough ) {
			resultBuf = newHeadPos;	
			mDataHead = -(int)head_end_size;//Set New Head Position in Extra Buffer
			bResult = true;
		}
		else if (canBeResized && (dataFullSize > 0) ) {
			resultBuf = newHeadPos;
			mDataHead = -(int)head_end_size;//Set New Head Position in Extra Buffer
			*reqSize = dataFullSize;
			bResult = true;
		}
	}

	return resultBuf;
}

//Set DataTail without data-memcpy (Linear) :  Using for OnRecvComplete.
bool CircleBuffer::AddDataTail(int size)
{
	CS_LOCK

	bool bLinear = ! IsCircularData();
	char* emptyStart = GetDataTail();
	char* emptyEnd = bLinear ? mCircleEnd : (mCircleStart + CIRCULAR_DATATAIL_MAX);
	bool bEnough = (emptyEnd - emptyStart) >= size;

	if ( bEnough ) {
		mDataTail += size;
		if (mDataTail >= mBufferSize) {
			if ( mDataHead > 0 )
				mDataTail = 0;
		}
	}

	CS_UNLOCK

	return bEnough;
}

//For Recv
char* CircleBuffer::GetEmpty(int* requiredSize)	// size == 0 is Maximum Size as Possible
{
	CS_LOCK
	if ( mDataHead == mDataTail ) {
		mDataHead = mDataTail = 0;	// == ClearAll();
	}

	bool bLinear = ! IsCircularData();
	if ( bLinear && (mDataTail == mBufferSize) && (0 < mDataHead) ) {   //Make Circular
		mDataTail = 0;
		bLinear = false;
	}

	char* resultBuf = NULL;
	char* emptyStart = GetDataTail();
	char* emptyEnd = bLinear ? mCircleEnd : (mCircleStart + CIRCULAR_DATATAIL_MAX);
	bool bEnough = (emptyEnd - emptyStart) >= *requiredSize;
	bool muchAsPossible = ((*requiredSize) == 0);

	if ( bEnough ) {
		resultBuf = emptyStart;

		if ( muchAsPossible ) {
			int emptySize = (emptyEnd - emptyStart);
			(*requiredSize) = emptySize;	//size out
			bEnough = emptySize > 0;
		}
	}
	CS_UNLOCK

	return resultBuf;
}

bool CircleBuffer::ClearData(int size)
{
	char* dataHead = Read(&size, false, false);

	CS_LOCK
	if ( dataHead ) {
		mDataHead += size;
		/*
		ASSERT(mDataHead <= mDataTail);
		if ( mDataHead == mDataTail) {
			mDataHead = mDataTail = 0;
		}
		*/
	}
	CS_UNLOCK
	return (dataHead && (size > 0));
}

int CircleBuffer::GetDataSize()
{
	int size = 0;
	if ( IsCircularData() ) {
		size = (mBufferSize - mDataHead) + (mDataTail);
	} 
	else {
		size = (mDataTail - mDataHead);
	}

	return size;
}
