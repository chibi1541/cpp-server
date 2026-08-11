#pragma once
#include "Protocol.pb.h"

class Actor
{
public:
	Actor() = default;
	Actor(uint64 objectId, int x, int y);
	virtual ~Actor() = default;

	virtual void Tick(float deltaTime);

	const Protocol::Vector2& GetPosition() const { return position; }
	void SetPosition(int32 x, int32 y);

	uint64 GetObjectId() const { return objectId; }
	void SetObjectId(uint64 objectId);

protected:
	uint64 objectId;
	Protocol::Vector2 position;
};

