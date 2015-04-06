#pragma once

#include "UserSession.h"


namespace Game
{

	class UserSessionManager
	{
	public :
		UserSessionManager(UINT reserveUserCount, UINT maxUserCount);
		~UserSessionManager();

	public :
		UserSession*		GetUserSession(Network::Session* netSession, bool bCreate);
		UserSession*		GetUserSession(UINT sessoinId);

	protected:
		UserSession*		CreateUserSession(UINT sessionIndex);


	protected :
		std::vector<UserSession*>			mSessionVec;
	};
}
