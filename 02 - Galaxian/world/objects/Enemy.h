#pragma once

#include "Unit.h"

class FlyHandler;
class Player;

class Enemy : public Unit
{
public:
	Enemy(Vector2* gPos, Image* img1, Image* img2);
	virtual ~Enemy();

	Rectangle2& getBounds();

	bool isFlying();

	void startFlying(FlyHandler* flyHandler);
	void stopFlying();

	Vector2* getGlobalPos();
	Vector2& getLocalPos();

	void swapImages();

	void onDestroyed(Bullet* bullet);
	void onCollision(Bullet* bullet);
	virtual void onCollision(Player* player);

	void tick(float delta);
	void draw(Graphics2D* g2d);

protected:
	Vector2* gPos;
	Rectangle2 gBounds;

	Image* img2;
	int animationTime;

	int shootingChance;

	bool canRandomlyFlyOff;
	FlyHandler* flyHandler;
};
