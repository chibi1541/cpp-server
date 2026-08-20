#include "pch.h"
#include "SnakeHead.h"
#include "Room.h"
#include "SnakeBody.h"
#include "ObjectIdHandler.h"
#include "ClientPacketHandler.h"
#include "Player.h"

using namespace Protocol;

SnakeHead::SnakeHead(PlayerRef owner)
	: _owner(owner)
{
}

SnakeHead::SnakeHead(uint64 objectId, int32 x, int32 y, PlayerRef owner)
	:Actor(objectId, x, y), _owner(owner)
{
	prevPos.set_x(x);
	prevPos.set_y(y);
}

SnakeHead::SnakeHead(uint64 objectId, Vector2 pos, PlayerRef owner)
	:Actor(objectId, pos), _owner(owner)
{
	prevPos = pos;
}

SnakeHead::~SnakeHead()
{

}

void SnakeHead::SetDirection(Protocol::DirectionType newDirection)
{
	int32 prevDirValue = static_cast<int32>(_direction);
	int32 newDirValue = static_cast<int32>(newDirection);
	int32 moveValue = ((newDirValue % 2) == 0) ? 1 : -1;

	// 이동 하려는 방향이 현재와 반대 방향이라면 못감
	if (newDirValue == prevDirValue + moveValue)
	{
		return;
	}

	_direction = newDirection;
	_canTurn = false;

	return;
}

void SnakeHead::MakeHeadData(Protocol::HeadData** OUT data)
{
	ActorInfo* actor = (*data)->mutable_actor();
	actor->set_objectid(objectId);
	Vector2* pos = actor->mutable_pos();
	pos->set_x(position.x());
	pos->set_y(position.y());

	(*data)->set_movespeed(_moveSpeed);
	(*data)->set_dir(_direction);

	(*data)->mutable_trails()->Reserve(static_cast<int32>(_trailQueue.size()));
	for (const TrailData& trail : _trailQueue)
	{
		TrailData* newTrail = (*data)->add_trails();
		Vector2* newPos = newTrail->mutable_pos();
		newPos->set_x(trail.pos().x());
		newPos->set_y(trail.pos().y());
		newTrail->set_curdir(trail.curdir());
		newTrail->set_prevdir(trail.prevdir());
	}
}

void SnakeHead::PushInput(const DirectionType& input)
{
	_inputQueue.emplace(input);
}

void SnakeHead::AddTrail(const Vector2& pos)
{
	DirectionType prevDir = DirectionType::DIR_NONE;
	if(_trailQueue.size() > 0)
	{
		TrailData prevTrail =  _trailQueue.back();
		prevDir = prevTrail.curdir();
	}

	Protocol::TrailData newTrail;
	Vector2* newPos = newTrail.mutable_pos();
	newPos->set_x(pos.x());
	newPos->set_y(pos.y());
	newTrail.set_prevdir(prevDir);
	newTrail.set_curdir(_direction);

	_trailQueue.emplace_back(newTrail);

	if (_addBodyCallCount > 0)
	{
		++_trailCount;
		--_addBodyCallCount;
	}

	while (_trailQueue.size() > _trailCount)
	{
		Protocol::TrailData trail = _trailQueue.front();
		_trailQueue.pop_front();
	}
}

DirectionType SnakeHead::FindTrailDir(const Vector2 pos)
{
	for(const Protocol::TrailData& trail : _trailQueue)
	{
		if(trail.pos() == pos)
			return trail.curdir();
	}

	return DirectionType::DIR_NONE;
}

const vector<Protocol::Vector2> SnakeHead::GetSnakeArray() const
{
	vector<Protocol::Vector2> ret;
	Protocol::Vector2 pos;
	pos.set_x(position.x() / 100);
	pos.set_y(position.y() / 100);

	ret.emplace_back(pos);

	for (int32 index = static_cast<int32>(_trailQueue.size()) - 1; index > 0; --index)
	{
		ret.emplace_back(_trailQueue[index].pos());
	}
	
	return ret; 
}

bool SnakeHead::SelfCheck() const
{
	const vector<Protocol::Vector2> checkArea = GetCollisionCheckArea();
	const vector<Protocol::Vector2> fullArea = GetSnakeArray();

	if(checkArea.size() >= fullArea.size())
		return false;

	for(const Vector2& check : checkArea)
	{
		// checkArea 수만큼  넘어감(당연히 같을 것이기 때문에)
		for(int32 index = static_cast<int32>(checkArea.size()); index < static_cast<int32>(fullArea.size()); ++index)
		{
			// 자기 몸통이랑 충돌이 발생
			if(check == fullArea[index])
			{
				return true;
			}
		}
	}

	return false;
}

void SnakeHead::Tick(float deltaTime)
{
	Move(deltaTime);

	if (WarningTrailPos())
		cout << "몸통 좌표 겹침 \n";
}

