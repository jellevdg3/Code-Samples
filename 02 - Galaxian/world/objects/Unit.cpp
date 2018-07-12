#include "Unit.h"

#include "Bullet.h"

Unit::Unit()
{

}

Unit::~Unit()
{
	LinkedListIterator<Bullet*> it = bullets.getIterator();
	while (it.hasNext())
	{
		it.next()->removeOwner();
	}
}

void Unit::onDestroyed(Bullet* bullet)
{
	remove(bullet);
}

void Unit::onCollision(Bullet* bullet)
{
}

void Unit::spawn(Bullet* bullet)
{
	bullets.add(bullet);
}

void Unit::remove(Bullet* bullet)
{
	bullets.remove(bullet);
}
