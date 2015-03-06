#pragma once

#pragma comment(lib, "Ws2_32.lib")

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

#include <vector>
#include <list>
#include <map>
#include <queue>

#include "Core/Core.h"
#include "Defines.h"
#include "Networker.h"
#include "Session.h"

using namespace Network;