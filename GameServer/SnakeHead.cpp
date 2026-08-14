#include "pch.h"
#include "SnakeHead.h"
#include "Utils.h"
#include "Room.h"
#include "SnakeBody.h"
#include "ObjectIdHandler.h"
#include "ClientPacketHandler.h"

SnakeHead::SnakeHead(PlayerRef owner)
	: _owner(owner)
{
}

SnakeHead::SnakeHead(uint64 objectId, int32 x, int32 y, PlayerRef owner)
	:Actor(objectId, x, y), _owner(owner)
{
}

SnakeHead::SnakeHead(uint64 objectId, Vector2 pos, PlayerRef owner)
	:Actor(objectId, pos), _owner(owner)
{
}

SnakeHead::~SnakeHead()
{

}

void SnakeHead::SetDirection(Protocol::DirectionType newDirection)
{
	// 입력 방향이 현재 방향의 반대라면 캔슬
	// 0, 1 이 나오는데 0 이면 이전 이동방향이 +(Right, Bottom), 1이면 이전 이동 방향이 -(Left, Top)
	int32 dirType = static_cast<int32>(_direction);
	int dirValue = ((dirType % 2) == 0) ? -1 : 1;
	int32 newDirType = static_cast<int32>(newDirection);
	if (dirType + dirValue == newDirType)
	{
		// 반대 방향이므로 못감
		return;
	}

	Protocol::DirectionType prevDir = _direction;
	_direction = newDirection;

	cout << "New Dir : " << _direction << " Pos x : " << position.x() << " Pos y : " << position.y() << "\n";
}

void SnakeHead::AddBody(const Vector2 position)
{
	SnakeBodyRef newTail = _bodys.emplace_front(
		make_shared<SnakeBody>(ObjectIdHandler::GenerateObjectId(Protocol::ObjectType::OBJECT_SNAKE_BODY), position));

	GRoom->AddActor(newTail);

	// TODO : 꼬리 생성용 패킷으로 교체
	// 꼬리 스폰 패킷 브로드캐스팅
	Protocol::S_SPAWN_ACTOR pkt;
	pkt.set_id(newTail->GetObjectId());
	pkt.set_allocated_spawnpos(new Vector2(position));

	SendBufferRef sendBuffer = ClientPacketHandler::MakeSendBuffer(pkt);
	GRoom->DoAsync(&Room::Broadcast, sendBuffer);
}

void SnakeHead::SwapBody(const Vector2 position)
{
	// 꼬리 부분을 머리 앞 부분으로 이동
	if (_bodys.size() == 1)
	{
		// 몸체가 꼬리 밖에 없으므로 좌표만 이동
		_bodys[0]->SetPosition(position);
	}

	else if(_bodys.size() > 1)
	{
		int tailIdx = _bodys.size() - 1;
		SnakeBodyRef tailRef = _bodys.back();
		_bodys.pop_back();
		tailRef->SetPosition(position);
		_bodys.emplace_front(tailRef);
	}
}

void SnakeHead::MakeHeadData(Protocol::HeadData** OUT data)
{
	ActorInfo* actor = (*data)->mutable_actor();
	actor->set_objectid(objectId);
	actor->set_allocated_pos(new Vector2(position));

	(*data)->set_movespeed(_moveSpeed);
	(*data)->set_dir(_direction);

	for (const Vector2& trail : _trailQueue)
	{
		Vector2* newTrail = (*data)->add_trails();
		newTrail->set_x(trail.x());
		newTrail->set_y(trail.y());
	}
}

void SnakeHead::PushInput(const DirectionType& input)
{
	_inputQueue.emplace(input);
}

void SnakeHead::AddTrail(const Vector2& position)
{
	_trailQueue.emplace_back(position);

	if (_addBodyCallCount > 0)
	{
		++_trailCount;
		--_addBodyCallCount;
	}

	while (_trailQueue.size() > _trailCount)
	{
		_trailQueue.pop_front();
	}
}

