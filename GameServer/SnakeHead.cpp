#include "pch.h"
#include "SnakeHead.h"
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

//void SnakeHead::AddBody(const Vector2 position)
//{
//	SnakeBodyRef newTail = _bodys.emplace_front(
//		make_shared<SnakeBody>(ObjectIdHandler::GenerateObjectId(Protocol::ObjectType::OBJECT_SNAKE_BODY), position));
//
//	GRoom->AddActor(newTail);
//
//	// TODO : 꼬리 생성용 패킷으로 교체
//	// 꼬리 스폰 패킷 브로드캐스팅
//	Protocol::S_SPAWN_ACTOR pkt;
//	pkt.set_id(newTail->GetObjectId());
//	pkt.set_allocated_spawnpos(new Vector2(position));
//
//	SendBufferRef sendBuffer = ClientPacketHandler::MakeSendBuffer(pkt);
//	GRoom->DoAsync(&Room::Broadcast, sendBuffer);
//}
//
//void SnakeHead::SwapBody(const Vector2 position)
//{
//	// 꼬리 부분을 머리 앞 부분으로 이동
//	if (_bodys.size() == 1)
//	{
//		// 몸체가 꼬리 밖에 없으므로 좌표만 이동
//		_bodys[0]->SetPosition(position);
//	}
//
//	else if(_bodys.size() > 1)
//	{
//		int tailIdx = _bodys.size() - 1;
//		SnakeBodyRef tailRef = _bodys.back();
//		_bodys.pop_back();
//		tailRef->SetPosition(position);
//		_bodys.emplace_front(tailRef);
//	}
//}

void SnakeHead::MakeHeadData(Protocol::HeadData** OUT data)
{
	ActorInfo* actor = (*data)->mutable_actor();
	actor->set_objectid(objectId);
	actor->set_allocated_pos(new Vector2(position));

	(*data)->set_movespeed(_moveSpeed);
	(*data)->set_dir(_direction);

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

void SnakeHead::Tick(float deltaTime)
{
	// Actor::Tick(deltaTime);

	Move(deltaTime);

	//ASSERT_CRASH(_direction != Protocol::DirectionType::DIR_NONE);

	//int32 direction = static_cast<int32>(_direction);

	//// 이전 좌표 캐싱
	//_prevPos = position;

	//// y축 = 1, x축 = 0
	//// TODO : 이동 로직 정리
	//int32 axisType = direction / 3;
	//int32 valueType = (direction % 2 == 0) ? 1 : -1;

	//int32 xPos = position.x();
	//int32 yPos = position.y();

	//if (axisType == 0)
	//{
	//	xPos += (valueType * _moveSpeed * deltaTime) * 100;
	//}
	//else
	//{
	//	// TODO : 속도 및 좌표 보정치(0.66f, 100 ...) 매직 넘버화
	//	yPos += (valueType * (_moveSpeed * /* y축 이동이 체감상 너무 빨리서 속도 보정*/0.66f) * deltaTime) * 100;
	//}

	//position.set_x(xPos);
	//position.set_y(yPos);
	//
	//// 격자 간의 이동이 발생했는지 체크
	//Vector2 curPos;
	//curPos.set_x(xPos / 100);
	//curPos.set_y(yPos / 100);

	//Vector2 prevPos;
	//prevPos.set_x(_prevPos.x() / 100);
	//prevPos.set_y(_prevPos.y() / 100);

	//if (prevPos != curPos)
	//{
	//	int32 xDelta = curPos.x() - prevPos.x();
	//	int32 yDelta = curPos.y() - prevPos.y();

	//	// 좌표 값 사이의 부호 방향이 나옴
	//	int32 xValue = (xDelta != 0) ? (xDelta / ::abs(xDelta)) : 0;
	//	int32 yValue = (yDelta != 0) ? (yDelta / ::abs(yDelta)) : 0;

	//	// 2칸 이상 움직인 경우 1 이상의 값이 나옴
	//	int32 xCount = ::abs(xDelta) - 1;
	//	int32 yCount = ::abs(yDelta) - 1;

	//	// 이동 궤적 추가
	//	AddTrail(prevPos);

	//	// 2칸 이상 이동 시의 추가적인 이동 궤적 추가
	//	while (xCount > 0)
	//	{
	//		prevPos.set_x(prevPos.x() + xValue);
	//		Vector2 trail = prevPos;
	//		
	//		AddTrail(trail);
	//		--xCount;
	//	}

	//	while (yCount > 0)
	//	{
	//		prevPos.set_y(prevPos.y() + yValue);
	//		Vector2 trail = prevPos;

	//		AddTrail(trail);
	//		--yCount;
	//	}

	//	// TODO : 방향 전환 타이밍 수정, 2칸 이상을 움직인 경우에 사이 사이 방향 전환이 필요한 지를 파악해야 함
	//	// input queue에 방향 전환을 해야하는 경우 방향을 전환
	//	if(_inputQueue.empty() == false)
	//	{ 
	//		DirectionType newDir = _inputQueue.front();
	//		_inputQueue.pop();

	//		SetDirection(newDir);
	//	}
	//}

	if (WarningTrailPos())
		cout << "몸통 좌표 겹침 \n";
}

void SnakeHead::OnCollision(const Protocol::ObjectType& objectType)
{
	switch(objectType)
	{
		case Protocol::ObjectType::OBJECT_SNAKE_HEAD:
		case Protocol::ObjectType::OBJECT_ACTOR:
			MarkDestory();
			break;

		case Protocol::ObjectType::OBJECT_ITEM:
			++_addBodyCallCount;
			break;
	}

}

void SnakeHead::MarkDestory()
{
	Actor::MarkDestory();
}

const vector<Vector2> SnakeHead::GetCollisionCheckArea()
{
	vector<Vector2> ret;
	ret.emplace_back(position);
	if(position != prevPos)
	{
		for(int32 index = static_cast<int32>(_trailQueue.size())- 1 ; index > 0 ; --index)
		{
			ret.emplace_back(_trailQueue[index].pos());
			
			// 큐의 마지막(머리 바로 뒤의 몸통)부터 순서대로 순회하면서 궤적을 체크 영역으로 반환
			if(_trailQueue[index].pos() == prevPos)
				break;
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

		const Vector2 prev = position;
		
		// 좌우로 이동 중
		if (axisType == SnakeHead::AsixType::X)
		{
			int32 xNextPos = position.x();

			// 진행 방향으로 다음 그리드 정위치 값을 찾음
			int32 nextX = ((position.x() / 100) + moveValue) * 100;

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
			int32 nextY = ((position.y() / 100) + moveValue) * 100;

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
