#pragma once


namespace Core 
{
	class ThreadMgr;

	enum ThreadState {
		THREAD_NONE,
		THREAD_SUSPEND,
		THREAD_RUNNING,
		THREAD_END,
	};

	class Thread  
	{
	friend ThreadMgr;

	public:
		Thread();
		virtual ~Thread();

		virtual bool	Begin(bool bSuspend=false);
		virtual bool	End();

		bool			Suspend();
		bool			Resume();
		bool			Termainate();

		TCHAR*			GetErrorString(DWORD errorCode);
		ThreadState		GetState() { return mState; }

		void			SetTerminateWaitTime(DWORD milliSec)		{ mTermWaitTime = milliSec; };
		bool			IsState(ThreadState state) { return (mState == state); }

	private:
		static	DWORD CALLBACK	ThreadRunner(LPVOID param);

	protected:
		virtual DWORD			ThreadTick()		= 0;	//if returns 0, Thread Loop breaks and thread func returns. not 0, Keep gonig loop.

		virtual void			OnEnd(bool bTerminated);

	protected:

		HANDLE						mhThread;
		DWORD						mThreadId;
		volatile	ThreadState		mState;
		DWORD						mTermWaitTime;
	};
}
