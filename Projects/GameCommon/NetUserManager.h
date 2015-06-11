#pragma once

#include "NetUser.h"


namespace Game
{
	class NetUser;

	class NetUserManager
	{
	public :
		NetUserManager(UINT reserveUserCount, UINT maxUserCount);
		~NetUserManager();

	public :
		NetUser*		GetNetUser(Network::Session* netSession, bool bCreate);
		NetUser*		GetNetUser(UINT sessoinId);

	protected:
		NetUser*		CreateNetUser(UINT userIndex);


	protected :
		std::vector<NetUser*>			mUserVec;
	};
}
