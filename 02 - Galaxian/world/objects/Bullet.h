#pragma once

#include "WorldObject.h"
#include "game/util/linkedlist/LinkedList.h"

class Unit;

class Bullet : public WorldObject
{
public:
	Bullet(Image* img, Unit* owner, LinkedList<Unit*>* targets, Vector2& pos, Vector2& speed);
	~Bullet();

	void removeOwner();
	
	void tick(float delta);
	void draw(Graphics2D* g2d);

private:
	Unit* owner;
	LinkedList<Unit*>* targets;
	Vector2 speed;
};
