#pragma once

namespace Game
{
	class UserSession;

	class UserSessionManager
	{
	public :
		UserSessionManager();
		~UserSessionManager();

	public :
		void				CreateUserSession(UINT sessionIndex);
		UserSession*		GetUserSession(UINT sessionIndex);
	};
}
