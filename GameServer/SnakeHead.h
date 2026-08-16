#pragma once
#include "Actor.h"
#include "Enum.pb.h"

class SnakeBody;

class SnakeHead : public Actor
{
public:
	enum AsixType 
	{
		X = 0,
		Y = 1,
		NUMBER = 3,
	};

	SnakeHead(PlayerRef owner);
	SnakeHead(uint64 objectId, int32 x, int32 y, PlayerRef owner);
	SnakeHead(uint64 objectId, Vector2 pos, PlayerRef owner);
	~SnakeHead();

	const Protocol::DirectionType& GetDirection() const { return _direction; }

	void SetDirection(Protocol::DirectionType newDirection);

	float GetMoveSpeed() const { return _moveSpeed; }

	void MakeHeadData(Protocol::HeadData** OUT data);

	void PushInput(const DirectionType& input);

	void AddTrail(const Vector2& pos);

	DirectionType FindTrailDir(const Vector2 pos);

	const deque<Protocol::TrailData>& GetTrailQueue() const {return _trailQueue; }

	const vector<Vector2> GetSnakeArray() const;

	bool SelfCheck() const;

	virtual void OnCollision(const Protocol::ObjectType& objectType) override;

private:
	virtual void Tick(float deltaTime) override;

	virtual void MarkDestory() override;

	virtual const vector<Vector2> GetCollisionCheckArea() const override;

	void Move(float detaTime);

	void ProcessInputQueue();

	// 앞 궤적의 좌표와 뒷 궤적의 좌표가 같은 액터가 있다면 true를 반환
	bool WarningTrailPos();

	const PlayerRef& GetOwner() const {return _owner;}

private:
	queue<DirectionType>		_inputQueue;	// 클라로부터 받은 입력을 바로 처리하지 않고 좌표가 1이라도 변경된 후에 하나씩 처리

	PlayerRef					_owner;		// 참조 사이클 주의!
	deque<SnakeBodyRef>			_bodys;
	DirectionType				_direction = DirectionType::DIR_RIGHT;
	uint32						_tailIndex = 0;
	float						_moveSpeed = 15.f;

	// 방향 전환 후에 1그리드 이상 움직여서 방향 전환이 가능한 상태를 체크
	bool						_canTurn = false;

	// 몸체를 늘리는 요청 카운터, 하나는 가지고 시작함(꼬리가 없으면 볼품없음)
	uint32						_addBodyCallCount = 1;

	// 몸체를 궤적 방식으로 변경
	
	// 몸체 사이즈, 머리가 기역해야 할 궤적 좌표 값 갯수
	// 이것보다 궤적 queue에 쌓인게 많으면 앞에서부터 하나씩 날림
	uint32							_trailCount = 0;
	deque<Protocol::TrailData>		_trailQueue;
};

