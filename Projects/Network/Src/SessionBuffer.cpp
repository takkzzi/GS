#include "PCH.h"
#include "SessionBuffer.h"

using namespace Network;

#define		CS_LOCK					mCritiSect.Enter();
#define		CS_UNLOCK				mCritiSect.Leave();
#define		CIRCULAR_DATATAIL_MAX	(mDataHead - 1)

CircularBuffer::CircularBuffer()
	: mBufferSize(0)
	, mBufferExtraSize(0)
	, mBuffer(NULL)
	, mCircleStart(0)
	, mCircleEnd(0)
	, mDataHead(0)
	, mDataTail(0)
{
}

CircularBuffer::~CircularBuffer()
{
	CS_LOCK
	delete[] mBuffer;
	CS_UNLOCK
}

void CircularBuffer::Init(int size, int extraBufferSize)
{
	CS_LOCK

	int newBufferTotal = extraBufferSize + size;
	int oldBufferTotal = mBufferSize + mBufferExtraSize;
	
	//Ring buffer
	if(mBuffer && (oldBufferTotal != newBufferTotal)) {
		delete[] mBuffer;
		mBuffer = NULL;
	}
	
	if ( !mBuffer) {
		mBuffer = new char[newBufferTotal];
	}

	mBufferSize = size;
	mBufferExtraSize = extraBufferSize;

	mCircleStart = mBuffer + extraBufferSize;
	mCircleEnd = mCircleStart + mBufferSize;
	
	CS_UNLOCK

	ClearAll();
}

void CircularBuffer::ClearAll()
{
	CS_LOCK
	mDataHead = mDataTail = 0;
	CS_UNLOCK
}

bool CircularBuffer::Write(char* data, size_t size)
{	
	CS_LOCK

	bool bLinear = ! IsCircularData();
	char* emptyStart = mCircleStart + mDataTail;
	char* emptyEnd = bLinear ? mCircleEnd : (mCircleStart + CIRCULAR_DATATAIL_MAX);

	bool bEnough = (size_t)(emptyEnd - emptyStart) >= size;

	if ( bEnough ) {
		memcpy(emptyStart, data, size);
		mDataTail += size;
	}
	else if ( bLinear && ! IsUsingExtraBuffer() ) {
		bEnough = DoWriteSeparate(data, size);
	}

	CS_UNLOCK

	return bEnough;
}

bool CircularBuffer::DoWriteSeparate(char* data, size_t size)
{
	size_t tail_end = (mCircleEnd - (mCircleStart + mDataTail));
	size_t start_head = CIRCULAR_DATATAIL_MAX;

	bool bEnough = ( tail_end + start_head ) >= (size_t)size;

	if ( bEnough ) {   //Separate Tail-End, Start-Head
		size_t start_tail = (size - tail_end);
		memcpy(mCircleStart + mDataTail, data, tail_end);
		memcpy(mCircleStart, (data + tail_end), start_tail);
		mDataTail = start_tail;
	}
	return bEnough;
}

char* CircularBuffer::Read(int* reqSize, bool bResize, bool bCircularMerge)
{
	CS_LOCK
	
	char* resultBuf = NULL;
	bool bLinear = ! IsCircularData();
	char* continousDataEnd = bLinear ? GetDataTail() : mCircleEnd;

	size_t dataSize = (continousDataEnd - GetDataHead());
	bool bDataEnough = dataSize >= (size_t)*reqSize;

	if ( bDataEnough ) {
		resultBuf = (mCircleStart + mDataHead);
	}
	else if ( bResize && (dataSize > 0) ) {
		resultBuf = (mCircleStart + mDataHead);
		*reqSize = dataSize;
	}
	
	if ( (resultBuf == NULL) && ! bLinear && bCircularMerge && ! IsUsingExtraBuffer() ) {    
		resultBuf = DoReadAndMergeData(reqSize, bResize);	//Separate End-Start. Merge it! (Make it Linear by Using Extra Buffer)
	}

	CS_UNLOCK

	return resultBuf;
}

//Use Only Separated Data. Make buffer Linear
char* CircularBuffer::DoReadAndMergeData(int* reqSize, bool bResize)
{
	size_t head_end_size = (mCircleEnd - (mCircleStart + mDataHead));
	bool bResult = false;

	char* resultBuf = NULL;
	if ( head_end_size <= (size_t)mBufferExtraSize ) {

		size_t start_tail_size = (mDataTail);
		size_t dataFullSize = (head_end_size + start_tail_size);

		bool bDataEnough = dataFullSize >= (size_t)(*reqSize);

		char* newHeadPos = (mCircleStart - head_end_size);
		memcpy(newHeadPos, mCircleStart + mDataHead, head_end_size);

		if ( bDataEnough ) {
			resultBuf = newHeadPos;	
			mDataHead = -(int)head_end_size;//Set New Head Position in Extra Buffer
			bResult = true;
		}
		else if ( bResize && (dataFullSize > 0) ) {
			resultBuf = newHeadPos;
			mDataHead = -(int)head_end_size;//Set New Head Position in Extra Buffer
			*reqSize = dataFullSize;
			bResult = true;
		}
	}

	return resultBuf;
}

//Set DataTail without data-memcpy (Linear);
bool CircularBuffer::AddDataTail(size_t size) 
{
	CS_LOCK

	bool bLinear = ! IsCircularData();
	char* emptyStart = mCircleStart + mDataTail;
	char* emptyEnd = bLinear ? mCircleEnd : (mCircleStart + CIRCULAR_DATATAIL_MAX);
	bool bEnough = (size_t)(emptyEnd - emptyStart) >= size;

	if ( bEnough ) {
		mDataTail += size;
	}

	CS_UNLOCK

	return bEnough;
}

//For Recv
char* CircularBuffer::GetEmpty(int* size)	// size == 0 is Maximum Size as Possible
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
	char* emptyStart = mCircleStart + mDataTail;
	char* emptyEnd = bLinear ? mCircleEnd : (mCircleStart + CIRCULAR_DATATAIL_MAX);
	bool bEnough = (emptyEnd - emptyStart) >= *size;
	bool muchAsPossible = ((*size) == 0);

	if ( bEnough ) {
		resultBuf = emptyStart;

		if ( muchAsPossible ) {
			int emptySize = (emptyEnd - emptyStart);
			(*size) = emptySize;	//size out
			bEnough = emptySize > 0;
		}
	}
	CS_UNLOCK

	return resultBuf;
}

bool CircularBuffer::ClearData(int size)
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

size_t CircularBuffer::GetDataSize()
{
	size_t size = 0;
	if ( IsCircularData() ) {
		size = (mBufferSize - mDataHead) + (mDataTail);
	} 
	else {
		size = (mDataTail - mDataHead);
	}

	return size;
}