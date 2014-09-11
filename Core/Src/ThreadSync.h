#pragma once
#include "CriticalSection.h"

#ifdef __cplusplus
extern "C" {
#endif

namespace Core
{

class ThreadSync
{
public:

	ThreadSync();
	virtual ~ThreadSync();

	void		Enter();
	void		Leave();

protected:
	
	CriticalSection		mCS;
};

}

#ifdef __cplusplus
}
#endif
