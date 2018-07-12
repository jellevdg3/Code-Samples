#pragma once

#include "game/GameHandler.h"
class Graphics2D;

#include "game/util/Graphics2D.h"

class GameState
{
public:
	GameState();
	virtual ~GameState();

	virtual void tick(float delta) = 0;
	virtual void draw(Graphics2D* g2d) = 0;
};
