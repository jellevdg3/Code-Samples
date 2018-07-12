#include "Resources.h"

#include "util/linkedlist/LinkedList.h"

namespace Resources
{
	// --- DEFINE RESOURCES --- //
	Image* IMG_MENU_LOGO;

	Image* IMG_LEVEL;

	Image* IMG_EXPLOSION;

	Image* IMG_PLAYER_SHIP;
	Image* IMG_PLAYER_BULLET;

	Image* IMG_ENEMY_BULLET;
	Image* IMG_ENEMY_00A;
	Image* IMG_ENEMY_01A;
	Image* IMG_ENEMY_01B;
	Image* IMG_ENEMY_02A;
	Image* IMG_ENEMY_02B;
	Image* IMG_ENEMY_03A;
	Image* IMG_ENEMY_03B;

	Image* IMG_GAMEOVER_LOGO;
	// --- --- //

	void loadImage(char* name, Image** ptr);
	void loadImage(char* name, Image** ptr, bool scale);

	void load()
	{
		// --- LOAD RESOURCES --- //
		loadImage("Assets/Galaxian.png", &IMG_MENU_LOGO);

		loadImage("Assets/Level.png", &IMG_LEVEL, true);

		loadImage("Assets/Explosion.png", &IMG_EXPLOSION, false);

		loadImage("Assets/PlayerShip.png", &IMG_PLAYER_SHIP, true);
		loadImage("Assets/PlayerBullet.png", &IMG_PLAYER_BULLET, true);

		loadImage("Assets/EnemyBullet.png", &IMG_ENEMY_BULLET, true);
		loadImage("Assets/Commander.png", &IMG_ENEMY_00A, true);
		loadImage("Assets/Galaxian01a.png", &IMG_ENEMY_01A, true);
		loadImage("Assets/Galaxian01b.png", &IMG_ENEMY_01B, true);
		loadImage("Assets/Galaxian02a.png", &IMG_ENEMY_02A, true);
		loadImage("Assets/Galaxian02b.png", &IMG_ENEMY_02B, true);
		loadImage("Assets/Galaxian03a.png", &IMG_ENEMY_03A, true);
		loadImage("Assets/Galaxian03b.png", &IMG_ENEMY_03B, true);

		loadImage("Assets/gameover.png", &IMG_GAMEOVER_LOGO);
		// --- --- //
	}

	Image* scaleImage(Image* surface, int scale);
	LinkedList<Image**>* images = new LinkedList<Image**>();
	LinkedList<Image**>* scaleImages = new LinkedList<Image**>();

	void scale(int scale)
	{
		LinkedListIterator<Image**> it = scaleImages->getIterator();
		while (it.hasNext())
		{
			Image** img = it.next();
			*img = scaleImage(*img, scale);
		}
	}

	void clean()
	{
		LinkedListIterator<Image**> it = images->getIterator();
		while (it.hasNext())
		{
			delete *it.next();
		}
		delete images;
		delete scaleImages;
	}

	void loadImage(char* name, Image** ptr)
	{
		loadImage(name, ptr, false);
	}

	void loadImage(char* name, Image** ptr, bool scale)
	{
		Image* img = new Image(name);
		*ptr = img;
		images->add(ptr);

		if (scale)
		{
			scaleImages->add(ptr);
		}
	}

	Image* scaleImage(Image* surface, int scale)
	{
		// Create the new surface with the new scaled dimensions
		Tmpl8::Surface* tmp = new Tmpl8::Surface(surface->GetWidth() * scale, surface->GetHeight() * scale);
		tmp->Clear(0);

		// Get required data
		Tmpl8::Pixel* srcBuffer = surface->GetBuffer();
		Tmpl8::Pixel* dstBuffer = tmp->GetBuffer();
		int w = surface->GetWidth();
		int h = surface->GetHeight();

		// Scan through pixels, and copy the pixels of the source to the new buffer with the new scale
		for (int x = 0; x < w; x++)
		{
			for (int y = 0; y < h; y++)
			{
				for (int _x = 0; _x < scale; _x++)
				{
					for (int _y = 0; _y < scale; _y++)
					{
						int px = (x * scale) + _x;
						int py = (y * scale) + _y;

						if (srcBuffer[x + (y*w)] != 0)
						{
							dstBuffer[px + (py*w*scale)] = srcBuffer[x + (y*w)];
						}
					}
				}
			}
		}

		// Finalize
		delete surface;
		surface = tmp;

		return surface;
	}
}
