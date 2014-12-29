#include "PCH.h"
#include "SessionBuffer.h"

using namespace Network;

#define		CS_LOCK		mCriticalSec.Enter();
#define		CS_UNLOCK		mCriticalSec.Leave();


CircularBuffer::CircularBuffer()
	: mBufferSize(0)
	, mBufferExtraSize(0)
	, mBuffer(NULL)
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

	mBufferStart = mBuffer + extraBufferSize;
	mBufferEnd = mBufferStart + mBufferSize;
	
	mCriticalSec.Leave();

	ClearAll();
}

void CircularBuffer::ClearAll()
{
	mCriticalSec.Enter();
	mDataHead = mDataTail = mBufferStart;
	mCriticalSec.Leave();
}

bool CircularBuffer::Write(char* data, int size)
{	
	mCriticalSec.Enter();

	bool bLinear = ! IsCircularData();
	char* emptyStart = mDataTail;
	char* emptyEnd = bLinear ? mBufferEnd : mDataHead;

	bool bEnough = (emptyEnd - emptyStart) >= size;

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

bool CircularBuffer::DoWriteSeparate(char* data, int size)
{
	size_t tail_end = (mBufferEnd - mDataTail);
	size_t start_head = (mDataHead - mBufferStart);
	bool bEnough = ( tail_end + start_head ) >= (size_t)size;

	if ( bEnough ) {   //Separate Tail-End, Start-Head
		memcpy(mDataTail, data, tail_end);
		memcpy(mBufferStart, data + tail_end, start_head);
		mDataTail = mBufferStart + start_head;
	}
	return bEnough;
}

//Set Data without memcpy (Linear);
bool CircularBuffer::Write(int size) 
{
	mCriticalSec.Enter();

	bool bLinear = ! IsCircularData();
	char* emptyStart = mDataTail;
	char* emptyEnd = bLinear ? mBufferEnd : mDataHead;
	bool bEnough = (emptyEnd - emptyStart) >= size;

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
	char* dataTail = bCircualrSeparate ? mBufferEnd : mDataTail;
	*bufPtr = dataHead;
	int size = (dataTail - dataHead);
	mCriticalSec.Leave();
	return size;
}
*/

bool CircularBuffer::Read(char** bufPtr, int* reqSize, bool bResize, bool bCircularMerge)
{
	mCriticalSec.Enter();
	
	bool bLinear = ! IsCircularData();
	bool bResult = false;
	if ( bLinear ) {
		size_t dataSize = (mDataTail - mDataHead);
		bool bDataEnough = dataSize >= (size_t)*reqSize;

		if ( bDataEnough ) {
			*bufPtr = mDataHead;
			bResult = true;
		}
		else if ( bResize && (dataSize > 0) ) {
			*bufPtr = mDataHead;
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

//Use Only Separated Data.
bool CircularBuffer::DoGetAndMergeData(char** bufPtr, int* reqSize, bool bResize)
{
	size_t head_end_size = (mBufferEnd - mDataHead);
	bool bResult = false;

	if ( head_end_size <= (size_t)mBufferExtraSize ) {

		size_t start_tail_size = (mDataTail - mBufferStart);
		size_t dataFullSize = (head_end_size + start_tail_size);

		bool bDataEnough = dataFullSize >= (size_t)*reqSize;

		if ( bDataEnough ) {
			char* newHeadPos = (mBufferStart - head_end_size);
			memcpy(newHeadPos, mDataHead, head_end_size);
			*bufPtr = mDataHead = newHeadPos;	//Set New Head Position in Extra Buffer
			bResult = true;
		}
		else if ( bResize && (dataFullSize > 0) ) {
			char* newHeadPos = (mBufferStart - head_end_size);
			memcpy(newHeadPos, mDataHead, head_end_size);
			*bufPtr = mDataHead = newHeadPos;	//Set New Head Position in Extra Buffer
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
	if ( bLinear && (mDataTail == mBufferEnd) && (mBufferStart < mDataHead) ) {   //Converting Circular
		mDataTail = mBufferStart;
		bLinear = false;
	}

	char* emptyStart = mDataTail;
	char* emptyEnd = bLinear ? mBufferEnd : mDataHead;
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
			mDataHead = mDataTail = mBufferStart;
		}
	}
	mCriticalSec.Leave();
	return bGet;
}

size_t CircularBuffer::GetDataSize()
{
	size_t size = 0;
	if ( IsCircularData() ) {
		size = (mBufferEnd - mDataHead) + (mDataTail - mBufferStart);
	} 
	else {
		size = (mDataTail - mDataHead);
	}

	return (int)size;
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
