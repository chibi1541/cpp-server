#pragma once
#include "JobQueue.h"

class FieldInfo
{
public:
	void AddFieldFlag(const Protocol::FieldType& flag)
	{
		fieldFlag = fieldFlag | flag;
	}

	void RemoveFieldFlag(const Protocol::FieldType& flag)
	{
		fieldFlag = fieldFlag & ~flag;
	}

	bool CheckFlag(const Protocol::FieldType& flag) const
	{
		return fieldFlag & flag;
	}

	bool IsGround() const
	{
		return fieldFlag == Protocol::FieldType::FIELD_GROUND;
	}

	uint32 GetFlag() const {return fieldFlag;}

private:
	uint32 fieldFlag = Protocol::FieldType::FIELD_GROUND;
};

class Room : public JobQueue
{
	enum { WIDTH = 80, HEIGHT = 30 };

public:
	Room();
	~Room();

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

	void ProcessDestoryActor(Actor* actor);

	void DestoryActors();

	void RegisterActors();

	uint32 GetFieldWidth() const { return WIDTH; }
	uint32 GetFieldHeight() const { return HEIGHT; }

	void AddFieldFlag(uint32 x, uint32 y, const Protocol::FieldType& flag);
	void RemoveFieldFlag(uint32 x, uint32 y, const Protocol::FieldType& flag);
	const FieldInfo* GetField() const { return _field; }
	const FieldInfo& GetFieldInfo(uint32 x, uint32 y) const;

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

	// TODO : 룸 사이즈 체크 시스템화
	uint32 _width = 0;
	uint32 _height = 0;

	uint32 _itemCount = 0;

	FieldInfo* _field;
};

extern shared_ptr<Room> GRoom;

