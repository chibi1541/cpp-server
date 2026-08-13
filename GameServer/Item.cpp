#include "pch.h"
#include "Item.h"

Item::Item(uint64 objectId, int32 x, int32 y, int32 score)
	: Actor(objectId, x, y), _score(score)
{
}

Item::~Item()
{
}
