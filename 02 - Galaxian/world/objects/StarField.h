#pragma once

class Graphics2D;

#include "game/util/SimpleList.h"
#include "game/util/Vector2.h"

class Star
{
public:
	Vector2 pos;
	int color;
};

class StarField
{
public:
	StarField();
	~StarField();

	void tick(float delta);
	void draw(Graphics2D* g2d);

private:
	int lists;

	int* timers;
	SimpleList<Star>** starLists;
};
