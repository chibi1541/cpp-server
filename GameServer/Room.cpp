#include "pch.h"
#include "Room.h"
#include "Player.h"
#include "ClientPacketHandler.h"
#include "ObjectIdHandler.h"
#include "Item.h"
#include "CollisionSystem.h"
#include "Actor.h"
#include "SnakeHead.h"
#include "GameSession.h"


shared_ptr<Room> GRoom = make_shared<Room>();

Room::Room()
{
	// TODO : 룸 세분화 시에 별도의 초기화 로직 준비
	collisionSys = make_unique<CollisionSystem>();

	int32 fieldSize = WIDTH * HEIGHT;

	_field = new FieldInfo[fieldSize]();
	::memset(_field, 0, sizeof(FieldInfo) * fieldSize);

	for (int32 idx = 0; idx < fieldSize; ++idx)
	{
		// 왼쪽 오른쪽 테두리
		if ((idx % WIDTH) == 0 || (idx % WIDTH) == WIDTH - 1)
			_field[idx].AddFieldFlag(FieldType::FIELD_OBSTACLE);

		// 위 아래 테두리
		if ((idx / WIDTH) == 0 || (idx / WIDTH) == HEIGHT - 1)
			_field[idx].AddFieldFlag(FieldType::FIELD_OBSTACLE);
	}
}

Room::~Room()
{
	delete[] _field;
}

void Room::Enter(PlayerRef player)
{
	_players[player->playerId] = player;
	_actors.push_back(player->headActor);
	_heads.push_back(player->headActor);
}

void Room::Leave(PlayerRef player)
{
	_players.erase(player->playerId);
	static_pointer_cast<Actor>(player->headActor)->MarkDestory();
	player->ReleaseControlActor();
}

void Room::Broadcast(SendBufferRef sendBuffer)
{
	for (auto player : _players)
	{
		player.second->ownerSession->Send(sendBuffer);
	}
}

vector<PlayerRef> Room::GetPlayersLocked()
{
	WRITE_LOCK;

	vector<PlayerRef> ret;
	ret.reserve(_players.size());

	for (auto p : _players)
	{
		ret.push_back(p.second);
	}

	return ret;
}

PlayerRef Room::GetPlayerLocked(uint64 playerId)
{
	WRITE_LOCK;

	return _players[playerId];
}

void Room::AddActor(ActorRef newActor)
{
	_addRequestedActorList.emplace_back(newActor);
}


void Room::ReleaseActor(uint64 objectId)
{
	for (auto it = _actors.begin(); it < _actors.end(); )
	{
		if (it->get()->GetObjectId() == objectId)
		{
			it = _actors.erase(it);
			break;
		}

		++it;
	}

	for (auto it = _heads.begin(); it < _heads.end(); )
	{
		if (it->get()->GetObjectId() == objectId)
		{
			it = _heads.erase(it);
			break;
		}

		++it;
	}
}


void Room::Tick(float deltaTime)
{
	uint64 elapsedTime = (_prevElapsedTime != 0) ? (GetTickCount64() - _prevElapsedTime) : 0;
	float fElapsedTime = static_cast<float>(elapsedTime) / 1000.f;

	// TODO : 아이템 추가 로직 수정
	if (_players.size() > 0 && _itemCount < 150)
	{
		_elapsedTime += fElapsedTime;
		if (_spawnDelta <= _elapsedTime)
		{
			int32 x = RandomRange32(1, 79);
			int32 y = RandomRange32(1, 29);

			if(GetFieldInfo(x, y).CheckFlag(Protocol::FIELD_ITEM) == false)
				AddFieldFlag(x, y, Protocol::FIELD_ITEM);
		}

	}

	RegisterActors();


	for (ActorRef actor : _actors)
	{
		actor->Tick(fElapsedTime);
	}

	// 액터끼리(뱀 머리)의 충돌 판정
	collisionSys->ProcessCollision(_actors);

	// 자기영역 체크
	for (SnakeHeadRef head : _heads)
	{
		if(head->SelfCheck())
			head->OnCollision(ObjectType::OBJECT_SNAKE_BODY);
	}

	// 맵 정보와 충돌 판정
	collisionSys->ProcessFieldCheck(_actors, _field, WIDTH, HEIGHT);

	// 파괴 처리
	DestoryActors();


	Protocol::S_UPDATE_ROOM updatePkt;

	for (SnakeHeadRef head : _heads)
	{
		updatePkt.mutable_heads()->Reserve(_heads.size());
		Protocol::HeadData* data = updatePkt.add_heads();
		head->MakeHeadData(&data);
	}

	for (uint32 index = 0; index < WIDTH * HEIGHT; ++index)
	{
		if(false == _field[index].CheckFlag(FieldType::FIELD_ITEM))
			continue;

		Protocol::FieldData* field = updatePkt.add_fielddata();
		field->set_fieldflag(_field[index].GetFlag());
		Protocol::Vector2* pos = field->mutable_pos();
		pos->set_x(index % WIDTH);
		pos->set_y(index / WIDTH);
	}

	if(updatePkt.heads_size() > 0 || updatePkt.fielddata_size() > 0)
	{
		_itemCount = static_cast<uint32>(updatePkt.fielddata_size());
		SendBufferRef sendBuffer = ClientPacketHandler::MakeSendBuffer(updatePkt);
		DoAsync(&Room::Broadcast, sendBuffer);
	}

	_prevElapsedTime = GetTickCount64();

	DoTimer(50, &Room::Tick, 0.05f);
}

