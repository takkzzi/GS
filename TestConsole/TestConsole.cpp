// TestConsole.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include <windows.h>
#include <stdio.h>
#include "Core\Core.h"

class test {
public :
	int			i;
	int			j;
};

using namespace Core;

int _tmain(int argc, _TCHAR* argv[])
{
	CoreSystem::Init();

	test* t = new test;
	delete t;
	t = NULL;
	ASSERT(t != NULL);


	return 0;
}

