#pragma once

#include "Unit.h"

class Enemy;

class Player : public Unit
{
public:
	Player();
	~Player();

	void onDestroyed(Bullet* bullet);
	void onCollision(Bullet* bullet);
	void onCollision(Enemy* enemy);

	int getLives();
	bool isClipped();

	void tick(float delta);
	void draw(Graphics2D* g2d);

private:
	bool canSpawnBullet;
	int lives;
	int noClipTime;
};
