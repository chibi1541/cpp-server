#include "pch.h"
#include "Player.h"

Player::~Player()
{
	if (headActor)
		headActor.reset();
}

void Player::ReleaseControlActor()
{
	if (headActor)
		headActor.reset();

	headActor = nullptr;
}

void Player::AddScore()
{
	++score;
}
