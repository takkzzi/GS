// TestConsole.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include <windows.h>

#include "Core/Core.h"
#include "Network/Network.h"



int _tmain(int argc, _TCHAR* argv[])
{
	CoreSystem::Init(_T("TestLog"));
	NetworkSystem::Init();

	Networker* net = NULL;
	net->BeginListen(40001, true);

	CoreSystem::Shutdown();
	NetworkSystem::Shutdown();

}

