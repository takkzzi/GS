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
	for(auto & i : mSessionVec) {
		if ( i ) {
			delete i;
		}
	}
}

UserSession* UserSessionManager::CreateUserSession(UINT sessIdx)
{
	ASSERT(sessIdx > 0);

	UserSession* user = NULL;

	if ( sessIdx < mSessionVec.size() ) {
		user = mSessionVec[sessIdx];
	}
	else {
		while(sessIdx >= mSessionVec.size()) {
			mSessionVec.push_back(NULL);
		}
	}

	if ( ! user ) {
		mSessionVec[sessIdx] = user =  new UserSession();
	}

	return user;
}

UserSession* UserSessionManager::GetUserSession(Network::Session* netSession, bool bCreate)
{
	ASSERT(netSession);

	UINT sessionIndex = netSession->GetId();
	UserSession* user = GetUserSession(sessionIndex);

	if ( ! user && bCreate ) {
		user = CreateUserSession(sessionIndex);
	}

	if ( user->IsDestroyed() )
		user->Init(netSession);

	return user;
}

UserSession* UserSessionManager::GetUserSession(UINT sessionId)
{
	if (sessionId < mSessionVec.size())
		return mSessionVec[sessionId];

	return NULL;
}
