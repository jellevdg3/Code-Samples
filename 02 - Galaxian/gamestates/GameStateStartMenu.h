#pragma once

#include "GameState.h"

class GameStateStartMenu : public GameState
{
public:
	GameStateStartMenu();
	~GameStateStartMenu();

	void tick(float delta);
	void draw(Graphics2D* g2d);
};
