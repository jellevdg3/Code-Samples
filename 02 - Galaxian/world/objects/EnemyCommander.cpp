#include "EnemyCommander.h"

#include <stdlib.h>

#include "game/Resources.h"
#include "game/world/World.h"
#include "EnemyLieutenant.h"
#include "FlyHandler.h"

EnemyCommander::EnemyCommander(Vector2* gPos, Vector2& lPos) : 
Enemy(gPos, Resources::IMG_ENEMY_00A, Resources::IMG_ENEMY_00A)
{
	bounds.getPos() = lPos;

	bounds.getSize() = Vector2((float)(img->GetWidth()), (float)(img->GetHeight()));

	shootingChance = -1;
	canRandomlyFlyOff = false;
}

EnemyCommander::~EnemyCommander()
{
}

void EnemyCommander::onDestroyed(Bullet* bullet)
{
	Enemy::onDestroyed(bullet);
}

void EnemyCommander::onCollision(Bullet* bullet)
{
	Enemy::onCollision(bullet);
	getWorld()->remove(this);
	getWorld()->addScore(300);
	delete this;
}

void EnemyCommander::onCollision(Player* player)
{
	getWorld()->remove(this);
	Enemy::onCollision(player);
	delete this;
}

void EnemyCommander::tick(float delta)
{
	Enemy::tick(delta);

	if (!isFlying())
	{
		if (rand() % (10 * (getWorld()->getEnemies()->getLength() * 2)) == 1)
		{
			FlyHandler* flyHandler = new FlyHandler(this);

			LinkedListIterator<EnemyLieutenant*> it = getWorld()->getEnemyLieutenants()->getIterator();
			while (it.hasNext())
			{
				EnemyLieutenant* obj = it.next();
				if (!obj->isFlying() && obj->getBounds().getPos().distanceTo(this->getBounds().getPos()) < 100.0f)
				{
					obj->startFlying(flyHandler);
				}
			}

			this->startFlying(flyHandler);
		}
	}
}

void EnemyCommander::draw(Graphics2D* g2d)
{
	Enemy::draw(g2d);
}
