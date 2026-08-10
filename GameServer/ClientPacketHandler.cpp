#include "pch.h"
#include "ClientPacketHandler.h"
#include "BufferReader.h"
#include "BufferWriter.h"
#include "Player.h"
#include "Enum.pb.h"
#include "Room.h"
#include "GameSession.h"
#include <random>

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

		gameSession->_currentPlayer = playerRef;

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

	gameSession->_currentPlayer.get()->xPos = x;
	gameSession->_currentPlayer.get()->yPos = y;

	GRoom->DoAsync(&Room::Enter, gameSession->_currentPlayer);

	{
		Protocol::S_ENTER_GAME enterGamePkt;
		enterGamePkt.set_success(true);
		Protocol::Vector2* spawnPos = enterGamePkt.mutable_spawnpos();
		spawnPos->set_x(x);
		spawnPos->set_y(y);
		vector<PlayerRef> players = GRoom->GetPlayersLocked();
		for (const PlayerRef& player : players)
		{
			Protocol::PlayerInfo* info = enterGamePkt.add_players();
			if (info->id() == gameSession->_currentPlayer.get()->playerId)
				continue;

			info->set_id(player.get()->playerId);
			Protocol::Vector2* spawnPos = enterGamePkt.mutable_spawnpos();
			spawnPos->set_x(player.get()->xPos);
			spawnPos->set_y(player.get()->yPos);
		}

		auto sendBuffer = ClientPacketHandler::MakeSendBuffer(enterGamePkt);
		gameSession->_currentPlayer->ownerSession->Send(sendBuffer);
	}

	{
		Protocol::S_SPAWN_ACTOR spawnPkt;
		spawnPkt.set_id(gameSession->_currentPlayer.get()->playerId);
		Protocol::Vector2* spawnPos = spawnPkt.mutable_spawnpos();
		spawnPos->set_x(x);
		spawnPos->set_y(y);

		auto sendBuffer = ClientPacketHandler::MakeSendBuffer(spawnPkt);
		GRoom->DoAsync(&Room::Broadcast, sendBuffer);
	}

	return true;
}

bool Handle_C_SPAWN_ACTOR(PacketSessionRef& session, Protocol::C_SPAWN_ACTOR& pkt)
{


	return true;
}