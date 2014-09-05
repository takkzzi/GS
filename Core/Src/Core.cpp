#include "PCH.h"
#include "Core.h"


void CoreSystem::Init()
{
	System::Init();
	Time::Init();
	Math::Init();
	Dump::Init();
	Logger::Init();
}

void CoreSystem::Shutdown() 
{
	Logger::Shutdown();
	Dump::Shutdown();
}
