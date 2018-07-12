#pragma once

class World;

#include "game/util/Rectangle2.h"
#include "game/util/Graphics2D.h"

class WorldObject
{
public:
	WorldObject();
	virtual ~WorldObject();

	virtual Rectangle2& getBounds();

	virtual void tick(float delta) = 0;
	virtual void draw(Graphics2D* g2d) = 0;

protected:
	Image* img;
	Rectangle2 bounds;

	World* getWorld();
};

