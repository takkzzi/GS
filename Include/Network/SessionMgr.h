#pragma once

#ifdef __cpluscplus
extern "C" {
#endif // __cpluscplus

namespace Network
{
	class Session;

	class SessionMgr
	{
	public:
		SessionMgr(void);
		virtual ~SessionMgr(void);
	};
}

#ifdef __cpluscplus
}
#endif // __cpluscplus