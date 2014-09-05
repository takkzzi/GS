
#pragma once

#ifdef __cpluscplus
extern "C" {
#endif // __cpluscplus

namespace Core 
{
	class Dump 
	{
	public:
		static	void		Init();
		static	void		Shutdown();
	};
}

#ifdef __cpluscplus
}
#endif // __cpluscplus