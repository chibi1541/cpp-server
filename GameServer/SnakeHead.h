#pragma once
#include "Actor.h"
#include "Enum.pb.h"

class SnakeHead : public Actor
{
public:
	SnakeHead(PlayerRef owner);
	SnakeHead(uint64 objectId, int32 x, int32 y, PlayerRef owner);
	~SnakeHead();

	const Protocol::DirectionType& GetDirection() const { return _direction; }
	void SetDirection(Protocol::DirectionType newDirection);

	float GetMoveSpeed() const { return _moveSpeed; }

private:
	virtual void Tick(float deltaTime) override;

private:
	PlayerRef owner;		// 참조 사이클 주의!
	float _moveSpeed = 15.f;
	

	Protocol::DirectionType _direction = Protocol::DirectionType::DIR_RIGHT;
};

