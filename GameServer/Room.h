#pragma once
#include "JobQueue.h"
#include "SnakeHead.h"

class Room : public JobQueue
{
public:
	void Enter(PlayerRef player);
	void Leave(PlayerRef player);
	void Broadcast(SendBufferRef sendBuffer);

	vector<PlayerRef> GetPlayersLocked();

	PlayerRef GetPlayerLocked(uint64 playerId);

	void AddActor(ActorRef newActor);

	void Tick(float deltaTime);

	void SetDirection(uint64 objectId, Protocol::DirectionType newDir);

private:
	USE_LOCK;

	map<uint64, PlayerRef> _players;

	vector<ActorRef> _actors;

};

extern shared_ptr<Room> GRoom;

