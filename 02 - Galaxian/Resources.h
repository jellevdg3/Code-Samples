#pragma once

#include "template/surface.h"
#include "util/Graphics2D.h"

namespace Resources
{
	// --- PRE DEFINE RESOURCES --- //
	extern Image* IMG_MENU_LOGO;

	extern Image* IMG_LEVEL;

	extern Image* IMG_EXPLOSION;

	extern Image* IMG_PLAYER_SHIP;
	extern Image* IMG_PLAYER_BULLET;

	extern Image* IMG_ENEMY_BULLET;
	extern Image* IMG_ENEMY_00A;
	extern Image* IMG_ENEMY_01A;
	extern Image* IMG_ENEMY_01B;
	extern Image* IMG_ENEMY_02A;
	extern Image* IMG_ENEMY_02B;
	extern Image* IMG_ENEMY_03A;
	extern Image* IMG_ENEMY_03B;

	extern Image* IMG_GAMEOVER_LOGO;
	// ---  --- //

	void load();
	void clean();
	void scale(int scale);
}
