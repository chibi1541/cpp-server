#pragma once

class Player
{
public:
	~Player();
	
	void ReleaseControlActor();

public:

	uint64					playerId = 0;
	string					name;
	uint32					score = 0;
	GameSessionRef			ownerSession; // warning : Cycle
	SnakeHeadRef			headActor; // 순환 참조 주의
};

