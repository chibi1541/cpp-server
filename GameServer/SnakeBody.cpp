#include "pch.h"
#include "SnakeBody.h"
#include "SnakeHead.h"
#include "ObjectIdHandler.h"

SnakeBody::SnakeBody(uint64 objectId, int32 x, int32 y)
	: Actor(objectId, x, y)
{

}

SnakeBody::SnakeBody(uint64 objectId, Vector2 pos)
	: Actor(objectId, pos)
{
}

SnakeBody::~SnakeBody()
{

}

void SnakeBody::OnCollision(const ActorRef& other)
{
	ObjectType objType = ObjectIdHandler::GetObjectType(other->GetObjectId());

	if (objType == ObjectType::OBJECT_SNAKE_HEAD)
	{
		// TODO : 해당 유저 GameOver 처리
	}
}
