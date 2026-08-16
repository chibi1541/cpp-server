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

	gameSession->_room = GRoom;

	int x = RandomRange32(5, 75);
	int y = RandomRange32(3, 27);

	// TODO : spawn 좌표 변경
	SnakeHeadRef snakeActor = MakeShared<SnakeHead>(
		ObjectIdHandler::GenerateObjectId(Protocol::ObjectType::OBJECT_SNAKE_HEAD), x * 100, y* 100, gameSession->_player);
	gameSession->_player->headActor = snakeActor;

	GRoom->DoAsync(&Room::Enter, gameSession->_player);
	{
		Protocol::S_ENTER_GAME enterGamePkt;
		enterGamePkt.set_success(true);

		enterGamePkt.set_width(GRoom->GetFieldWidth());
		enterGamePkt.set_height(GRoom->GetFieldHeight());

		Protocol::PlayerInfo* playerInfo = enterGamePkt.add_players();
		playerInfo->set_id(gameSession->_player->playerId);
		playerInfo->set_name(gameSession->_player->name);
		playerInfo->set_score(gameSession->_player->score);

		Protocol::HeadData* headData = playerInfo->mutable_head();
		snakeActor->MakeHeadData(&headData);

		const vector<PlayerRef>& players = GRoom->GetPlayersLocked();
		for(const PlayerRef& player : players)
		{
			if(player->playerId == gameSession->_player->playerId)
				continue;
			
			Protocol::PlayerInfo* playerInfo = enterGamePkt.add_players();
			playerInfo->set_id(player->playerId);
			playerInfo->set_name(player->name);
			playerInfo->set_score(player->score);
			Protocol::HeadData* headData = playerInfo->mutable_head();
			player->headActor->MakeHeadData(&headData);

		}

		auto sendBuffer = ClientPacketHandler::MakeSendBuffer(enterGamePkt);
		gameSession->_player->ownerSession->Send(sendBuffer);
	}

	{
		PlayerRef player = gameSession->_player;

		Protocol::S_SPAWN_PLAYER spawnPkt;
		Protocol::PlayerInfo* playerInfo = spawnPkt.mutable_player();
		playerInfo->set_id(player->playerId);
		playerInfo->set_name(player->name);
		playerInfo->set_score(player->score);

		Protocol::HeadData* headData = playerInfo->mutable_head();
		player->headActor->MakeHeadData(&headData);

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