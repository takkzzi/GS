// TestConsole.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include "Test0.h"

using namespace Core;
using namespace std;


int _tmain(int argc, _TCHAR* argv[])
{
	CoreSystem::Init();
	
	for (int i = 0; i < 100; ++i)
		Logger::Log(_T("TEST0"), _T("al;kjsdgla;jsdglk;ajsld;kfjal;ksjdfl;asjf : %d"), i);

	for (int i = 0; i < 100; ++i)
		Logger::LogWithDate(_T("TEST1"), _T("asldfkjal;sdfjla;sjdkfl;kajsdfl;kajlsd;fjla;s : %d"), i);

	CoreSystem::Finalize();

	return 0;
}

