#include "Bullet.h"

#include "game/Resources.h"
#include "Unit.h"
#include "game/world/World.h"

Bullet::Bullet(Image* img, Unit* owner, LinkedList<Unit*>* targets, Vector2& pos, Vector2& speed)
{
	this->img = img;
	this->owner = owner;
	this->targets = targets;
	this->speed = speed;

	this->bounds = Rectangle2(pos, Vector2((float)(img->GetWidth()), (float)(img->GetHeight())));
}

Bullet::~Bullet()
{

}

void Bullet::removeOwner()
{
	owner = 0;
}

void Bullet::tick(float delta)
{
	// move up
	bounds.getPos() += speed;

	LinkedListIterator<Unit*> it = targets->getIterator();
	while (it.hasNext())
	{
		Unit* obj = it.next();
		if (obj->isClipped() && bounds.collidesWith(obj->getBounds()))
		{
			obj->onCollision(this);
			if (owner != 0)
			{
				owner->onDestroyed(this);
			}
			getWorld()->remove(this);
			delete this;
			return;
		}
	}

	if (bounds.getPos().bounds(0, 0, 800, 600))
	{
		if (owner != 0)
		{
			owner->onDestroyed(this);
		}
		getWorld()->remove(this);
		delete this;
		return;
	}
}

void Bullet::draw(Graphics2D* g2d)
{
	g2d->drawImage(img, (int)(bounds.getLeft()), (int)(bounds.getTop()));
}
