#pragma once
#include "Enum.pb.h"

class Player
{
public:

	uint64					playerId = 0;
	string					name;
	GameSessionRef			ownerSession; // warning : Cycle
	int32					xPos = 0;
	int32					yPos = 0;
};

