#pragma once


namespace Core 
{
	class ThreadMgr;

	enum eState {
		STATE_NONE,
		STATE_SUSPEND,
		STATE_RUNNING,
		STATE_ENDING,
		STATE_END
	};

	class Thread  
	{
	friend ThreadMgr;

	public:
		Thread();
		virtual ~Thread();

		void			Begin(bool bSuspend=false);
		void			End(bool bForceTerminate=false);
		bool			Resume();
		
		TCHAR*			GetErrorString(DWORD errorCode);
		eState			GetState() { return mState; }

		void			SetTerminateWaitTime(DWORD milliSec)		{ mTermWaitTime = milliSec; };

	protected:
		
		static DWORD CALLBACK	ThreadRunner(LPVOID param);
		virtual DWORD			Run();//			=	0;

		virtual void			OnEnd(bool bTerminated=false);

	protected:

		HANDLE					mhThread;
		HANDLE					mhEndEvent;
		eState					mState;
		DWORD					mTermWaitTime;
	};
}
