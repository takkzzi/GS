#include "PCH.h"
#include "Network.h"

#include <winsock2.h>





using namespace Core;
using namespace Network;


bool NetworkSystem::msbInit = false;

//static
void NetworkSystem::Init()
{
	if ( msbInit ) return;

	WSADATA wsaData;
	WSAStartup(MAKEWORD(2,2), &wsaData);
	msbInit = true;
}

//static
void NetworkSystem::Shutdown()
{
	if ( ! msbInit ) return;

	WSACleanup();
	msbInit  = false;
}
