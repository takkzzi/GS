#pragma once

#include "ThreadSync.h"

namespace Core
{

/* Memory Leak 문제로 사용하지 말것.
template <class T, int ALLOC_BLOCK_SIZE = 50>
class MemPoolObject : public ThreadSyncStatic<T>
{
public:

	static VOID* operator new (std::size_t allocLength)
	{
		ThreadSync sync;

		ASSERT(sizeof(T) == allocLength);
		ASSERT(sizeof(T) >= sizeof(UCHAR*));

		if (!mFreePointer)
			AllocBlock();

		UCHAR *ReturnPointer = mFreePointer;
		mFreePointer = *reinterpret_cast<UCHAR**>(ReturnPointer);

		return ReturnPointer;
	}

	static VOID	operator delete(VOID* deletePointer)
	{
		ThreadSync sync;

		*reinterpret_cast<UCHAR**>(deletePointer) = mFreePointer;
		mFreePointer = static_cast<UCHAR*>(deletePointer);
	}

	static VOID DeallocAll()
	{
		ThreadSync sync;

		delete[] mBeginPointer;
		mBeginPointer = NULL;
		mFreePointer = NULL;
	}

private:
	static VOID	AllocBlock()
	{
		ThreadSync sync;

		mBeginPointer	= new UCHAR[sizeof(T) * ALLOC_BLOCK_SIZE];
		mFreePointer	= mBeginPointer;

		UCHAR **Current = reinterpret_cast<UCHAR **>(mFreePointer);
		UCHAR *Next		= mFreePointer;

		for (INT i=0;i<ALLOC_BLOCK_SIZE-1;++i)
		{
			Next		+= sizeof(T);
			*Current	= Next;
			Current		= reinterpret_cast<UCHAR**>(Next);
		}

		*Current = 0;
	}

private:
	static UCHAR*				mFreePointer;
	static UCHAR*				mBeginPointer;

	//static ThreadSync			mSync;

protected:
	~MemPoolObject() {}
};

template <class T, int ALLOC_BLOCK_SIZE>
UCHAR* MemPoolObject<T, ALLOC_BLOCK_SIZE>::mFreePointer = NULL;

template <class T, int ALLOC_BLOCK_SIZE>
UCHAR* MemPoolObject<T, ALLOC_BLOCK_SIZE>::mBeginPointer = NULL;
*/

}

