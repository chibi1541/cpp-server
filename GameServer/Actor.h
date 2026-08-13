#pragma once
#include "Protocol.pb.h"

class Actor
{
public:
	Actor() = default;
	Actor(uint64 objectId, int32 x, int32 y);
	virtual ~Actor() = default;

	virtual void Tick(float deltaTime);

	const Protocol::Vector2& GetPosition() const { return position; }
	void SetPosition(const Protocol::Vector2& pos) { position = pos; }
	void SetPosition(int32 x, int32 y);

	uint64 GetObjectId() const { return objectId; }
	void SetObjectId(uint64 objectId);

	virtual void MarkDestory();
	bool IsActive() const { return isExpired == false; }

	virtual void OnCollision(const ActorRef& other);

protected:
	uint64					objectId;
	Protocol::Vector2		position;
	bool					isExpired = false;
};

