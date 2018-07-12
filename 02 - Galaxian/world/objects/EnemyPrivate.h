#pragma once

#include "Enemy.h"

class EnemyPrivate : public Enemy
{
public:
	EnemyPrivate(Vector2* gPos, Vector2& lPos);
	~EnemyPrivate();

	void onDestroyed(Bullet* bullet);
	void onCollision(Bullet* bullet);
	void onCollision(Player* player);

	void tick(float delta);
	void draw(Graphics2D* g2d);
};
