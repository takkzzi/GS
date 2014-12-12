#include "PCH.h"
#include "SessionBuffer.h"

using namespace Network;



SessionBuffer::SessionBuffer()
	: mSession(NULL)
	, mBufferSize(0)
{
}

SessionBuffer::~SessionBuffer()
{
	for(auto &i : mBuffers)
		delete[] i;
}

void SessionBuffer::Init(Session* sess, OverlappedIoType ioType, int bufferCount, int bufferSize)
{
	Clear();

	if ( mBuffers.size() > 0 )
		return;

	mSession = sess;
	mIOType = ioType;
	mBufferSize = bufferSize;

	mBuffers.reserve(bufferCount);
	for(int i = 0; i < bufferCount; ++i) {
		BufferItem* newBuf = new BufferItem(sess, ioType, bufferSize, i);
		mBuffers.push_back(newBuf);
		//mBufferMap[newBuf->mOverlapped] = newBuf;
	}
}

void SessionBuffer::Clear()
{
	for(auto &i : mBuffers) {
		i->Clear();
	}
}

bool SessionBuffer::PushCopy(char* data)
{
	BufferItem* newBuffer = GetEmptyBuffer();
	newBuffer->Push(data);
	return true;
}

BufferItem*	SessionBuffer::GetEmptyBuffer()
{
	for(auto &i : mBuffers) {
		if ( i->mWsaBuf.len == 0 )
			return i;
	}

	Logger::Log("SessionBuffer", "SessionBuffer::GetEmptyBuffer() -- No Empty Buffer. Create New One.");
	BufferItem* newBuffer = new BufferItem(mSession, mIOType, mBufferSize, mBuffers.size());
	mBuffers.push_back(newBuffer);
	return newBuffer;
}

void SessionBuffer::OnSend(Overlapped* overlapped, int byteCount)
{
	ASSERT(overlapped);
	BufferItem* buff = (BufferItem*)overlapped;
	ASSERT(buff->mWsaBuf.len == byteCount);
	if ( buff->mWsaBuf.len == byteCount ) {
		buff->Clear();
	}
}

void SessionBuffer::OnRecv(Overlapped* overlapped, int byteCount)
{
	//Just Check
	ASSERT(overlapped);
	BufferItem* buff = (BufferItem*)overlapped;
	if ( buff->mWsaBuf.len != byteCount ) {
		Logger::Log("SessionBuffer", "OnRecv() -- Data Lenth Different");
	}
}

BufferItem* SessionBuffer::GetFilledBuffer()
{
	for(auto &i : mBuffers) {
		if ( i->mWsaBuf.len > 0 )
			return i;
	}

	return NULL;
}

void SessionBuffer::ClearBuffer(BufferItem* item)
{
	ASSERT(item);
	mBuffers[item->mIndex]->Clear();
}

