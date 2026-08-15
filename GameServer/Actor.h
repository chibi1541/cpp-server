#pragma once
#include "Protocol.pb.h"

using namespace Protocol;

class Actor
{
public:
	Actor() = default;
	Actor(uint64 objectId, int32 x, int32 y);
	Actor(uint64 objectId, Vector2 pos);
	virtual ~Actor() = default;

	virtual void Tick(float deltaTime);

	const Protocol::Vector2& GetPosition() const { return position; }
	void SetPosition(const Protocol::Vector2& pos) { position = pos; }
	void SetPosition(int32 x, int32 y);

	const Protocol::Vector2& GetPrevPosition() const { return prevPos; }

	uint64 GetObjectId() const { return objectId; }
	void SetObjectId(uint64 objectId);

	virtual void MarkDestory();
	bool IsActive() const { return isExpired == false; }

	virtual void OnCollision(const Protocol::ObjectType& objectType);

	ObjectType GetObjecType() const;

	virtual const vector<Vector2> GetCollisionCheckArea() const;

protected:
	uint64						objectId;
	Protocol::Vector2			position;
	bool						isExpired = false;

	// 이전 Tick의 좌표를 캐싱
	Vector2						prevPos;
};

