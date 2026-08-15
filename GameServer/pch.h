#pragma once

#define WIN32_LEAN_AND_MEAN             // Exclude rarely-used stuff from Windows headers

#pragma warning(disable: 4251)

#ifdef _DEBUG
#pragma comment(lib, "ServerCore\\Debug\\ServerCore.lib")
#pragma comment(lib, "Protobuf\\Debug\\libprotobufd.lib")
#else
#pragma comment(lib, "ServerCore\\Release\\ServerCore.lib")
#pragma comment(lib, "Protobuf\\Release\\libprotobuf.lib")
#endif

#include "CorePch.h"
#include "Enum.pb.h"
#include "Struct.pb.h"

using GameSessionRef = shared_ptr<class GameSession>;
using PlayerRef = shared_ptr<class Player>;
using SnakeHeadRef = shared_ptr<class SnakeHead>;
using ActorRef = shared_ptr<class Actor>;
using SnakeHeadRef = shared_ptr<class SnakeHead>;
using SnakeBodyRef = shared_ptr<class SnakeBody>;

inline bool operator==(const Protocol::Vector2& left, const Protocol::Vector2& right)
{
	return (left.x() == right.x() && left.y() == right.y());
}

inline bool operator!=(const Protocol::Vector2& left, const Protocol::Vector2& right)
{
	return !(left == right);
}