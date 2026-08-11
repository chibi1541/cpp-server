#pragma once
#include "Actor.h"
#include "Enum.pb.h"

class SnakeHead : public Actor
{
public:
	SnakeHead(PlayerRef owner);
	SnakeHead(uint64 objectId, int x, int y, PlayerRef owner);
	virtual ~SnakeHead() override;

	const Protocol::DirectionType& GetDirection() const { return _direction; }
	void SetDirection(Protocol::DirectionType newDirection);

private:
	virtual void Tick(float deltaTime) override;

private:
	PlayerRef owner;		// 참조 사이클 주의!
	float _moveSpeed = 5.f;
	
	float _xPos;			// Tick 이동 시의 좌표 값
	float _yPos;

	Protocol::DirectionType _direction = Protocol::DirectionType::DIR_RIGHT;
};

