#include "PCH.h"
#include "SessionDataQueue.h"

using namespace Network;



SessionDataQueue::SessionDataQueue()
	: mSession(NULL)
	, mDataBufferSize(0)
{
}

SessionDataQueue::~SessionDataQueue()
{
	for(auto &i : mDataVec)
		delete i;
}

void SessionDataQueue::Init(Session* sess, OverlappedIoType ioType, int bufferCount, int bufferSize)
{
	Clear();

	if ( mDataVec.size() > 0 )
		return;

	mSession = sess;
	mIOType = ioType;
	mDataBufferSize = bufferSize;

	mDataVec.reserve(bufferCount * 2);
	for(int i = 0; i < bufferCount; ++i) {
		OverlappedData* newData = new OverlappedData(sess, ioType, bufferSize, i);
		mDataVec.push_back(newData);
	}
}

void SessionDataQueue::Clear()
{
	for(auto &i : mDataVec) {
		i->Clear();
	}
}

void SessionDataQueue::Push(char* data, int length)
{
	OverlappedData* oldata = GetEmptyData();
	oldata->Push(data, length);
	mDataQueue.push_back(oldata);
}

OverlappedData* SessionDataQueue::Pop()
{
	if ( mDataQueue.size() <= 0 )
		return NULL;

	OverlappedData* data = mDataQueue.front();
	mDataQueue.pop_front();

	bool bFilled = (data->wsaBuf.len > 0);
	ASSERT(bFilled);
	data->Clear();	//Data Buffer is Still Alive
	return data;
}

OverlappedData* SessionDataQueue::GetEmptyData()
{
	for(auto &i : mDataVec) {
		if ( i->wsaBuf.len == 0 )
			return i;
	}

	Logger::Log("SessionBuffer", "SessionBuffer::GetEmptyBuffer() -- No Empty Buffer. Create New One.");

	OverlappedData* newData = new OverlappedData(mSession, mIOType, mDataBufferSize, mDataVec.size());
	mDataVec.push_back(newData);
	return newData;
}

void SessionDataQueue::OnSendComplete(OverlappedData* olData, int byteCount)
{
	ASSERT(olData && (olData->wsaBuf.len == byteCount));
	if ( olData->wsaBuf.len == byteCount ) {
		olData->Clear();
		mDataQueue.pop_front();
	}
}

void SessionDataQueue::OnRecvComplete(OverlappedData* olData, int byteCount)
{
	//Just Check
	ASSERT(olData && (olData->wsaBuf.len == byteCount));
	mDataQueue.push_back(olData);
}

