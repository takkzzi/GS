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

		static	void	SetTerminateWaitTime(DWORD milliSec)		{ msTermWaitTime = milliSec; };

		void			Begin(bool bSuspend=false);
		void			End(bool bForceTerminate=false);
		bool			Resume();
		
		TCHAR*			GetErrorString(DWORD errorCode);
		eState			GetState() { return mState; }

	protected:
		
		static DWORD CALLBACK	ThreadRunner(LPVOID param);
		virtual DWORD			Run();//			=	0;

		virtual void			OnEnd(bool bTerminated=false);

	protected:

		static	DWORD			msTermWaitTime;

		HANDLE					mhThread;
		HANDLE					mhEndEvent;
		eState					mState;
	};
}
