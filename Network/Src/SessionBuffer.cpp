#include "PCH.h"
#include "SessionBuffer.h"

using namespace Network;


CircularBuffer::CircularBuffer()
	: mBufferSize(0)
	, mBufferExtraSize(0)
	, mBuffer(NULL)
{
}

CircularBuffer::~CircularBuffer()
{
	delete[] mBuffer;
}

void CircularBuffer::Init(int size, int dataMaxSize)
{
	int newBufferTotal = dataMaxSize + size;
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
	mBufferExtraSize = dataMaxSize;

	mBufferStart = mBuffer + dataMaxSize;
	mBufferEnd = mBufferStart + mBufferSize;

	ClearAll();
}

void CircularBuffer::ClearAll()
{
	mDataHead = mDataTail = mBufferStart;
}

bool CircularBuffer::Push(char* data, int size)
{	
	bool bLinear = (mDataHead <= mDataTail);
	char* emptyStart = mDataTail;
	char* emptyEnd = bLinear ? mBufferEnd : mDataHead;

	bool bEnough = (emptyEnd - emptyStart) >= size;

	if ( bEnough ) {
		memcpy(emptyStart, data, size);
		mDataTail += size;
	}
	else if ( bLinear && ! IsUsingExtraBuffer() ) {
		bEnough = DoPushSeparate(data, size);
	}

	return bEnough;
}

bool CircularBuffer::DoPushSeparate(char* data, int size)
{
	int tail_end = (mBufferEnd - mDataTail);
	int start_head = (mDataHead - mBufferStart);
	bool bEnough = ( tail_end + start_head ) >= size;

	if ( bEnough ) {   //Separate Tail-End, Start-Head
		memcpy(mDataTail, data, tail_end);
		memcpy(mBufferStart, data + tail_end, start_head);
		mDataTail = mBufferStart + start_head;
	}

	return bEnough;
}

int CircularBuffer::GetData(char** bufPtr)
{
	bool bCircualrSeparate = (mDataHead > mDataTail);
	char* dataHead = mDataHead;
	char* dataTail = bCircualrSeparate ? mBufferEnd : mDataTail;
	*bufPtr = dataHead;
	int size = (dataTail - dataHead);
	return size;
}

bool CircularBuffer::GetData(char** bufPtr, int reqSize, bool bCombineSeparate)
{
	bool bLinear = (mDataHead <= mDataTail);
	bool bResult = false;

	if ( bLinear ) {
		bool bDataEnough = (mDataTail - mDataHead) >= reqSize;
		if ( bDataEnough ) {
			*bufPtr = mDataHead;
			bResult = true;
		}
	}
	else if ( bCombineSeparate ) {    //Separate End-Start. Merg it! (Make it Linear Using Extra Buffer)
		bResult = DoGetAndMergeData(bufPtr, reqSize);
	}

	return bResult;
}

//Use Only Separated Data.
bool CircularBuffer::DoGetAndMergeData(char** bufPtr, int size)
{
	int head_end_size = (mDataHead - mBufferEnd);
	int start_tail_size = (mBufferStart - mDataTail);
	bool bDataEnough = (head_end_size + start_tail_size) >= size;

	if ( bDataEnough && head_end_size <= mBufferExtraSize ) {
		char* newHeadPos = (mBufferStart - head_end_size);
		memcpy(newHeadPos, mDataHead, head_end_size);
		*bufPtr = mDataHead = newHeadPos;	//Set New Head Position in Extra Buffer
		return true;
	}

	return false;
}

//
bool CircularBuffer::GetEmpty(char** bufPtr, int* size)
{
	bool bLinear = (mDataHead <= mDataTail);
	char* emptyStart = mDataTail;
	char* emptyEnd = bLinear ? mBufferEnd : mDataHead;
	bool bEnough = (emptyEnd - emptyStart) >= *size;
	
	if ( bEnough ) {
		*bufPtr = emptyStart;
	}
	return bEnough;
}

bool CircularBuffer::Clear(int size)
{
	char* dataHead = NULL;
	bool bGet = GetData(&dataHead, size, false);
	if ( bGet ) {
	}

	return bGet;
}



SessionBufferQueue::SessionBufferQueue()
{
}

SessionBufferQueue::~SessionBufferQueue()
{
	mCriticalSec.Enter();
	mCriticalSec.Leave();
}


//SendBufferQueue ///////////////////////////////////////////////////////////////////////////////////
bool SendBufferQueue::OnIoComplete(char* bufPtr, DWORD transferBytes)
{
	mCriticalSec.Enter();
	mCriticalSec.Leave();
	return true;
}


//RecvBufferQueue ///////////////////////////////////////////////////////////////////////////////////
bool RecvBufferQueue::OnIoComplete(char* bufPtr, DWORD transferBytes)
{
	mCriticalSec.Enter();
	mCriticalSec.Leave();
	return true;
}
