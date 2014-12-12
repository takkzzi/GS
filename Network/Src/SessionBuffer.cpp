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

void SessionBuffer::Init(Session* sess, int bufferCount, int bufferSize)
{
	Clear();

	if ( mBuffers.size() > 0 )
		return;

	mSession = sess;
	mBufferSize = bufferSize;

	mBuffers.reserve(bufferCount);
	for(int i = 0; i < bufferCount; ++i)
		mBuffers.push_back(new char[bufferSize]);

	mBufferIndexList.clear();;
}

int SessionBuffer::Clear()
{
	for(auto &i : mBuffers) {
		::ZeroMemory(i, mBufferSize);
	}

	int count = mBufferIndexList.size();
	mBufferIndexList.clear();

	return count;
}

bool SessionBuffer::Push(char* data)
{
	return false;
}

char* SessionBuffer::GetBuffer(int idx)
{
	return NULL;
}

bool SessionBuffer::RemoveData(int idx)
{
	ASSERT(mBuffers.size() > idx);

	::ZeroMemory(mBuffers[idx], mBufferSize);

	return false;
}

SessionBuffer::BufferItem*	SessionBuffer::GetEmptyBuffer()
{
	return NULL;
}

