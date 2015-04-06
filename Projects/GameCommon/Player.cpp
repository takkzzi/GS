#include "pch.h"
#include "Player.h"

using namespace Game;

Player::Player() 
{
	mActorType = ACTOR_PLAYER;
}

Player::~Player()
{
}

void Player::OnCreate()
{
	__super::OnCreate();
	
}

void Player::OnDestroy()
{
	__super::OnDestroy();
	Logger::Log("Player", "Player Disconnected (Destroyed) [id:%d]", mActorId);
}

void Player::Update(float dt)
{
}
