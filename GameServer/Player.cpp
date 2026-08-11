#include "pch.h"
#include "Player.h"

Player::~Player()
{
	if (headActor)
		headActor.reset();
}
