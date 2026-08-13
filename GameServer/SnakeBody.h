#pragma once
#include "Actor.h"
#include "Struct.pb.h"

class SnakeBody : public Actor
{
public:
	SnakeBody(uint64 objectId, int32 x, int32 y);
	SnakeBody(uint64 objectId, Vector2 pos);
	~SnakeBody();

private:
	//virtual void Tick(float deltaTime) override;

	virtual void OnCollision(const ActorRef& other) override;

private:

};

