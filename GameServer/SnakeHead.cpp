#include "pch.h"
#include "SnakeHead.h"
#include "Utils.h"

SnakeHead::SnakeHead(PlayerRef owner)
	: owner(owner)
{
}

SnakeHead::SnakeHead(uint64 objectId, int32 x, int32 y, PlayerRef owner)
	:Actor(objectId, x, y), owner(owner)
{

}

SnakeHead::~SnakeHead()
{

}

void SnakeHead::SetDirection(Protocol::DirectionType newDirection)
{
	Protocol::DirectionType prevDir = _direction;
	_direction = newDirection;

	int32 xPos = position.x();
	int32 yPos = position.y();

	// 0, 1 이 나오는데 0 이면 이전 이동방향이 +(Right, Bottom), 1이면 이전 이동 방향이 -(Left, Top)
	int32 dirType = static_cast<int32>(prevDir) % 2;

	if (dirType == 0)
	{
		xPos = ((xPos % 100) >= CHECK_VALUE) ? ((xPos / 100) + 1) * 100 : (xPos / 100) * 100;
		yPos = ((yPos % 100) >= CHECK_VALUE) ? ((yPos / 100) + 1) * 100 : (yPos / 100) * 100;
	}
	else
	{
		xPos = ((xPos % 100) <= 100 - CHECK_VALUE) ? (xPos / 100) * 100 : ((xPos / 100) + 1) * 100;
		yPos = ((yPos % 100) <= 100 - CHECK_VALUE) ? (yPos / 100) * 100 : ((yPos / 100) + 1) * 100;
	}


	SetPosition(Utils::MakeVector(xPos, yPos));

	cout << "New Dir : " << _direction << " Pos x : " << xPos << " Pos y : " << yPos << "\n";
}

void SnakeHead::Tick(float deltaTime)
{
	// Actor::Tick(deltaTime);

	ASSERT_CRASH(_direction != Protocol::DirectionType::DIR_NONE);
	int32 direction = static_cast<int32>(_direction);

	// y축 = 1, x축 = 0
	// TODO : 이동 로직 정리
	int32 axisType = direction / 3;
	int32 valueType = (direction % 2 == 0) ? 1 : -1;

	int32 xPos = position.x();
	int32 yPos = position.y();

	if (axisType == 0)
	{
		xPos += (valueType * _moveSpeed * deltaTime) * 100;
	}
	else
	{
		// TODO : 속도 및 좌표 보정치(0.66f, 100 ...) 매직 넘버화
		yPos += (valueType * (_moveSpeed * /* y축 이동이 체감상 너무 빨리서 속도 보정*/0.66f) * deltaTime) * 100;
	}

	position.set_x(xPos);
	position.set_y(yPos);
}
