#include "pch.h"
#include "ClientPacketHandler.h"
#include "BufferReader.h"
#include "BufferWriter.h"
#include "Player.h"
#include "Enum.pb.h"
#include "Room.h"
#include "GameSession.h"
#include <random>
#include "SnakeHead.h"
#include "ObjectIdHandler.h"

PacketHandlerFunc GPacketHandler[UINT16_MAX];

bool Handle_INVALID(PacketSessionRef& session, BYTE* buffer, int32 len)
{
	
	// TODO : Log
	return false;
}

bool Handle_C_LOGIN(PacketSessionRef& session, Protocol::C_LOGIN& pkt)
{
	GameSessionRef gameSession = static_pointer_cast<GameSession>(session);

	// TODO : Validation 체크
	
	Protocol::S_LOGIN loginPkt;
	loginPkt.set_success(true);

	static Atomic<uint64> idGenerator = 1;

	{
		auto user = loginPkt.mutable_user();
		user->set_id(idGenerator);

		PlayerRef playerRef = MakeShared<Player>();
		playerRef->playerId = idGenerator++;
		playerRef->ownerSession = gameSession;

		gameSession->_player = playerRef;

	}

	auto sendBuffer = ClientPacketHandler::MakeSendBuffer(loginPkt);
	session->Send(sendBuffer);

	return true;
}

bool Handle_C_ENTER_GAME(PacketSessionRef& session, Protocol::C_ENTER_GAME& pkt)
{
	GameSessionRef gameSession = static_pointer_cast<GameSession>(session);

	// TODO : Validation

	gameSession->_room = GRoom;


	static std::random_device rd;
	static std::mt19937 gen(rd());
	std::uniform_int_distribution<int> distx(0, 80);
	int x = distx(gen);

	std::uniform_int_distribution<int> disty(0, 20);
	int y = disty(gen);

	SnakeHeadRef snakeActor = MakeShared<SnakeHead>(
		ObjectIdHandler::GenerateObjectId(Protocol::ObjectType::OBJECT_SNAKE_HEAD), x, y, gameSession->_player);
	gameSession->_player->headActor = snakeActor;

	GRoom->DoAsync(&Room::Enter, gameSession->_player);

	{
		Protocol::S_ENTER_GAME enterGamePkt;
		enterGamePkt.set_success(true);
		Protocol::PlayerInfo* playerInfo = new Protocol::PlayerInfo();
		playerInfo->set_id(gameSession->_player->playerId);
		Protocol::ActorInfo* actorInfo = new Protocol::ActorInfo();
		actorInfo->set_objectid(gameSession->_player->headActor->GetObjectId());
		Protocol::Vector2* pos = new Protocol::Vector2(gameSession->_player->headActor->GetPosition());
		actorInfo->set_allocated_pos(pos);
		playerInfo->set_allocated_actor(actorInfo);
		enterGamePkt.set_allocated_player(playerInfo);

		auto sendBuffer = ClientPacketHandler::MakeSendBuffer(enterGamePkt);
		gameSession->_player->ownerSession->Send(sendBuffer);
	}

	{
		Protocol::S_SPAWN_ACTOR spawnPkt;
		spawnPkt.set_id(gameSession->_player.get()->headActor->GetObjectId());
		Protocol::Vector2* spawnPos = spawnPkt.mutable_spawnpos();
		spawnPos->set_x(x);
		spawnPos->set_y(y);

		auto sendBuffer = ClientPacketHandler::MakeSendBuffer(spawnPkt);
		GRoom->DoAsync(&Room::Broadcast, sendBuffer);
	}

	return true;
}

bool Handle_C_MOVE_ACTOR(PacketSessionRef& session, Protocol::C_MOVE_ACTOR& pkt)
{
	GameSessionRef gameSession = static_pointer_cast<GameSession>(session);

	shared_ptr<Room> room = gameSession->_room.lock();

	room->DoAsync(&Room::SetDirection, gameSession->_player->headActor->GetObjectId(), pkt.newdir());

	return true;
}