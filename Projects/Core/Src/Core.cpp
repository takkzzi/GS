#include "PCH.h"
#include "Core.h"

using namespace Core;

bool	CoreSystem::msInit = false;

void CoreSystem::Init(const TCHAR* logDirName)
{
	Logger::Init(logDirName);
	System::Init();
	Time::Init();
	Math::Init();
	Dump::Init();

	msInit = true;
}

void CoreSystem::Shutdown() 
{
	Dump::Shutdown();
	Logger::Shutdown();
	Dump::Shutdown();

	msInit = false;
}
