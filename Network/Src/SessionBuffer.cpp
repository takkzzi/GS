#include "PCH.h"
#include "SessionBuffer.h"

using namespace Network;

#define		CS_LOCK		mCriticalSec.Enter();
#define		CS_UNLOCK		mCriticalSec.Leave();


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
	mCriticalSec.Enter();
	delete[] mBuffer;
	mCriticalSec.Leave();
}

void CircularBuffer::Init(int size, int extraBufferSize)
{
	mCriticalSec.Enter();

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
	
	mCriticalSec.Leave();

	ClearAll();
}

void CircularBuffer::ClearAll()
{
	mCriticalSec.Enter();
	mDataHead = mDataTail = 0;
	mCriticalSec.Leave();
}

bool CircularBuffer::Write(char* data, size_t size)
{	
	mCriticalSec.Enter();

	bool bLinear = ! IsCircularData();
	char* emptyStart = mCircleStart + mDataTail;
	char* emptyEnd = bLinear ? mCircleEnd : (mCircleStart + mDataHead);

	bool bEnough = (size_t)(emptyEnd - emptyStart) >= size;

	if ( bEnough ) {
		memcpy(emptyStart, data, size);
		mDataTail += size;
	}
	else if ( bLinear && ! IsUsingExtraBuffer() ) {
		bEnough = DoWriteSeparate(data, size);
	}

	mCriticalSec.Leave();

	return bEnough;
}

bool CircularBuffer::DoWriteSeparate(char* data, size_t size)
{
	size_t tail_end = (mCircleEnd - (mCircleStart + mDataTail));
	size_t start_head = (mDataHead);
	ASSERT(tail_end > 0 && start_head > 0);
	bool bEnough = ( tail_end + start_head ) >= (size_t)size;

	if ( bEnough ) {   //Separate Tail-End, Start-Head
		size_t start_tail = (size - tail_end);
		memcpy(mCircleStart + mDataTail, data, tail_end);
		memcpy(mCircleStart, (data + tail_end), start_tail);
		mDataTail = start_tail;
	}
	return bEnough;
}

//Set Data without memcpy (Linear);
bool CircularBuffer::Write(size_t size) 
{
	mCriticalSec.Enter();

	bool bLinear = ! IsCircularData();
	char* emptyStart = mCircleStart + mDataTail;
	char* emptyEnd = bLinear ? mCircleEnd : (mCircleStart + mDataHead);
	bool bEnough = (size_t)(emptyEnd - emptyStart) >= size;

	if ( bEnough ) {
		mDataTail += size;
	}

	mCriticalSec.Leave();

	return bEnough;
}

/*
//Linear Data Only
int CircularBuffer::GetData(char** bufPtr)
{
	mCriticalSec.Enter();
	bool bCircualrSeparate = (mDataHead > mDataTail);
	char* dataHead = mDataHead;
	char* dataTail = bCircualrSeparate ? mCircleEnd : mDataTail;
	*bufPtr = dataHead;
	int size = (dataTail - dataHead);
	mCriticalSec.Leave();
	return size;
}
*/

bool CircularBuffer::Read(char** bufPtr, int* reqSize, bool bResize, bool bCircularMerge)
{
	mCriticalSec.Enter();
	
	*bufPtr = NULL;
	bool bResult = false;
	bool bLinear = ! IsCircularData();

	if ( bLinear ) {
		size_t dataSize = (mDataTail - mDataHead);
		bool bDataEnough = dataSize >= (size_t)*reqSize;
		*bufPtr = (mCircleStart + mDataHead);

		if ( bDataEnough ) {
			bResult = true;
		}
		else if ( bResize && (dataSize > 0) ) {
			*reqSize = dataSize;
			bResult = true;
		}
	}
	else if ( bCircularMerge && ! IsUsingExtraBuffer() ) {    //Separate End-Start. Merge it! (Make it Linear by Using Extra Buffer)
		bResult = DoGetAndMergeData(bufPtr, reqSize, bResize);
	}

	mCriticalSec.Leave();

	return bResult;
}

//Use Only Separated Data. Make buffer Linear
bool CircularBuffer::DoGetAndMergeData(char** bufPtr, int* reqSize, bool bResize)
{
	size_t head_end_size = (mCircleEnd - (mCircleStart + mDataHead));
	bool bResult = false;

	if ( head_end_size <= (size_t)mBufferExtraSize ) {

		size_t start_tail_size = (mDataTail);
		size_t dataFullSize = (head_end_size + start_tail_size);

		bool bDataEnough = dataFullSize >= (size_t)(*reqSize);

		char* newHeadPos = (mCircleStart - head_end_size);
		memcpy(newHeadPos, mCircleStart + mDataHead, head_end_size);

		if ( bDataEnough ) {
			*bufPtr = newHeadPos;	
			mDataHead = -(int)head_end_size;//Set New Head Position in Extra Buffer
			bResult = true;
		}
		else if ( bResize && (dataFullSize > 0) ) {
			*bufPtr = newHeadPos;
			mDataHead = -(int)head_end_size;//Set New Head Position in Extra Buffer
			*reqSize = dataFullSize;
			bResult = true;
		}
	}

	return bResult;
}

//For Recv
bool CircularBuffer::GetEmpty(char** bufPtr, int* size)
{
	mCriticalSec.Enter();

	bool bLinear = ! IsCircularData();
	if ( bLinear && (mDataTail == mBufferSize) && (0 < mDataHead) ) {   //Make Circular
		mDataTail = 0;
		bLinear = false;
	}

	char* emptyStart = mCircleStart + mDataTail;
	char* emptyEnd = bLinear ? mCircleEnd : (mCircleStart + mDataHead);
	bool bEnough = (emptyEnd - emptyStart) >= *size;
	bool muchAsPossible = ((*size) == 0);

	if ( bEnough ) {
		*bufPtr = emptyStart;

		if ( muchAsPossible ) {
			int emptySize = (emptyEnd - emptyStart);
			(*size) = emptySize;	//size out
			bEnough = emptySize > 0;
		}
	}
	mCriticalSec.Leave();
	return bEnough;
}

bool CircularBuffer::ClearData(int size)
{
	char* dataHead = NULL;
	bool bGet = Read(&dataHead, &size, false, false);

	mCriticalSec.Enter();
	if ( bGet ) {
		mDataHead += size;
		ASSERT(mDataHead <= mDataTail);
		if ( mDataHead == mDataTail) {
			mDataHead = mDataTail = 0;
		}
	}
	mCriticalSec.Leave();
	return bGet;
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

/*
//SendBuffer ///////////////////////////////////////////////////////////////////////////////////
bool SendBuffer::OnIoComplete(char* bufPtr, DWORD transferBytes)
{
	mCriticalSec.Enter();
	mCriticalSec.Leave();
	return true;
}


//RecvBuffer ///////////////////////////////////////////////////////////////////////////////////
bool RecvBuffer::OnIoComplete(char* bufPtr, DWORD transferBytes)
{
	mCriticalSec.Enter();
	mCriticalSec.Leave();
	return true;
}
*/
