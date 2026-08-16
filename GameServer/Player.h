#pragma once
#include "Enum.pb.h"

class Player
{
public:
	~Player();
	
	void ReleaseControlActor();

	void AddScore();

public:
	uint64					playerId = 0;
	string					name;
	Protocol::PlayerColor	color;
	uint32					score = 0;
	GameSessionRef			ownerSession; // warning : Cycle
	SnakeHeadRef			headActor; // 순환 참조 주의
	bool					bGameOver = false;
};

