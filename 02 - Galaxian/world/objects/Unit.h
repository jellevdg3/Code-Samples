#pragma once

#include "WorldObject.h"
#include "game/util/linkedlist/LinkedList.h"

class Bullet;

class Unit : public WorldObject
{
public:
	Unit();
	virtual ~Unit();

	virtual bool isClipped(){ return true; };

	virtual void onDestroyed(Bullet* bullet);
	virtual void onCollision(Bullet* bullet);

protected:
	void spawn(Bullet* bullet);
	void remove(Bullet* bullet);

	LinkedList<Bullet*> bullets;
};
