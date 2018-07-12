#pragma once

#include "game/util/Vector2.h"
class Enemy;

// Fly movement in groups, and in an arc
class FlyHandler
{
public:
	FlyHandler(Enemy* owner);
	~FlyHandler();

	Vector2& getPos();

	bool isDone();
	void add();
	void remove();

	Enemy* getOwner();
	int getStatus();

	void setOwner(Enemy* owner);

	void tick(float delta);

private:
	int objects;

	Vector2* gPos;

	Enemy* owner;
	float startFlyingArc;
	float endFlyingArc;
	float flyingArc;
	float flyingDiff;

	int flyingStatus;

	Vector2 flyPos;
	float flySpeedX;
	Vector2 flySpeed;
	Vector2 gotoFlySpeed;
};
