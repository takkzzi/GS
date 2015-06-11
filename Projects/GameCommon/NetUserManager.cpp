#include "pch.h"
#include "NetUserManager.h"


using namespace Game;

NetUserManager::NetUserManager(UINT reserveUserCount, UINT maxUserCount)
{
	mUserVec.reserve(maxUserCount);

	for(UINT i = 0; i < reserveUserCount; ++i) {
		mUserVec.push_back(new NetUser());
	}
}

NetUserManager::~NetUserManager()
{
	for(auto & i : mUserVec) {
		if ( i ) {
			delete i;
		}
	}
}

NetUser* NetUserManager::CreateNetUser(UINT sessIdx)
{
	ASSERT(sessIdx > 0);

	NetUser* user = NULL;

	if ( sessIdx < mUserVec.size() ) {
		user = mUserVec[sessIdx];
	}
	else {
		while(sessIdx >= mUserVec.size()) {
			mUserVec.push_back(NULL);
		}
	}

	if ( ! user ) {
		mUserVec[sessIdx] = user =  new NetUser();
	}

	return user;
}

NetUser* NetUserManager::GetNetUser(Network::Session* netSession, bool bCreate)
{
	ASSERT(netSession);

	UINT sessionIndex = netSession->GetId();
	NetUser* user = GetNetUser(sessionIndex);

	if ( (user == NULL) && bCreate ) {
		user = CreateNetUser(sessionIndex);
	}

	if ( user->IsDestroyed() )	//Reuse !
		user->Init(netSession);

	return user;
}

NetUser* NetUserManager::GetNetUser(UINT sessionId)
{
	if (sessionId < mUserVec.size())
		return mUserVec[sessionId];

	return NULL;
}
