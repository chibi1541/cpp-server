#include "pch.h"
#include "SnakeBody.h"

SnakeBody::SnakeBody(uint64 objectId, int32 x, int32 y)
	: Actor(objectId, x, y)
{

}

SnakeBody::~SnakeBody()
{
}

void SnakeBody::AttachFront(const weak_ptr<Actor>& front)
{
	_front = front;

	if (ActorRef ref = _front.lock())
	{
		_frontPos = ref->GetPosition();
	}
}

void SnakeBody::Tick(float deltaTime)
{
	// Actor::Tick(deltaTime);
	ActorRef ref = _front.lock();
	if (ref != nullptr)
	{
		// early out
		if (ref->GetPosition() == _frontPos)
			return;
	}
	else
	{
		// 이 경우는 현재 액터가 살아있을 필요가 없음
		// TODO : 파괴 처리

		return;
	}

	// 액터를 이동시키고 front에 대한 정보를 갱신
	SetPosition(_frontPos);
	_frontPos = ref->GetPosition();
}
