#include "World.h"

#include <string>

#include "game/GameHandler.h"
#include "game/Resources.h"

#include "objects/Player.h"
#include "objects/WorldObject.h"
#include "objects/EnemyCommander.h"
#include "objects/EnemyLieutenant.h"
#include "objects/EnemySergeant.h"
#include "objects/EnemyPrivate.h"

World::World()
{
	spawn(new Player());

	calcBounds();

	spawnTime = 0;

	score = 0;
	level = 0;
}

World::~World()
{
	LinkedListIterator<WorldObject*> it = objects.getIterator();
	while (it.hasNext())
	{
		delete it.next();
	}
}

void World::spawn()
{
	// reset position
	gPos = Vector2(0.0f, 0.0f);

	int r = rand() % 2;
	if (r == 0)
	{
		curSpeed = Vector2(1.0f, 0.0f);
	}
	else
	{
		curSpeed = Vector2(-1.0f, 0.0f);
	}
	

	// commanders
	spawn(new EnemyCommander(&gPos, Vector2(296.0f, 40.0)));
	spawn(new EnemyCommander(&gPos, Vector2(446.0f, 40.0f)));

	// lieutenants
	for (int i = 0; i < 6; i++)
	{
		EnemyLieutenant* obj = new EnemyLieutenant(&gPos, Vector2(250.0f + (i * 50), 80.0f));
		if (i % 2 == 0)
		{
			obj->swapImages();
		}
		spawn(obj);
	}

	// sergants
	for (int i = 0; i < 8; i++)
	{
		EnemySergeant* obj = new EnemySergeant(&gPos, Vector2(200.0f + (i * 50), 110.0f));
		if ((i + 1) % 2 == 0)
		{
			obj->swapImages();
		}
		spawn(obj);
	}

	// privates
	for (int j = 0; j < 3; j++)
	{
		for (int i = 0; i < 10; i++)
		{
			EnemyPrivate* obj = new EnemyPrivate(&gPos, Vector2(150.0f + (i * 50), 140.0f + (j * 30)));
			if ((i + j) % 2 == 0)
			{
				obj->swapImages();
			}
			spawn(obj);
		}
	}

	calcBounds();
}

void World::spawn(WorldObject* obj)
{
	objects.add(obj);
}

void World::spawn(Bullet* obj)
{
	bullets.add(obj);
	this->spawn((WorldObject*)(obj));
}

void World::spawn(Enemy* obj)
{
	enemies.add(obj);
	this->spawn((WorldObject*)(obj));
}

void World::spawn(EnemyLieutenant* obj)
{
	enemyLieutenants.add(obj);
	this->spawn((Enemy*)(obj));
}

void World::spawn(Player* obj)
{
	if (players.getLength() == 0)
	{
		players.add(obj);
		this->spawn((WorldObject*)(obj));
		player = obj;
	}
}

void World::remove(WorldObject* obj)
{
	objects.remove(obj);
}

void World::remove(Bullet* obj)
{
	bullets.remove(obj);
	this->remove((WorldObject*)(obj));
}

void World::remove(Enemy* obj)
{
	enemies.remove(obj);
	this->remove((WorldObject*)(obj));

	calcBounds();
}

void World::remove(EnemyLieutenant* obj)
{
	enemyLieutenants.remove(obj);
	this->remove((Enemy*)(obj));
}

void World::remove(Player* obj)
{
	if (players.getLength() == 1)
	{
		players.remove(obj);
		this->remove((WorldObject*)(obj));
		player = 0;
	}
}

LinkedList<Enemy*>* World::getEnemies()
{
	return &enemies;
}

LinkedList<EnemyLieutenant*>* World::getEnemyLieutenants()
{
	return &enemyLieutenants;
}

LinkedList<Bullet*>* World::getBullets()
{
	return &bullets;
}

LinkedList<Player*>* World::getPlayers()
{
	return &players;
}

Player* World::getPlayer()
{
	return player;
}

void World::addScore(int score)
{
	this->score += score;
}

int World::getScore()
{
	return score;
}

// calculate enemy bounds in world
void World::calcBounds()
{
	// start
	bounds.getPos().gotoMax();
	bounds.getSize().gotoMin();

	// calculate
	inBoundsSize = 0;
	LinkedListIterator<Enemy*> it = enemies.getIterator();
	while (it.hasNext())
	{
		Enemy* enemy = it.next();

		if (!enemy->isFlying())
		{
			bounds.getPos().min(enemy->getBounds().getPos() - gPos);
			bounds.getSize().max(enemy->getBounds().getSize() + (enemy->getBounds().getPos() - gPos));

			inBoundsSize++;
		}
	}

	// finalize
	if (bounds.getPos().isMax())
	{
		bounds.getPos().reset();
	}

	if (bounds.getSize().isMin())
	{
		bounds.getSize().reset();
	}

	bounds.getSize() -= bounds.getPos();
}

void World::tick(float delta)
{
	LinkedListIterator<WorldObject*> it = objects.getIterator();
	while (it.hasNext())
	{
		WorldObject* obj = it.next();
		obj->tick(delta);
	}

	if (score > instanceGameHandler->getHighScore())
	{
		instanceGameHandler->setHighScore(score);
	}

	if (enemies.getLength() == 0)
	{
		spawnTime++;
		if (spawnTime > 120)
		{
			spawnTime = 0;
			level++;
			spawn();
		}
	}

	// move left and right
	if (inBoundsSize > 0)
	{
		if ((bounds.getPos() + gPos).getX() <= 0.0f)
		{
			if (curSpeed.getX() < 0.0f)
			{
				curSpeed.reverse();
			}
		}
		else if ((bounds.getPos() + gPos + bounds.getSize()).getX() >= 800.0f)
		{
			if (curSpeed.getX() > 0.0f)
			{
				curSpeed.reverse();
			}
		}
	}
	else
	{
		if (gPos.getX() < 0.0f)
		{
			if (curSpeed.getX() < 0.0f)
			{
				curSpeed.reverse();
			}
		}
		else
		{
			if (gPos.getX() > 800.0f)
			{
				if (curSpeed.getX() > 0.0f)
				{
					curSpeed.reverse();
				}
			}
		}
	}

	gPos += curSpeed;
}

void World::draw(Graphics2D* g2d)
{
	// instructions
	if (score == 0)
	{
		g2d->drawCenteredText("Use left and right arrow keys to move", 400, 400, 0x888888, 3);
		g2d->drawCenteredText("Press space to fire", 400, 430, 0x999999, 4);
	}

	// draw objects
	LinkedListIterator<WorldObject*> it = objects.getIterator();
	while (it.hasNext())
	{
		WorldObject* obj = it.next();
		obj->draw(g2d);
	}

	// draw lives
	for (int i = 0; i < player->getLives(); i++)
	{
		g2d->drawImage(Resources::IMG_PLAYER_SHIP, 10 + (30 * i), 600 - Resources::IMG_PLAYER_SHIP->GetHeight());
	}

	// draw level
	for (int i = 0; i < level; i++)
	{
		g2d->drawImage(Resources::IMG_LEVEL, 800 - (Resources::IMG_LEVEL->GetWidth() + (20 * i)), 600 - Resources::IMG_LEVEL->GetHeight());
	}

	// draw score
	g2d->drawText((char*)(std::string("Score: ").append(std::to_string(score)).c_str()), 10, 10, 0xFFFFFF, 3);
	g2d->drawRightAlignedText((char*)(std::string("HighScore: ").append(std::to_string(instanceGameHandler->getHighScore())).c_str()), 795, 10, 0xFFFFFF, 3);
}
