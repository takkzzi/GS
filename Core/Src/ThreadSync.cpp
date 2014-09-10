#include "PCH.h"
#include "ThreadSync.h"

using namespace Core;

ThreadSync::ThreadSync()
{
}

ThreadSync::~ThreadSync()
{
}

void ThreadSync::Enter()
{
	mCS.Enter();
}

void ThreadSync::Leave()
{
	mCS.Leave();
}

