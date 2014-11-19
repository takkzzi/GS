#pragma once


namespace Network
{
	class NetworkSystem
	{
	public:

		static void			Init();
		static void			Shutdown();
		static bool			IsInit()			{ return msbInit; }

	private:
		static	bool		msbInit;
	};
}

#ifndef WIN32_LEAN_AND_MEAN             
	#define	WIN32_LEAN_AND_MEAN		// Exclude rarely-used stuff from Windows headers
#endif
#include <Windows.h>
#include <WinSock2.h>
#include <mswsock.h>

using namespace Network;

#include "Defines.h"
#include "IOCP.h"
#include "Session.h"