#pragma once
#include "Actor.h"

class Item : public Actor
{
public:
	Item(uint64 objectId, int32 x, int32 y, int32 score);
	~Item();

	int32 GetScore() const { return _score; }
	void SetScore(int32 score) { _score = score; }

public:
	

private:
	// 획득 시에 추가될 스코어 점수
	int32 _score = 0;

};

