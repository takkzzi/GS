#include "PCH.h"
#include "SessionBuffer.h"

using namespace Network;


SessionBufferQueue::SessionBufferQueue()
	: mBufferLen(0)
{
}

SessionBufferQueue::~SessionBufferQueue()
{
	mCriticalSec.Enter();
	for(auto &i : mBufferVec)
		delete i;

	mCriticalSec.Leave();
}

void SessionBufferQueue::Init(int bufferCount, int bufferLength)
{
	Clear();

	mCriticalSec.Enter();
	if ( mBufferVec.size() > 0 ) {
		mCriticalSec.Leave();
		return;
	}

	mBufferLen = bufferLength;

	mBufferVec.reserve(bufferCount * 1.2f);
	for(int i = 0; i < bufferCount; ++i) {
		SessionBuffer* newBuf = new SessionBuffer(bufferLength, i);
		mBufferVec.push_back(newBuf);
	}

	mCriticalSec.Leave();
}

void SessionBufferQueue::Clear()
{
	mCriticalSec.Enter();
	for(auto &i : mBufferVec) {
		i->Clear();
	}
	mCriticalSec.Leave();
}

void SessionBufferQueue::Push(char* data, int length)
{
	SessionBuffer* buf = GetEmpty();

	mCriticalSec.Enter();

	::CopyMemory(buf->buf, data, length);
	buf->len = length;
	mBufferQ.push_back(buf);

	mCriticalSec.Leave();
}


SessionBuffer* SessionBufferQueue::Pop(bool bClearBuffer)
{
	mCriticalSec.Enter();
	SessionBuffer* buf = NULL;
	
	if (mBufferQ.size() > 0) {
		buf = mBufferQ.front();
		mBufferQ.pop_front();
		if ( bClearBuffer )
			buf->Clear();
	}
	mCriticalSec.Leave();
	return buf;
}

SessionBuffer* SessionBufferQueue::GetFront()
{
	mCriticalSec.Enter();
	SessionBuffer* frontbuf = NULL;
	if (mBufferQ.size() > 0)
		frontbuf = mBufferQ.front();
	mCriticalSec.Leave();
	return frontbuf;
}

SessionBuffer* SessionBufferQueue::GetEmpty()
{
	mCriticalSec.Enter();
	SessionBuffer* newBuf = NULL;
	for(auto &i : mBufferVec) {
		if ( i->len == 0 ) {
			newBuf = i;
			break;
		}
	}

	if ( ! newBuf ) {
		Logger::Log("SessionBuffer", "GetEmpty() Create New One.(%d)", mBufferVec.size());

		newBuf = new SessionBuffer(mBufferLen, mBufferVec.size());
		mBufferVec.push_back(newBuf);
	}
	mCriticalSec.Leave();
	return newBuf;
}


//SendBufferQueue ///////////////////////////////////////////////////////////////////////////////////
bool SendBufferQueue::OnIoComplete(SessionBuffer* buffer, DWORD transferBytes)
{
	mCriticalSec.Enter();
	ASSERT((mBufferQ.front() == buffer) && (buffer->len == transferBytes));
	bool bValid = (mBufferQ.front() == buffer) && (buffer->len == transferBytes);
	if ( bValid ) {
		SessionBuffer* popBuffer = Pop(true);
	}
	mCriticalSec.Leave();
	return bValid;
}


//RecvBufferQueue ///////////////////////////////////////////////////////////////////////////////////
bool RecvBufferQueue::OnIoComplete(SessionBuffer* buffer, DWORD transferBytes)
{
	mCriticalSec.Enter();
	ASSERT(mBufferVec[buffer->index] == buffer);

	bool bValid = (mBufferVec[buffer->index] == buffer);
	if ( bValid ) {
		buffer->len = transferBytes;
		mBufferQ.push_back(buffer);
	}
	mCriticalSec.Leave();
	return bValid;
}

SessionBuffer* RecvBufferQueue::GetEmpty()
{
	SessionBuffer* emptyBuff = SessionBufferQueue::GetEmpty();
	mCriticalSec.Enter();
	emptyBuff->len = mBufferLen;	//Reserve Receiving
	mCriticalSec.Leave();
	return emptyBuff;
}