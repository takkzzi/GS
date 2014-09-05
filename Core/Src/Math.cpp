#include "pch.h"
#include "Math.h"
#include <time.h>


using namespace Core;

bool	Math::msbInit = false;
volatile DWORD	Math::msRandSeed = 0;

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
	//return rand();
	return( ((msRandSeed = msRandSeed * 214013L + 2531011L) >> 16) & 0x7FFF );
}

int Math::RandRange(int begin, int end)
{
	int diff = end - begin;
	return (Rand() % diff) + begin;
}

float Math::RandFloat()
{
	return (float)Rand() / (float)RAND_MAX;
}