void SnakeHead::Tick(float deltaTime)
{
	// Actor::Tick(deltaTime);

	ASSERT_CRASH(_direction != Protocol::DirectionType::DIR_NONE);
	int32 direction = static_cast<int32>(_direction);

	// y축 = 1, x축 = 0
	// TODO : 이동 로직 정리
	int32 axisType = direction / 3;
	int32 valueType = (direction % 2 == 0) ? 1 : -1;

	int32 xPos = position.x();
	int32 yPos = position.y();

	Vector2 prevPos;
	prevPos.set_x(xPos / 100);
	prevPos.set_y(yPos / 100);

	if (axisType == 0)
	{
		xPos += (valueType * _moveSpeed * deltaTime) * 100;
	}
	else
	{
		// TODO : 속도 및 좌표 보정치(0.66f, 100 ...) 매직 넘버화
		yPos += (valueType * (_moveSpeed * /* y축 이동이 체감상 너무 빨리서 속도 보정*/0.66f) * deltaTime) * 100;
	}

	position.set_x(xPos);
	position.set_y(yPos);

	Vector2 nextPos;
	nextPos.set_x(xPos / 100);
	nextPos.set_y(yPos / 100);

	if (prevPos != nextPos)
	{
		int32 xDelta = nextPos.x() - prevPos.x();
		int32 yDelta = nextPos.y() - prevPos.y();

		// 좌표 값 사이의 부호 방향이 나옴
		int32 xValue = (xDelta != 0) ? (xDelta / ::abs(xDelta)) : 0;
		int32 yValue = (yDelta != 0) ? (yDelta / ::abs(yDelta)) : 0;

		// 2칸 이상 움직인 겨우 1 이상의 값이 나옴
		int32 xCount = ::abs(xDelta) - 1;
		int32 yCount = ::abs(yDelta) - 1;

		AddTrail(prevPos);

		while (xCount > 0)
		{
			prevPos.set_x(prevPos.x() + xValue);
			Vector2 trail = prevPos;
			
			AddTrail(trail);
			--xCount;
		}

		while (yCount > 0)
		{
			prevPos.set_y(prevPos.y() + yValue);
			Vector2 trail = prevPos;

			AddTrail(trail);
			--yCount;
		}

		// input queue 소비
		if(_inputQueue.empty() == false)
		{ 
			DirectionType newDir = _inputQueue.front();
			_inputQueue.pop();

			SetDirection(newDir);
		}

		prevPos.set_x(prevPos.x() * 100);
		prevPos.set_y(prevPos.y() * 100);

		// 격자에서 좌표 이동
		// 바디 이동
		//if (_addBodyCallCount > 0)
		//{
		//	AddBody(prevPos);
		//	--_addBodyCallCount;
		//}
		//else
		//{
		//	SwapBody(prevPos);
		//}

		if(WarningTrailPos())
			cout << "몸통 좌표 겹침 \n";
	}
}

void SnakeHead::OnCollision(const ActorRef& other)
{
	Protocol::ObjectType otherObjType = ObjectIdHandler::GetObjectType(other->GetObjectId());
	if (otherObjType == Protocol::ObjectType::OBJECT_ITEM)
	{
		// 현재 헤드 위치에 새 바디 추가
		++_addBodyCallCount;
	}
}

void SnakeHead::MarkDestory()
{
	Actor::MarkDestory();

	for (SnakeBodyRef body : _bodys)
	{
		if (body->IsActive())
		{
			body->MarkDestory();
		}
	}
}

bool SnakeHead::WarningTrailPos()
{
	Protocol::Vector2 prev; 
	prev.set_x(position.x() / 100);
	prev.set_y(position.y() / 100);

	int size = _trailQueue.size();

	for (int i = 0;i < size - 1; ++i)
	{
		for (int j = i + 1; j < size; ++j)
		{
			if (_trailQueue[i] == _trailQueue[j])
				return true;
		}
	}

	return false;
}
