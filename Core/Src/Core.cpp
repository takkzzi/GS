#include "PCH.h"
#include "Core.h"


using namespace Core;

void CoreSystem::Init()
{
	System::Init();
	Time::Init();
	Math::Init();
	Dump::Init();
	Logger::Init();
}

void CoreSystem::Finalize() 
{
	Logger::Finalize();
	Dump::Finalize();
}
