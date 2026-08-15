#include "pch.h"
#include "Actor.h"
#include "ObjectIdHandler.h"

Actor::Actor(uint64 objectId, int32 x, int32 y)
	: objectId(objectId)
{
	SetPosition(x, y);
}

Actor::Actor(uint64 objectId, Vector2 pos)
	:objectId(objectId), position(pos)
{
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

void Actor::MarkDestory()
{
	isExpired = true;
}

void Actor::OnCollision(const Protocol::ObjectType& objectType)
{
}

ObjectType Actor::GetObjecType() const
{
	return ObjectIdHandler::GetObjectType(objectId);
}

const vector<Vector2> Actor::GetCollisionCheckArea()
{
	return vector<Vector2>();
}
