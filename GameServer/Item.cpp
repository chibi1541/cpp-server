#include "pch.h"
#include "Item.h"
#include "ClientPacketHandler.h"
#include "Room.h"

Item::Item(uint64 objectId, int32 x, int32 y, int32 score)
	: Actor(objectId, x, y), _score(score)
{
}

Item::~Item()
{
}

//void Item::OnCollision(const ActorRef& other)
//{
//	// TODO : 점수 올려주는 처리
//	
//	MarkDestory();
//
//	Protocol::S_DESTROY_ACTOR pkt;
//	pkt.set_id(objectId);
//	SendBufferRef sendBuffer = ClientPacketHandler::MakeSendBuffer(pkt);
//	GRoom->DoAsync(&Room::Broadcast, sendBuffer);
//}
