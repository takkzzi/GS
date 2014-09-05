#pragma once

#ifdef __cpluscplus
extern "C" {
#endif // __cpluscplus

namespace Core {

	class Thread;

	class ThreadMgr {

	public :
		ThreadMgr();
		virtual ~ThreadMgr();

		DWORD CreateThread(LPTHREAD_START_ROUTINE threadFunc, void* param, OUT DWORD& id);

	protected:

		std::list<Thread*>		mThreadList;
	};
}

#ifdef __cpluscplus
}
#endif // __cpluscplus