#pragma once
#include "Enum.pb.h"

class Player
{
public:
	~Player();

public:

	uint64					playerId = 0;
	string					name;
	GameSessionRef			ownerSession; // warning : Cycle
	SnakeHeadRef			headActor; // 순환 참조 주의
};

