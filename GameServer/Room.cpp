#include "pch.h"
#include "Room.h"
#include "Player.h"
#include "GameSession.h"
#include "ClientPacketHandler.h"
#include "ObjectIdHandler.h"
#include "Item.h"
#include <random>

shared_ptr<Room> GRoom = make_shared<Room>();

void Room::Enter(PlayerRef player)
{
	_players[player->playerId] = player;
	_actors.push_back(player->headActor);
	_heads.push_back(player->headActor);
}

void Room::Leave(PlayerRef player)
{
	_players.erase(player->playerId);
	ReleaseActor(player->headActor->GetObjectId());
}

void Room::Broadcast(SendBufferRef sendBuffer)
{
	for(auto p : _players)
	{
		p.second->ownerSession->Send(sendBuffer);
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
	_actors.emplace_back(newActor);
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

	// TODO : 액터 스폰하는 로직 수정
	{
		_elapsedTime += fElapsedTime;
		if (_spawnDelta <= _elapsedTime)
		{
			static std::random_device rd;
			static std::mt19937 gen(rd());
			std::uniform_int_distribution<int32> distx(0, 80);
			int32 x = distx(gen);

			std::uniform_int_distribution<int32> disty(0, 20);
			int32 y = disty(gen);

			_elapsedTime = 0.f;
			ActorRef item = ObjectPool<Item>::MakeShared(ObjectIdHandler::GenerateObjectId(Protocol::ObjectType::OBJECT_ITEM), x * 100, y * 100, 1);
			GRoom->DoAsync(&Room::AddActor, item);

			Protocol::S_SPAWN_ACTOR spawnPkt;
			spawnPkt.set_id(item->GetObjectId());
			Protocol::Vector2* spawnPos = spawnPkt.mutable_spawnpos();
			spawnPos->set_x(x * 100);
			spawnPos->set_y(y * 100);

			auto sendBuffer = ClientPacketHandler::MakeSendBuffer(spawnPkt);
			GRoom->DoAsync(&Room::Broadcast, sendBuffer);
		}

	}

	for (ActorRef actor : _actors)
	{
		actor->Tick(fElapsedTime);
	}

	CheckCollision();

	Protocol::S_UPDATE_ROOM updatePkt;

	for (ActorRef actor : _actors)
	{
		Protocol::ActorInfo* actorInfo = updatePkt.add_actors();
		actorInfo->set_objectid(actor->GetObjectId());
		Protocol::Vector2* pos = new Protocol::Vector2(actor->GetPosition());
		actorInfo->set_allocated_pos(pos);
	}

	if (updatePkt.actors_size() > 0)
	{
		SendBufferRef sendBuffer = ClientPacketHandler::MakeSendBuffer(updatePkt);
		DoAsync(&Room::Broadcast, sendBuffer);
	}


	_prevElapsedTime = GetTickCount64();

	DoTimer(50, &Room::Tick, 0.05f);
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
			head->SetDirection(newDir);
		}
	}
}

void Room::CheckCollision()
{
	for (SnakeHeadRef head : _heads)
	{
		// TODO : 파괴 예약 처리

		for (auto it = _actors.begin() ; it < _actors.end() ;)
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
