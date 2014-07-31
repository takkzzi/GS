#include "PCH.h"
#include "Core.h"
#include "Macro.h"
#include "System.h"
#include "Math.h"
#include "ThreadMgr.h"

using namespace Core;

void CoreSystem::Init()
{
	System::Init();
	Time::Init();
	Math::Init();
}

