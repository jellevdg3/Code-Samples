#pragma once

#include "game/util/Vector2.h"
#include "game/util/Rectangle2.h"
#include "game/util/linkedlist/LinkedList.h"

class Graphics2D;

class WorldObject;
class Player;
class Enemy;
class EnemyLieutenant;
class Bullet;

class World
{
public:
	World();
	~World();

	void spawn(WorldObject* obj);
	void spawn(Bullet* obj);
	void spawn(Enemy* obj);
	void spawn(EnemyLieutenant* obj);
	void spawn(Player* obj);

	void remove(WorldObject* obj);
	void remove(Bullet* obj);
	void remove(Enemy* obj);
	void remove(EnemyLieutenant* obj);
	void remove(Player* obj);

	LinkedList<Enemy*>* getEnemies();
	LinkedList<EnemyLieutenant*>* getEnemyLieutenants();
	LinkedList<Bullet*>* getBullets();
	LinkedList<Player*>* getPlayers();
	Player* getPlayer();

	void addScore(int score);
	int getScore();

	void calcBounds();

	void tick(float delta);
	void draw(Graphics2D* g2d);

private:
	int score;
	Vector2 gPos;
	Vector2 curSpeed;
	Rectangle2 bounds;

	LinkedList<WorldObject*> objects;
	LinkedList<Bullet*> bullets;
	LinkedList<Enemy*> enemies;
	LinkedList<EnemyLieutenant*> enemyLieutenants;
	LinkedList<Player*> players;
	Player* player;

	int spawnTime;
	int level;
	int inBoundsSize;

	void spawn();
};
