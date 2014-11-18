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
		virtual void	End(bool bForceTerminate=false);

		bool			Suspend();
		bool			Resume();
		
		TCHAR*			GetErrorString(DWORD errorCode);
		ThreadState		GetState() { return mState; }

		void			SetTerminateWaitTime(DWORD milliSec)		{ mTermWaitTime = milliSec; };
		bool			IsState(ThreadState state) { return (mState == state); }

	protected:
		
		static DWORD CALLBACK	ThreadRunner(LPVOID param);
		virtual DWORD			Run();//			=	0;

		virtual void			OnEnd(bool bTerminated=false);

	protected:

		HANDLE					mhThread;
		//HANDLE					mhEndEvent;
		ThreadState				mState;
		DWORD					mTermWaitTime;
	};
}
