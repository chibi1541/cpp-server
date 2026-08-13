#pragma once
#include "Actor.h"
#include "Struct.pb.h"

class SnakeBody : public Actor
{
public:
	SnakeBody(uint64 objectId, int32 x, int32 y);
	~SnakeBody();

	// _front 노드를 붙히고 _frontPos를 초기화
	void AttachFront(const weak_ptr<Actor>& front);
	void AttachRear(const weak_ptr<Actor>& rear) { _rear = rear; }

private:
	virtual void Tick(float deltaTime) override;

private:
	// 앞에 붙어있는 head 혹은 body
	weak_ptr<Actor> _front;

	// 뒷 부분
	weak_ptr<Actor> _rear;

	// front 노드의 이전 위치 = 몸통이 다음에 움직여야하는 위치
	Protocol::Vector2 _frontPos;
	
};

