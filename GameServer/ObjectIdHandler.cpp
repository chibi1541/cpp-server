#include "pch.h"
#include "ObjectIdHandler.h"

uint64 ObjectIdHandler::GenerateObjectId(Protocol::ObjectType type)
{
	static atomic<uint64> objectCount = 1;

	uint64 objectId =  (static_cast<uint64>(type) << 48) | (OBJECT_COUNT_MASK & objectCount++);

	return objectId;
}

Protocol::ObjectType ObjectIdHandler::GetObjectType(uint64 objectId)
{
	return static_cast<Protocol::ObjectType>((OBJECT_TYPE_MASK & objectId) >> 48);
}
