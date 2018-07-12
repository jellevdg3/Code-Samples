#pragma once

#include "Enemy.h"

class EnemySergeant : public Enemy
{
public:
	EnemySergeant(Vector2* gPos, Vector2& lPos);
	~EnemySergeant();

	void onDestroyed(Bullet* bullet);
	void onCollision(Bullet* bullet);
	void onCollision(Player* player);

	void tick(float delta);
	void draw(Graphics2D* g2d);
};
