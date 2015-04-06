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
		void				CreateUserSession(UINT sessionIndex);
		UserSession*		GetUserSession(Network::Session* netSession, bool bCreate);
		UserSession*		GetUserSession(UINT sessoinId);


	protected :
		std::vector<UserSession*>			mSessionVec;
	};
}
