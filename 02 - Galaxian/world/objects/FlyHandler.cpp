#include "FlyHandler.h"

#include <cmath>
#include <stdlib.h>

float pi = 3.14159265358979323846f;

#include "game/GameHandler.h"
#include "game/gamestates/GameStateInGame.h"
#include "game/world/World.h"
#include "Enemy.h"
#include "Player.h"

FlyHandler::FlyHandler(Enemy* owner)
{
	this->owner = owner;

	flyingStatus = 0;

	if (owner->getBounds().getPos().getX() < 400.0f)
	{
		// left
		flyingArc = pi;
		endFlyingArc = pi * 2;
		flyingDiff = 0.05f;
		
		flySpeed = Vector2(cosf(flyingArc - (pi / 1.5f)) * 4.0f, sinf(flyingArc - (pi / 1.5f)) * 4.0f);
	}
	else
	{
		// right
		flyingArc = 0.0f;
		endFlyingArc = -pi;
		flyingDiff = -0.05f;

		flySpeed = Vector2(cosf(flyingArc + (pi / 1.5f)) * 4.0f, sinf(flyingArc + (pi / 1.5f)) * 4.0f);
	}

	startFlyingArc = this->flyingArc;
	flyPos = Vector2(owner->getGlobalPos());
	gPos = owner->getGlobalPos();
}

FlyHandler::~FlyHandler()
{

}

void FlyHandler::add()
{
	objects++;
}

void FlyHandler::remove()
{
	objects--;
	if (objects == 0)
	{
		delete this;
	}
}

Vector2& FlyHandler::getPos()
{
	return flyPos;
}

Enemy* FlyHandler::getOwner()
{
	return owner;
}

int FlyHandler::getStatus()
{
	return flyingStatus;
}

void FlyHandler::setOwner(Enemy* owner)
{
	this->owner = owner;
}

bool FlyHandler::isDone()
{
	return flyingStatus == 3;
}

void FlyHandler::tick(float delta)
{
	// arc
	if (flyingStatus == 0)
	{
		flyingArc += flyingDiff;
		
		Vector2 lastFlyPos = Vector2(flyPos);
		flyPos = (*gPos + Vector2(cosf(flyingArc) * 75.0f, sinf(flyingArc) * 75.0f)) - Vector2(cosf(startFlyingArc) * 75.0f, sinf(startFlyingArc) * 75.0f);

		if (flyingDiff > 0 && flyingArc > endFlyingArc)
		{
			flyingArc = endFlyingArc;
			flyingStatus = 1;
			flySpeedX = 0;
		}

		if (flyingDiff < 0 && flyingArc < endFlyingArc)
		{
			flyingArc = endFlyingArc;
			flyingStatus = 1;
			flySpeedX = 0;
		}
	}
	// fly down
	else if (flyingStatus == 1)
	{
		// goto player
		if (owner != 0)
		{
			Player* player = ((GameStateInGame*)(instanceGameHandler->getGameState()))->getWorld()->getPlayer();
			float playerX = player->getBounds().getCenter().getX();
			float thisX = owner->getBounds().getCenter().getX();

			if (thisX < playerX)
			{
				flySpeedX += (playerX - thisX) / 2000.0f;
				if (flySpeedX > 5.0f)
				{
					flySpeedX = 5.0f;
				}
			}
			else
			{
				flySpeedX += (playerX - thisX) / 2000.0f;
				if (flySpeedX < -5.0f)
				{
					flySpeedX = -5.0f;
				}
			}
		}

		flySpeed.setX(flySpeedX);
		flyPos += flySpeed;

		if (owner->getBounds().getTop() > 600.0f)
		{
			flyingStatus = 2;
			flyPos.setY(flyPos.getY() - (650.0f + owner->getBounds().getH()));
		}
	}
	// go back to previous position
	else if (flyingStatus == 2)
	{
		flyPos += Vector2(0.0f, 2.0f);
		flyPos.setX(gPos->getX());
		if (owner->getBounds().getY() > gPos->getY() + owner->getLocalPos().getY())
		{
			flyingStatus = 3;
		}
	}
}
