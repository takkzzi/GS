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

UserSession* UserSessionManager::GetUserSession(Network::Session* session, bool bCreate)
{
	UINT sessionIndex = session->GetId();
	UserSession* user = NULL;

	if ( sessionIndex < mSessionVec.size() ) {
		user = mSessionVec[sessionIndex];
		if ( user ) {
			if ( ! mSessionVec[sessionIndex]->IsDestroyed() )
				user = mSessionVec[sessionIndex];
			else if ( bCreate ) {
				mSessionVec[sessionIndex]->Init(session);
			}
		}
	}

	return user;
}

UserSession* UserSessionManager::GetUserSession(UINT sessionId)
{
	if (sessionId < mSessionVec.size())
		return mSessionVec[sessionId];

	return NULL;
}