void SnakeHead::OnCollision(const Protocol::ObjectType& objectType)
{
	switch(objectType)
	{
		case Protocol::ObjectType::OBJECT_SNAKE_HEAD:
		case Protocol::ObjectType::OBJECT_SNAKE_BODY:
		case Protocol::ObjectType::OBJECT_WALL:
			MarkDestory();
			break;

		case Protocol::ObjectType::OBJECT_ITEM:
			
			if(_owner)
				_owner->AddScore();

			++_addBodyCallCount;
			break;
	}

}

const vector<Vector2> SnakeHead::GetCollisionCheckArea() const
{
	vector<Vector2> ret;
	Vector2 current;
	current.set_x(position.x() / 100);
	current.set_y(position.y() / 100);

	ret.emplace_back(current);

	Vector2 prev;
	prev.set_x(prevPos.x() / 100);
	prev.set_y(prevPos.y() / 100);
	if(current != prev)
	{
		for(int32 index = static_cast<int32>(_trailQueue.size())- 1 ; index > 0 ; --index)
		{
			// 큐의 마지막(머리 바로 뒤의 몸통)부터 순서대로 순회하면서 궤적을 체크 영역으로 반환
			// 이전 프레임의 영역은 전 Tick에서 체크 했을 것이므로 반환하지 않음
			if (_trailQueue[index].pos() == prev)
				break;

			ret.emplace_back(_trailQueue[index].pos());
		}
	}

	return ret;
}

void SnakeHead::Move(float detaTime)
{
	ASSERT_CRASH(_direction != Protocol::DirectionType::DIR_NONE);

	float remainTime = detaTime;

	// 이전 좌표 캐싱
	prevPos = position;

	do
	{
		ProcessInputQueue();

		SnakeHead::AsixType axisType = static_cast<SnakeHead::AsixType>(static_cast<int32>(_direction) / static_cast<int32>(AsixType::NUMBER));
		int32 moveValue = (static_cast<int32>(_direction) % 2 == 0) ? 1 : -1;

		Vector2 prev;
		prev.set_x(position.x()/100);
		prev.set_y(position.y()/100);
		
		// 좌우로 이동 중
		if (axisType == SnakeHead::AsixType::X)
		{
			int32 xNextPos = position.x();

			// 진행 방향으로 다음 그리드 정위치 값을 찾음
			// -축으로 이동하는 경우 1000 -> 999 로 1만 이동해도 값 좌표가 바뀌니 99를 기준으로 다음 위치를 찾음
			int32 nextX = (moveValue > 0) ? ((position.x() / 100) + moveValue) * 100 : ((position.x() / 100) * 100) + moveValue;

			// 다음 그리드까지 남은 시간을 구함
			float needTime = (nextX - xNextPos) / (moveValue * _moveSpeed * 100);

			// 남은 시간이 더 적은 경우
			if(remainTime < needTime)
			{
				// 될 수 있는 만큼 현재 방향으로 이동
				xNextPos += static_cast<int32>((moveValue * _moveSpeed * remainTime) * 100);
				position.set_x(xNextPos);
				return;
			}

			// 다음 그리드까지 진행
			position.set_x(nextX);

			// 궤적을 추가
			AddTrail(prev);

			// 1칸 이상 움직였으므로 방향 전환이 가능
			_canTurn = true;

			remainTime -= needTime;
		}
		// 상하로 이동 중
		else
		{
			int32 yNextPos = position.y();

			// 진행 방향으로 다음 그리드 정위치 값을 찾음
			int32 nextY = (moveValue > 0) ? ((position.y() / 100) + moveValue) * 100 : ((position.y() / 100) * 100) + moveValue;

			// TODO : 속도 및 좌표 보정치(0.66f, 100 ...) 매직 넘버화
			float needTime = (nextY - yNextPos) / (moveValue * (_moveSpeed * /* y축 이동이 체감상 너무 빨리서 속도 보정*/0.66f) * 100);

			// 남은 시간이 더 적은 경우
			if (remainTime < needTime)
			{
				// 될 수 있는 만큼 현재 방향으로 이동
				yNextPos += static_cast<int32>((moveValue * _moveSpeed * 0.66f * remainTime) * 100);
				position.set_y(yNextPos);
				return;
			}

			// 다음 그리드까지 진행
			position.set_y(nextY);

			// 궤적을 추가
			AddTrail(prev);

			// 1칸 이상 움직였으므로 방향 전환이 가능
			_canTurn = true;

			remainTime -= needTime;
		}
	}
	while(remainTime > 0.f);

}

void SnakeHead::ProcessInputQueue()
{
	// 이전 방향 전환 이후 한 칸도 움직이지 않음
	if(false == _canTurn)
		return;

	if (_inputQueue.empty() == false)
	{
		DirectionType newDir = _inputQueue.front();
		_inputQueue.pop();

		SetDirection(newDir);
	}
}

bool SnakeHead::WarningTrailPos()
{
	int32 size = static_cast<int32>(_trailQueue.size());

	for (int i = 0;i < size - 1; ++i)
	{
		for (int j = i + 1; j < size; ++j)
		{
			if (_trailQueue[i].pos() == _trailQueue[j].pos())
				return true;
		}
	}

	return false;
}
