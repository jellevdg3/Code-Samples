#pragma once

#include "WorldObject.h"

class Explosion : public WorldObject
{
public:
	Explosion(Vector2& pos);
	~Explosion();

	void tick(float delta);
	void draw(Graphics2D* g2d);

private:
	int maxX;
	int maxY;
	int curX;
	int curY;

	int spriteWidth;
	int spriteHeight;
	int imageWidth;
	int imageHeight;
};
