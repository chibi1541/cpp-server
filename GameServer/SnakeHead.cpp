#include "pch.h"
#include "SnakeHead.h"

SnakeHead::SnakeHead(PlayerRef owner)
	: owner(owner)
{
}

SnakeHead::SnakeHead(uint64 objectId, int x, int y, PlayerRef owner)
	:Actor(objectId, x, y), owner(owner), _xPos(static_cast<float>(x)), _yPos(static_cast<float>(y))
{

}

SnakeHead::~SnakeHead()
{

}

void SnakeHead::SetDirection(Protocol::DirectionType newDirection)
{
	_direction = newDirection;
}

void SnakeHead::Tick(float deltaTime)
{
	Actor::Tick(deltaTime);

	ASSERT_CRASH(_direction != Protocol::DirectionType::DIR_NONE);
	int32 direction = static_cast<int32>(_direction);

	// y축 = 1, x축 = 0
	int32 axisType = direction / 3;
	int32 valueType = (direction % 2 == 0) ? 1 : -1;

	if (axisType == 0)
	{
		_xPos = _xPos + (static_cast<float>(valueType) * _moveSpeed * deltaTime);
	}
	else
	{
		_yPos = _yPos + (static_cast<float>(valueType) * _moveSpeed * deltaTime);
	}

	position.set_x(static_cast<int32_t>(_xPos));
	position.set_y(static_cast<int32_t>(_yPos));
}
