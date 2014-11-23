#include "PCH.h"
#include "Core.h"

bool	CoreSystem::msInit = false;

void CoreSystem::Init()
{
	System::Init();
	Time::Init();
	Math::Init();
	Dump::Init();
	Logger::Init();

	msInit = true;
}

void CoreSystem::Shutdown() 
{
	Logger::Shutdown();
	Dump::Shutdown();

	msInit = false;
}
