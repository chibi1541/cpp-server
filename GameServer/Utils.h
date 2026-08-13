#pragma once

#include "Types.h"
#include "Struct.pb.h"

using namespace Protocol;

namespace Utils
{
	inline Vector2 MakeVector(int32 xPos, int32 yPos)
	{
		Vector2 newPos;
		newPos.set_x(xPos);
		newPos.set_y(yPos);

		return newPos;
	}
}

