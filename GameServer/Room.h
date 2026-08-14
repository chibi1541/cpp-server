#pragma once
#include "JobQueue.h"
#include "SnakeHead.h"

class Room : public JobQueue
{
public:
	Room();

	void Enter(PlayerRef player);
	void Leave(PlayerRef player);
	void Broadcast(SendBufferRef sendBuffer);

	void AddActor(ActorRef newActor);
	void ReleaseActor(uint64 objectId);

	vector<PlayerRef> GetPlayersLocked();

	PlayerRef GetPlayerLocked(uint64 playerId);

	void Tick(float deltaTime);

	void BeginPlay();

	void SetDirection(uint64 objectId, Protocol::DirectionType newDir);

	void CheckCollision();

	bool ComparePos(const Protocol::Vector2& left, const Protocol::Vector2& right);

	void DestoryActors();

	void RegisterActors();


private:
	USE_LOCK;

	map<uint64, PlayerRef> _players;

	vector<ActorRef> _actors;
	vector<SnakeHeadRef> _heads;

	vector<ActorRef> _addRequestedActorList;

	// temp
	float _spawnDelta = 3.f;
	float _elapsedTime = 0.f;

	uint64 _prevElapsedTime = 0;

	unique_ptr<class CollisionSystem> collisionSys;
};

extern shared_ptr<Room> GRoom;

