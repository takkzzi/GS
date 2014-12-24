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
	mCriticalSec.Enter();
	delete[] mBuffer;
	mCriticalSec.Leave();
}

void CircularBuffer::Init(int size, int dataMaxSize)
{
	mCriticalSec.Enter();

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
	
	mCriticalSec.Leave();

	ClearAll();
}

void CircularBuffer::ClearAll()
{
	mCriticalSec.Enter();
	mDataHead = mDataTail = mBufferStart;
	mCriticalSec.Leave();
}

bool CircularBuffer::PushData(char* data, int size)
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
		bEnough = DoPushSeparate(data, size);
	}

	mCriticalSec.Leave();

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

//Set Data without memcpy (Linear);
bool CircularBuffer::ReserveData(int size) 
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

bool CircularBuffer::GetData(char** bufPtr, int* reqSize, bool bResize, bool bCircularMerge)
{
	mCriticalSec.Enter();
	
	bool bLinear = ! IsCircularData();
	bool bResult = false;
	if ( bLinear ) {
		int dataSize = (mDataTail - mDataHead);
		bool bDataEnough = dataSize >= *reqSize;

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
	else if ( bCircularMerge && ! IsUsingExtraBuffer() ) {    //Separate End-Start. Merg it! (Make it Linear Using Extra Buffer)
		bResult = DoGetAndMergeData(bufPtr, reqSize, bResize);
	}

	mCriticalSec.Leave();

	return bResult;
}

//Use Only Separated Data.
bool CircularBuffer::DoGetAndMergeData(char** bufPtr, int* reqSize, bool bResize)
{
	int head_end_size = (mDataHead - mBufferEnd);
	bool bResult = false;

	if ( head_end_size <= mBufferExtraSize ) {
		int start_tail_size = (mBufferStart - mDataTail);
		int dataFullSize = (head_end_size + start_tail_size);

		bool bDataEnough = dataFullSize >= *reqSize;

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

//
bool CircularBuffer::GetEmpty(char** bufPtr, int* size)
{
	mCriticalSec.Enter();

	bool bLinear = ! IsCircularData();
	char* emptyStart = mDataTail;
	char* emptyEnd = bLinear ? mBufferEnd : mDataHead;
	bool bEnough = (emptyEnd - emptyStart) >= *size;
	bool muchAsPossible = ((*size) == 0);

	if ( bEnough ) {
		*bufPtr = emptyStart;

		if ( muchAsPossible ) {
			(*size) = (emptyEnd - emptyStart);	//Reassign size
		}
	}
	mCriticalSec.Leave();
	return bEnough;
}

bool CircularBuffer::ClearData(int size)
{
	char* dataHead = NULL;
	bool bGet = GetData(&dataHead, &size, false, false);

	mCriticalSec.Enter();
	if ( bGet ) {
		mDataHead += size;
		if ( mDataHead > mDataTail) {
			mDataHead = mDataTail = mBufferStart;
		}
	}
	mCriticalSec.Leave();
	return bGet;
}

int CircularBuffer::GetDataSize()
{
	int size = 0;
	if ( IsCircularData() ) {
		size = (mDataHead - mBufferEnd) + (mBufferStart - mDataTail);
	} 
	else {
		size = (mDataHead - mDataTail);
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
