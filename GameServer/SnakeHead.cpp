#include "pch.h"
#include "SnakeHead.h"

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
	_direction = newDirection;

	cout << "New Dir : " << _direction << " Pos x : " << position.x() << " Pos y : " << position.y() << "\n";
}

void SnakeHead::Tick(float deltaTime)
{
	// Actor::Tick(deltaTime);

	ASSERT_CRASH(_direction != Protocol::DirectionType::DIR_NONE);
	int32 direction = static_cast<int32>(_direction);

	// y축 = 1, x축 = 0
	int32 axisType = direction / 3;
	int32 valueType = (direction % 2 == 0) ? 1 : -1;

	int32 xPos = position.x();
	int32 yPos = position.y();

	if (axisType == 0)
	{
		xPos = xPos + (valueType * _moveSpeed * deltaTime) * 100;
	}
	else
	{
		yPos = yPos + (valueType * _moveSpeed * deltaTime) * 100;
	}

	position.set_x(static_cast<int32_t>(xPos));
	position.set_y(static_cast<int32_t>(yPos));
}
