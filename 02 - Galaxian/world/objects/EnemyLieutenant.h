#pragma once

#include "Enemy.h"

class EnemyLieutenant : public Enemy
{
public:
	EnemyLieutenant(Vector2* gPos, Vector2& lPos);
	~EnemyLieutenant();

	void onDestroyed(Bullet* bullet);
	void onCollision(Bullet* bullet);
	void onCollision(Player* player);

	void tick(float delta);
	void draw(Graphics2D* g2d);
};
