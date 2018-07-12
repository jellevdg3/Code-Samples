#pragma once

#include "GameState.h"

class World;

class GameStateInGame : public GameState
{
public:
	GameStateInGame();
	~GameStateInGame();

	World* getWorld();

	void tick(float delta);
	void draw(Graphics2D* g2d);

private:
	World* world;
};