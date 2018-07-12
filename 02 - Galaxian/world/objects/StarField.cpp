#include "StarField.h"

#include <stdlib.h>

#include "game/util/Graphics2D.h"

StarField::StarField()
{
	lists = 30;
	starLists = new SimpleList<Star>*[lists];
	timers = new int[lists];

	int starsInList = 20;

	int diffColors = 10;
	int* colors = new int[diffColors]{0xFF0000, 0x00FF00, 0x0000FF, 0xFFFF00, 0x333333, 0x777777, 0xFFFFFF, 0x555500, 0x00FFFF, 0xFF00FF};

	for (int i = 0; i < lists; i++)
	{
		timers[i] = i;

		starLists[i] = new SimpleList<Star>(starsInList);
		for (int j = 0; j < starsInList; j++)
		{
			Star star;
			star.pos = Vector2((float)(rand() % 799), (float)(rand() % 600));
			star.color = colors[rand() % (diffColors)];
			starLists[i]->add(star);
		}
	}
}

StarField::~StarField()
{

}

void StarField::tick(float delta)
{
	for (int i = 0; i < lists; i++)
	{
		timers[i]++;
		timers[i] %= 30;

		while (starLists[i]->iterate())
		{
			starLists[i]->it->pos.addY(1.0f);

			if (starLists[i]->it->pos.getY() > 599.0f)
			{
				starLists[i]->it->pos.setY(0.0f);
			}
		}
	}
}

void StarField::draw(Graphics2D* g2d)
{
	for (int i = 0; i < lists; i++)
	{
		if (timers[i] > 15)
		{
			while (starLists[i]->iterate())
			{
				g2d->drawPoint(starLists[i]->it->pos.getIX(), starLists[i]->it->pos.getIY(), starLists[i]->it->color);
			}
		}
	}
}
