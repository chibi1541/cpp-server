#pragma once

class Actor;

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
	void ProcessCollision(const vector<ActorRef>& actorList);

	// TODO : 뱀 머리를 대상으로 콜리전 판단하는 함수

private:
	// 두 액터가 충돌했는지 확인(테스트)하는 함수
	bool Test(const ActorRef& left, const ActorRef& right);
};

