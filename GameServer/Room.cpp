#include "pch.h"
#include "Room.h"
#include "Player.h"
#include "GameSession.h"
#include "ClientPacketHandler.h"
#include "ObjectIdHandler.h"

shared_ptr<Room> GRoom = make_shared<Room>();

void Room::Enter(PlayerRef player)
{
	_players[player->playerId] = player;
	_actors.push_back(player->headActor);
}

void Room::Leave(PlayerRef player)
{
	_players.erase(player->playerId);
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
}

void Room::Tick(float deltaTime)
{
	Protocol::S_UPDATE_ROOM updatePkt;

	for (ActorRef actor : _actors)
	{
		actor->Tick(deltaTime);

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

	DoTimer(200, &Room::Tick, 0.2f);
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
