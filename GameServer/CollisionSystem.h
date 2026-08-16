#pragma once

class Actor;
class FieldInfo;

class CollisionSystem
{
	struct CollisionPair
	{
		ActorRef actor;
		ActorRef other;
	};

public:
	CollisionSystem() = default;
	~CollisionSystem() = default;

	// 액터를 순회하면서 충돌을 확인하는 함수.
	void ProcessCollision(const vector<SnakeHeadRef>& actorList);

	// 뱀 머리랑 필드 정보를 기반으로 충돌 처리하는 함수
	void ProcessFieldCheck(const vector<SnakeHeadRef>& actorList, const FieldInfo* field, uint32 width, uint32 height);

private:
	// 두 액터가 충돌했는지 확인(테스트)하는 함수
	bool Test(const ActorRef& left, const ActorRef& right);
};

