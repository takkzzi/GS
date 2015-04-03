#pragma once

#include "UserSession.h"


namespace Game
{
	class Session;

	class UserSessionManager
	{
	public :
		UserSessionManager(UINT reserveUserCount, UINT maxUserCount);
		~UserSessionManager();

	public :
		void				CreateUserSession(UINT sessionIndex);
		UserSession*		GetUserSession(Session* netSession, bool bCreate);


	protected :
		std::vector<UserSession*>			mSessionVec;
	};
}
