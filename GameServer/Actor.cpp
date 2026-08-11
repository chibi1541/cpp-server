#include "pch.h"
#include "Actor.h"

Actor::Actor(uint64 objectId, int x, int y)
	: objectId(objectId)
{
	SetPosition(x, y);
}

void Actor::Tick(float deltaTime)
{
}

void Actor::SetPosition(int32 x, int32 y)
{
	position.set_x(x);
	position.set_y(y);
}

void Actor::SetObjectId(uint64 objectId)
{
	this->objectId = objectId;
}
