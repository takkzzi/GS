#include "PCH.h"
#include "SessionBuffer.h"

using namespace Network;


SessionBufferQueue::SessionBufferQueue()
	: mBufferLen(0)
	, mBuffer(NULL)
{
}

SessionBufferQueue::~SessionBufferQueue()
{
	mCriticalSec.Enter();
	delete[] mBuffer;
	mCriticalSec.Leave();
}

void SessionBufferQueue::Init(int bufferCount, int bufferLength)
{
	mCriticalSec.Enter();

	int newBufferLen = bufferCount*bufferLength;
	//Ring buffer
	if(mBuffer && (newBufferLen != mBufferLen)) {
		delete[] mBuffer;
		mBuffer = NULL;
	}
	
	if ( !mBuffer) {
		mBuffer = new char[mBufferLen];
	}
	mBufferLen = newBufferLen;

	mBufferStart = &mBuffer[0];
	mBufferEnd = mBufferStart + mBufferLen;

	mCriticalSec.Leave();

	ClearAll();
}

void SessionBufferQueue::ClearAll()
{
	mCriticalSec.Enter();
	mDataHead = mDataTail = mBufferStart;
	mCriticalSec.Leave();
}

bool SessionBufferQueue::Push(char* data, int size)
{
	mCriticalSec.Enter();
	
	bool bLinear = (mDataHead <= mDataTail);
	char* emptyStart = mDataTail;
	char* emptyEnd = bLinear ? mBufferEnd : mDataHead;

	bool bEnough = (emptyEnd - emptyStart) >= size;

	if ( bEnough ) {
		memcpy(emptyStart, data, size);
		mDataTail += size;
	}
	else if ( bLinear ) {    //Separating Situation
		int tail_end = (mBufferEnd - mDataTail);
		int start_head = (mDataHead - mBufferStart);
		bEnough = ( tail_end + start_head ) >= size;

		if ( bEnough ) {   //Separate Tail-End, Start-Head
			memcpy(mDataTail, data, tail_end);
			memcpy(mBufferStart, data + tail_end, start_head);
			mDataTail = mBufferStart + start_head;
		}
	}

	mCriticalSec.Leave();
	return bEnough;
}

int SessionBufferQueue::GetData(char** bufPtr)
{
	bool bCircualrSeparate = (mDataHead > mDataTail);
	char* dataHead = mDataHead;
	char* dataTail = bCircualrSeparate ? mBufferEnd : mDataTail;
	*bufPtr = dataHead;
	int size = (dataTail - dataHead);
	return size;
}

//
bool SessionBufferQueue::GetEmpty(char** bufPtr, int* size)
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

bool SessionBufferQueue::ClearData(int len)
{
	bool bCircualrSeparate = (mDataHead > mDataTail);
	char* dataHead = mDataHead;
	char* dataTail = bCircualrSeparate ? mBufferEnd : mDataTail;

	return true;
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
