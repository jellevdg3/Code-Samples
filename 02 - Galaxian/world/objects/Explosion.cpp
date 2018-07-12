#include "Explosion.h"

#include "game/Resources.h"
#include "game/world/World.h"

Explosion::Explosion(Vector2& pos)
{
	bounds.getPos().set(pos);

	img = Resources::IMG_EXPLOSION;
	imageWidth = img->GetWidth();
	imageHeight = img->GetHeight();
	spriteWidth = imageWidth / 5;
	spriteHeight = imageHeight / 5;

	bounds.getPos() -= (Vector2((float)(spriteWidth), (float)(spriteHeight)) / 2.0f);

	curX = 0;
	curY = 0;
	maxX = 5;
	maxY = 5;
}

Explosion::~Explosion()
{
}

void Explosion::tick(float delta)
{
	curX++;
	if (curX >= maxX)
	{
		if (curY >= maxY - 1)
		{
			curX = 0;
			curY = 0;

			getWorld()->remove(this);
			delete this;
		}
		else
		{
			curX = 0;
			curY++;
		}
	}
}

void Explosion::draw(Graphics2D* g2d)
{
	g2d->drawImageClipped(img, (int)(bounds.getLeft()), (int)(bounds.getTop()), curX * spriteWidth, curY * spriteHeight, spriteWidth, spriteHeight);
}
