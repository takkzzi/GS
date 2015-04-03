#include "pch.h"
#include "UserSessionManager.h"
#include "UserSession.h"


using namespace Game;

UserSessionManager::UserSessionManager(UINT reserveUserCount, UINT maxUserCount)
{
	mSessionVec.reserve(maxUserCount);

	for(UINT i = 0; i < reserveUserCount; ++i) {
		mSessionVec.push_back(new UserSession());
	}
}

UserSessionManager::~UserSessionManager()
{
}

void UserSessionManager::CreateUserSession(UINT sessionIndex)
{
}

UserSession* UserSessionManager::GetUserSession(Session* netSession, bool bCreate)
{
	UINT sessionIndex = 0;
	UserSession* user = NULL;

	if ( sessionIndex < mSessionVec.size() ) {
		if ( ! mSessionVec[sessionIndex]->IsDestroyed() )
			user = mSessionVec[sessionIndex];
		else if ( bCreate ) {
			mSessionVec[sessionIndex]->Init(netSession);
		}
	}

	return user;
}
