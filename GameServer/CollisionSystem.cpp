#include "pch.h"
#include "CollisionSystem.h"
#include "Actor.h"
#include "Struct.pb.h"
#include "Utils.h"
#include "Room.h"
#include "SnakeHead.h"

#define CHECK_VALUE 80

using namespace Protocol;

void CollisionSystem::ProcessCollision(const vector<SnakeHeadRef>& actorList)
{
	// 예외 처리
	if (actorList.empty())
	{
		return;
	}

	// 충돌한 액터의 이벤트를 한번에 정리해 전달하기 위한 배열
	vector<CollisionPair> collidedActorList;

	// 레벨에 배치된 액터 수
	const uint32 count = static_cast<uint32>(actorList.size());

	// 모든 액터를 순회하면서 충돌 검사
	for (uint32 i = 0; i < count; ++i)
	{
		const SnakeHeadRef& left = actorList[i];
		if (nullptr == left || left->IsActive() == false)
		{
			continue;
		}

		for (uint32 j = 0; j < count; ++j)
		{
			const SnakeHeadRef& right = actorList[j];
			if (nullptr == right || right->IsActive() == false)
			{
				continue;
			}

			if(left == right)
			{
				// 자기 영역이랑 붙힌건지 체크
				if (left->SelfCheck())
					left->OnCollision(ObjectType::OBJECT_SNAKE_BODY);
			}
			else
			{
				// 충돌 검사
				if (Test(left, right))
				{
					// 이벤트 발행할 목록에 추가할 데이터 생성.
					CollisionPair pair = {};
					pair.actor = left;
					pair.other = right;

					// 목록에 추가
					collidedActorList.emplace_back(pair);
				}
			}
		}
	}

	// 충돌 발생한 액터 목록 확인. 충돌한 액터가 없으면 함수 종료.
	if (collidedActorList.empty())
	{
		return;
	}

	// 충돌한 액터에 이벤트 전달
	for (const CollisionPair& pair : collidedActorList)
	{
		// 충돌 이벤트 전달
		pair.actor->OnCollision(ObjectType::OBJECT_SNAKE_HEAD);
	}
}

void CollisionSystem::ProcessFieldCheck(const vector<SnakeHeadRef>& actorList, const FieldInfo* field, uint32 width, uint32 height)
{
	ASSERT_CRASH(field != nullptr);

	for (const ActorRef& actor : actorList)
	{
		const vector<Vector2>& checkList = actor->GetCollisionCheckArea();
		for (const Vector2& pos : checkList)
		{
			uint32 index = (width * pos.y()) + pos.x();

			if (field[index].CheckFlag(Protocol::FieldType::FIELD_ITEM))
			{
				actor->OnCollision(ObjectType::OBJECT_ITEM);
				GRoom->RemoveFieldFlag(pos.x(), pos.y(), FieldType::FIELD_ITEM);
			}

			// 벽과 충돌
			if (field[index].CheckFlag(Protocol::FieldType::FIELD_OBSTACLE))
			{
				actor->OnCollision(ObjectType::OBJECT_WALL);
			}
		}
	}
}

bool CollisionSystem::Test(const ActorRef& left, const ActorRef& right)
{
	if (nullptr == left || nullptr == right)
	{
		return false;
	}

	SnakeHeadRef rHead = static_pointer_cast<SnakeHead>(right);
	const vector<Vector2> leftCheckArea = left->GetCollisionCheckArea();
	const vector<Vector2> rightCheckArea = rHead->GetSnakeArray();

	for (const Vector2& leftPos : leftCheckArea)
	{
		for (const Vector2 rightPos : rightCheckArea)
		{
			if (leftPos == rightPos)
				return true;
		}
	}

	return false;
}
