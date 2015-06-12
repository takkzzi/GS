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

	LOG_WARNING_A("asldkjflaskdjfl;ajsdfl");

	CoreSystem::Shutdown();
	NetworkSystem::Shutdown();

}

