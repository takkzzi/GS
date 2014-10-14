// TestConsole.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include "Core/Core.h"
#include "Network\Network.h"

int _tmain(int argc, _TCHAR* argv[])
{
	CoreSystem::Init();
	NetworkSystem::Init();

	Session *s = new Session(0, 10, 10);

	SAFE_DELETE(s);
	//ASSERT(s != NULL && "--- Assert Test");
	//s->Disconnect();

	Logger::LogError("AAA!!!");

	CoreSystem::Shutdown();
	NetworkSystem::Shutdown();
}

