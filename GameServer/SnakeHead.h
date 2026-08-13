#pragma once
#include "Actor.h"
#include "Enum.pb.h"

class SnakeBody;

class SnakeHead : public Actor
{
public:
	SnakeHead(PlayerRef owner);
	SnakeHead(uint64 objectId, int32 x, int32 y, PlayerRef owner);
	SnakeHead(uint64 objectId, Vector2 pos, PlayerRef owner);
	~SnakeHead();

	const Protocol::DirectionType& GetDirection() const { return _direction; }
	void SetDirection(Protocol::DirectionType newDirection);

	float GetMoveSpeed() const { return _moveSpeed; }

	void AddBody(const Vector2 position);

	void SwapBody(const Vector2 position);

	void MakeHeadData(Protocol::HeadData** OUT data);

private:
	virtual void Tick(float deltaTime) override;

	virtual void OnCollision(const ActorRef& other) override;

	virtual void MarkDestory() override;

private:
	PlayerRef					owner;		// 참조 사이클 주의!
	deque<SnakeBodyRef>			_bodys;
	Protocol::DirectionType		_direction = Protocol::DirectionType::DIR_RIGHT;
	uint32						_tailIndex = 0;
	float						_moveSpeed = 15.f;
	// 몸체를 늘리는 요청 카운터
	uint32						_addBodyCallCount = 0;
};