void Room::BeginPlay()
{
	// TODO : begin play
}

void Room::SetDirection(uint64 objectId, Protocol::DirectionType newDir)
{
	if (ObjectIdHandler::GetObjectType(objectId) != Protocol::ObjectType::OBJECT_SNAKE_HEAD)
		return;

	for (ActorRef actor : _actors)
	{
		if (actor->GetObjectId() == objectId)
		{
			SnakeHeadRef head = static_pointer_cast<SnakeHead>(actor);
			// 입력 큐에 추가
			head->PushInput(newDir);
		}
	}
}

void Room::CheckCollision()
{
	for (SnakeHeadRef head : _heads)
	{
		// TODO : 파괴 예약 처리

		for (auto it = _actors.begin(); it < _actors.end();)
		{
			if (false == ComparePos(head->GetPosition(), it->get()->GetPosition()))
			{
				++it;
				continue;
			}

			if (head->GetObjectId() == it->get()->GetObjectId())
			{
				++it;
				continue;
			}

			Protocol::ObjectType type = ObjectIdHandler::GetObjectType(it->get()->GetObjectId());
			switch (type)
			{
			case Protocol::ObjectType::OBJECT_ITEM:
			{
				// TODO : 점수 올려주는 처리

				// TODO : 직접 vector에서 빼주는 처리 지양, 수정 방향 강구
				Protocol::S_DESTROY_ACTOR pkt;
				pkt.set_id(it->get()->GetObjectId());
				SendBufferRef sendBuffer = ClientPacketHandler::MakeSendBuffer(pkt);
				DoAsync(&Room::Broadcast, sendBuffer);

				it = _actors.erase(it);
				continue;
			}

			case Protocol::ObjectType::OBJECT_SNAKE_HEAD:
				// TODO : Actor 쪽 뱀 머리 파괴 예약 처리

				break;
			case Protocol::ObjectType::OBJECT_SNAKE_BODY:
			case Protocol::ObjectType::OBJECT_WALL:
				// TODO : Head 쪽 뱀 머리 파괴 예약 처리

				break;
			}

			++it;
		}
	}
}

bool Room::ComparePos(const Protocol::Vector2& left, const Protocol::Vector2& right)
{
	// 100으로 나누어야 그리드에 맞게 판단이 가능
	Protocol::Vector2 lValue;
	lValue.set_x(left.x() / 100);
	lValue.set_y(left.y() / 100);

	Protocol::Vector2 rValue;
	rValue.set_x(right.x() / 100);
	rValue.set_y(right.y() / 100);

	return lValue == rValue;
}

void Room::ProcessDestoryActor(Actor* actor)
{
	Vector2 pos = actor->GetPosition();
	pos.set_x(pos.x()/ 100);
	pos.set_y(pos.y()/ 100);
	int32 index = pos.y() * WIDTH + pos.x();
	_field[index].AddFieldFlag(Protocol::FieldType::FIELD_ITEM);

	SnakeHead* head = dynamic_cast<SnakeHead*>(actor);
	auto trails = head->GetTrailQueue();
	for (auto trail : trails)
	{
		pos = trail.pos();
		index = pos.y() * WIDTH + pos.x();
		_field[index].AddFieldFlag(Protocol::FieldType::FIELD_ITEM);
		_field[index].RemoveFieldFlag(Protocol::FieldType::FIELD_OBSTACLE);
	}
}

void Room::DestoryActors()
{
	for (auto it = _actors.begin(); it < _actors.end();)
	{
		if (it->get()->IsActive() == false)
		{
			ProcessDestoryActor(it->get());
			it = _actors.erase(it);
			continue;
		}

		++it;
	}
}

void Room::RegisterActors()
{
	if (_addRequestedActorList.size() > 0)
	{
		for (ActorRef newActor : _addRequestedActorList)
		{
			_actors.emplace_back(newActor);
		}
	}

	_addRequestedActorList.clear();
}

void Room::AddFieldFlag(uint32 x, uint32 y, const Protocol::FieldType& flag)
{
	const uint32 index = y * WIDTH + x;
	_field[index].AddFieldFlag(flag);
}

void Room::RemoveFieldFlag(uint32 x, uint32 y, const Protocol::FieldType& flag)
{
	const uint32 index = y * WIDTH + x;
	_field[index].RemoveFieldFlag(flag);
}

const FieldInfo& Room::GetFieldInfo(uint32 x, uint32 y) const
{
	const uint32 index = y * WIDTH + x;
	return _field[index];
}
