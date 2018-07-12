#include "WorldObject.h"

#include "game/GameHandler.h"
#include "game/gamestates/GameStateInGame.h"
#include "game/world/World.h"

WorldObject::WorldObject()
{
}

WorldObject::~WorldObject()
{
}

Rectangle2& WorldObject::getBounds()
{
	return bounds;
}

World* WorldObject::getWorld()
{
	return ((GameStateInGame*)(instanceGameHandler->getGameState()))->getWorld();
}
