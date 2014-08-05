#include "pch.h"
#include "Math.h"
#include <time.h>


using namespace Core;

bool	Math::msbInit = false;
DWORD	Math::msRandSeed = 0;

void Math::Init() 
{
	if ( msbInit ) return;

	msbInit = true;
	SetRandSeed(GetTickCount());
}


void Math::SetRandSeed(DWORD seed)
{
	msRandSeed = seed;
	srand(seed);
}

int Math::Rand()
{
	return rand();
}

int Math::RandRange(int begin, int end)
{
	int diff = end - begin;
	//Debug::Assert(diff > 0);
	return (rand() % diff) + begin;
}

float Math::RandFloat()
{
	return (float)rand() / (float)RAND_MAX;
}
