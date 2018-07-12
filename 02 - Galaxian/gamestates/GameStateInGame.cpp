#include "GameStateInGame.h"

#include "game/world/World.h"

GameStateInGame::GameStateInGame()
{
	world = new World();
}

GameStateInGame::~GameStateInGame()
{
	if (world)
	{
		delete world;
		world = 0;
	}
}

World* GameStateInGame::getWorld()
{
	return world;
}

void GameStateInGame::tick(float delta)
{
	world->tick(delta);
}

void GameStateInGame::draw(Graphics2D* g2d)
{
	world->draw(g2d);
}
