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

	void PushInput(const DirectionType& input);

	void AddTrail(const Vector2& position);

private:
	virtual void Tick(float deltaTime) override;

	virtual void OnCollision(const ActorRef& other) override;

	virtual void MarkDestory() override;

	// 앞 궤적의 좌표와 뒷 궤적의 좌표가 같은 액터가 있다면 true를 반환
	bool WarningTrailPos();

private:
	queue<DirectionType>		_inputQueue;	// 클라로부터 받은 입력을 바로 처리하지 않고 좌표가 1이라도 변경된 후에 하나씩 처리

	PlayerRef					_owner;		// 참조 사이클 주의!
	deque<SnakeBodyRef>			_bodys;
	Protocol::DirectionType		_direction = Protocol::DirectionType::DIR_RIGHT;
	uint32						_tailIndex = 0;
	float						_moveSpeed = 15.f;
	
	// 몸체를 늘리는 요청 카운터
	uint32						_addBodyCallCount = 0;

	// 몸체를 궤적 방식으로 변경
	
	// 몸체 사이즈, 머리가 기역해야 할 궤적 좌표 값 갯수
	// 이것보다 궤적 queue에 쌓인게 많으면 앞에서부터 하나씩 날림
	uint32						_trailCount = 0;
	deque<Vector2>				_trailQueue;
};

