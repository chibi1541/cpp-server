#include "pch.h"
#include "CollisionSystem.h"
#include "Actor.h"
#include "Struct.pb.h"
#include "Utils.h"

#define CHECK_VALUE 80

using namespace Protocol;

void CollisionSystem::ProcessCollision(const vector<ActorRef>& actorList)
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
        const ActorRef& left = actorList[i];
        if (nullptr == left || left->IsActive() == false)
        {
            continue;
        }
        
        for (uint32 j = i + 1; j < count; ++j)
        {
            const ActorRef& right = actorList[j];
            if (nullptr == right || right->IsActive() == false)
            {
                continue;
            }

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

    // 충돌 발생한 액터 목록 확인. 충돌한 액터가 없으면 함수 종료.
    if (collidedActorList.empty())
    {
        return;
    }

    // 충돌한 액터에 이벤트 전달
    for (const CollisionPair& pair : collidedActorList)
    {
        // 이미 삭제되거나 비활성화 된 액터는 제외
        if (!pair.actor->IsActive() || !pair.other->IsActive())
        {
            continue;
        }

        // 충돌 이벤트 전달
        pair.actor->OnCollision(pair.other);
        pair.other->OnCollision(pair.actor);
    }
}

void CollisionSystem::ProcessFieldCheck(const vector<ActorRef>& actorList, const FieldInfo* field, uint32 width, uint32 height)
{
	ASSERT_CRASH(field != nullptr);

	for(const ActorRef& actor : actorList)
	{
		
	}

}

bool CollisionSystem::Test(const ActorRef& left, const ActorRef& right)
{
    if (nullptr == left || nullptr == right)
    {
        return false;
    }

    // AABB (Axis Aligned Bounding Box)
    // 이번 프로젝트는 액터가 1문자이기 때문에 box check는 필요 없음

    //TODO : 보정 값에 방향치 추가

    // left의 위치보정
    const Vector2 leftRawPos = left->GetPosition();
    const int32 leftX = ((leftRawPos.x() % 100) >= CHECK_VALUE) ? (leftRawPos.x() / 100) + 1 : (leftRawPos.x() / 100);
    const int32 leftY = ((leftRawPos.y() % 100) >= CHECK_VALUE) ? (leftRawPos.y() / 100) + 1 : (leftRawPos.y() / 100);
    Vector2 upperLeftPos = Utils::MakeVector(leftX, leftY);
    Vector2 curLeftPos = Utils::MakeVector(leftRawPos.x()/100, leftRawPos.y() / 100);

    // right의 위치보정
    const Vector2 rightRawPos = right->GetPosition();
    const int32 rightX = ((rightRawPos.x() % 100) >= CHECK_VALUE) ? (rightRawPos.x() / 100) + 1 : (rightRawPos.x() / 100);
    const int32 rightY = ((rightRawPos.y() % 100) >= CHECK_VALUE) ? (rightRawPos.y() / 100) + 1 : (rightRawPos.y() / 100);
    Vector2 upperRightPos = Utils::MakeVector(rightX, rightY);
    Vector2 curRightPos = Utils::MakeVector(rightRawPos.x() / 100, rightRawPos.y() / 100);

    const int leftXMin = (upperLeftPos.x() < curLeftPos.x()) ? upperLeftPos.x() : curLeftPos.x();
    const int leftXMax = (upperLeftPos.x() > curLeftPos.x()) ? upperLeftPos.x() : curLeftPos.x();

    const int rightXMin = (upperRightPos.x() < curRightPos.x()) ? upperRightPos.x() : curRightPos.x();
    const int rightXMax = (upperRightPos.x() > curRightPos.x()) ? upperRightPos.x() : curRightPos.x();

    if (rightXMin > leftXMax)
    {
        return false;
    }

    if (rightXMax < leftXMin)
    {
        return false;
    }

    const int leftYMin = (upperLeftPos.y() < curLeftPos.y()) ? upperLeftPos.y() : curLeftPos.y();
    const int leftYMax = (upperLeftPos.y() > curLeftPos.y()) ? upperLeftPos.y() : curLeftPos.y();

    const int rightYMin = (upperRightPos.y() < curRightPos.y()) ? upperRightPos.y() : curRightPos.y();
    const int rightYMax = (upperRightPos.y() > curRightPos.y()) ? upperRightPos.y() : curRightPos.y();

    if (rightYMin > leftYMax)
    {
        return false;
    }

    if (rightYMax < leftYMin)
    {
        return false;
    }

    return true;
}
