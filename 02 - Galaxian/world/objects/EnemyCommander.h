#pragma once

#include "Enemy.h"

class EnemyCommander : public Enemy
{
public:
	EnemyCommander(Vector2* gPos, Vector2& lPos);
	~EnemyCommander();

	void onDestroyed(Bullet* bullet);
	void onCollision(Bullet* bullet);
	void onCollision(Player* player);

	void tick(float delta);
	void draw(Graphics2D* g2d);
};
