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
	_addRequestedActorList.emplace_back(player->headActor);
}

void Room::Leave(PlayerRef player)
{
	_players.erase(player->playerId);
	player->headActor->MarkDestory();
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

PlayerRef& Room::GetPlayer(uint64 playerId)
{
	return _players[playerId];
}

void Room::ReleaseHead(uint64 objectId)
{
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

	if (_players.size() > 0 && _itemCount < 100)
	{
		_elapsedTime += fElapsedTime;
		if (_spawnDelta <= _elapsedTime)
		{
			SetRandomSeed32();
			int32 genCount = RandomRange32(5, 15);
			int32 x = RandomRange32(1, 79);
			int32 y = RandomRange32(1, 29);

			for(int32 idx = 0 ; idx< genCount; ++idx)
			{
				int32 xPos = x + (idx / 5);
				int32 yPos = y + (idx % 5);


				if (GetFieldInfo(xPos, yPos).CheckFlag(Protocol::FIELD_ITEM) == false)
					AddFieldFlag(xPos, yPos, Protocol::FIELD_ITEM);
			}

			_elapsedTime = 0.f;
		}
	}

	RegisterHeads();

	for (const SnakeHeadRef& head : _heads)
	{
		head->Tick(fElapsedTime);
	}

	// 액터끼리(뱀 머리)의 충돌 판정
	collisionSys->ProcessCollision(_heads);

	// 자기영역 체크
	for (SnakeHeadRef head : _heads)
	{
		if(head->SelfCheck())
			head->OnCollision(ObjectType::OBJECT_SNAKE_BODY);
	}

	// 맵 정보와 충돌 판정
	collisionSys->ProcessFieldCheck(_heads, _field, WIDTH, HEIGHT);

	// 파괴 처리
	DestoryHeads();

	Protocol::S_UPDATE_ROOM updatePkt;

	const vector<PlayerRef>& players = GetPlayersLocked();

	updatePkt.mutable_players()->Reserve(static_cast<int32>(players.size()));
	for (const PlayerRef& player : players)
	{
		PlayerInfo* info = updatePkt.add_players();
		info->set_id(player->playerId);
		info->set_score(player->score);
		info->set_isgameover(player->bGameOver);
		Protocol::HeadData* data = info->mutable_head();
		player->headActor->MakeHeadData(OUT &data);
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

	if(updatePkt.players_size() > 0 || updatePkt.fielddata_size() > 0)
	{
		_itemCount = static_cast<uint32>(updatePkt.fielddata_size());
		SendBufferRef sendBuffer = ClientPacketHandler::MakeSendBuffer(updatePkt);
		DoAsync(&Room::Broadcast, sendBuffer);
	}

	ProcessGameResult();

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

	for (SnakeHeadRef& actor : _heads)
	{
		if (actor->GetObjectId() == objectId)
		{
			SnakeHeadRef head = static_pointer_cast<SnakeHead>(actor);
			// 입력 큐에 추가
			head->PushInput(newDir);
		}
	}
}

void Room::ProcessDestoryActor(SnakeHead* actor)
{
	// 게임 오버 처리
	PlayerRef player = actor->GetOwner();
	player->bGameOver = true;

	Protocol::Vector2 pos = actor->GetPosition();
	pos.set_x(pos.x()/ 100);
	pos.set_y(pos.y()/ 100);
	int32 index = pos.y() * WIDTH + pos.x();
	_field[index].AddFieldFlag(Protocol::FieldType::FIELD_ITEM);

	auto trails = actor->GetTrailQueue();
	for (auto trail : trails)
	{
		pos = trail.pos();
		index = pos.y() * WIDTH + pos.x();
		_field[index].AddFieldFlag(Protocol::FieldType::FIELD_ITEM);
		_field[index].RemoveFieldFlag(Protocol::FieldType::FIELD_OBSTACLE);
	}
}

void Room::ProcessGameResult()
{
	int32 score = -1;
	uint64 playerId = 0;
	for(auto player : GetPlayersLocked())
	{
		if(player->bGameOver == false)
		{
			return;
		}

		score = max(score, static_cast<int32>(player->score));
		if(score == player->score)
			playerId = player->playerId;
	}

	Protocol::S_GAME_RESULT pkt;
	pkt.set_winplayerid(playerId);
	SendBufferRef sendBuffer = ClientPacketHandler::MakeSendBuffer(pkt);
	DoAsync(&Room::Broadcast, sendBuffer);

	_bGameOver = true;
}

void Room::DestoryHeads()
{
	for (auto it = _heads.begin(); it < _heads.end();)
	{
		if (it->get()->IsActive() == false)
		{
			ProcessDestoryActor(it->get());
			it = _heads.erase(it);
			continue;
		}

		++it;
	}
}

void Room::RegisterHeads()
{
	if (_addRequestedActorList.size() > 0)
	{
		for (SnakeHeadRef newHead : _addRequestedActorList)
		{
			_heads.emplace_back(newHead);
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

void Room::StartGame()
{
	if(_bStartGame || _bNowCounting)
		return;

	_bStartGame = true;
	_bNowCounting = true;
	_remainCount = static_cast<float>(COUNT_NUB);
}
