#include "Enemy.h"

#include <stdlib.h>

#include "game/Resources.h"
#include "game/world/World.h"
#include "FlyHandler.h"
#include "Bullet.h"
#include "Player.h"
#include "Explosion.h"

Enemy::Enemy(Vector2* gPos, Image* img1, Image* img2)
{
	this->gPos = gPos;

	animationTime = 0;
	this->img = img1;
	this->img2 = img2;

	flyHandler = 0;

	shootingChance = 100;
	canRandomlyFlyOff = true;
}

Enemy::~Enemy()
{
	if (flyHandler != 0)
	{
		if (flyHandler->getOwner() == this)
		{
			flyHandler->setOwner(0);
		}

		flyHandler->remove();
		flyHandler = 0;
	}
}

Rectangle2& Enemy::getBounds()
{
	if (flyHandler != 0)
	{
		gBounds = Rectangle2(bounds);
		gBounds.getPos() += flyHandler->getPos();
		return gBounds;
	}
	else
	{
		gBounds = Rectangle2(bounds);
		gBounds.getPos() += *gPos;
		return gBounds;
	}
}

bool Enemy::isFlying()
{
	return flyHandler != 0;
}

void Enemy::swapImages()
{
	Image* _tmpImg = img;
	img = img2;
	img2 = _tmpImg;
}

Vector2* Enemy::getGlobalPos()
{
	return gPos;
}

Vector2& Enemy::getLocalPos()
{
	return bounds.getPos();
}

void Enemy::startFlying(FlyHandler* flyHandler)
{
	this->flyHandler = flyHandler;
	flyHandler->add();

	getWorld()->calcBounds();
}

void Enemy::stopFlying()
{
	if (flyHandler->getOwner() == this)
	{
		flyHandler->setOwner(0);
	}

	this->flyHandler->remove();
	this->flyHandler = 0;

	getWorld()->calcBounds();
}

void Enemy::onDestroyed(Bullet* bullet)
{
	Unit::onDestroyed(bullet);
}

void Enemy::onCollision(Bullet* bullet)
{
	Explosion* explosion = new Explosion(getBounds().getCenter());
	getWorld()->spawn(explosion);
}

void Enemy::onCollision(Player* player)
{
	Explosion* explosion = new Explosion(getBounds().getCenter());
	getWorld()->spawn(explosion);
}

void Enemy::tick(float delta)
{
	// fly
	if (flyHandler != 0)
	{
		if (flyHandler->isDone())
		{
			stopFlying();
		}
		else
		{
			if (getWorld()->getPlayer()->getBounds().collidesWith(this->getBounds()))
			{
				if (getWorld()->getPlayer()->isClipped())
				{
					getWorld()->getPlayer()->onCollision(this);
					this->onCollision(getWorld()->getPlayer());
					return;
				}
			}

			if (flyHandler->getOwner() == this)
			{
				flyHandler->tick(delta);
			}
			else if (flyHandler->getOwner() == 0)
			{
				flyHandler->setOwner(this);
				flyHandler->tick(delta);
			}
		}
	}
	else
	{
		if (canRandomlyFlyOff)
		{
			if (rand() % (50 * (getWorld()->getEnemies()->getLength() * 2)) == 1)
			{
				FlyHandler* flyHandler = new FlyHandler(this);
				startFlying(flyHandler);
			}
		}
	}

	// shoot
	if (isFlying() && flyHandler->getStatus() < 2)
	{
		if (shootingChance != -1 && isFlying() && rand() % shootingChance == 1)
		{
			World* world = getWorld();

			Bullet* bullet = new Bullet(Resources::IMG_ENEMY_BULLET, this, (LinkedList<Unit*>*)(world->getPlayers()), getBounds().getPos() + ((bounds.getSize() / 2.0f) - Vector2(0.0f, bounds.getSize().getY() / 2.0f)), Vector2(0, 10.0f));
			this->spawn(bullet);
			world->spawn(bullet);
		}
	}

	// animation
	this->animationTime++;
	if (this->animationTime > 30)
	{
		swapImages();

		this->animationTime = 0;
	}
}

void Enemy::draw(Graphics2D* g2d)
{
	if (flyHandler != 0)
	{
		g2d->drawImage(img, (int)(bounds.getLeft() + flyHandler->getPos().getX()), (int)(bounds.getTop() + flyHandler->getPos().getY()));
	}
	else
	{
		g2d->drawImage(img, (int)(gPos->getX() + bounds.getLeft()), (int)(gPos->getY() + bounds.getTop()));
	}
}
