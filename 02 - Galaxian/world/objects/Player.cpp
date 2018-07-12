#include "Player.h"

#include <stdlib.h>

#include "game/GameHandler.h"
#include "game/Resources.h"
#include "Bullet.h"
#include "Enemy.h"
#include "game/gamestates/GameStateInGame.h"
#include "game/gamestates/GameStateGameOver.h"
#include "game/world/World.h"
#include "Explosion.h"

Player::Player()
{
	img = Resources::IMG_PLAYER_SHIP;

	bounds = Rectangle2(400.0f, 600.0f, (float)(img->GetWidth()), (float)(img->GetHeight()));
	bounds.getPos() -= bounds.getSize() / 2.0f;
	bounds.getPos().addY(-bounds.getSize().getY());

	canSpawnBullet = true;
	lives = 2;
	noClipTime = 0;
}

Player::~Player()
{

}

void Player::onDestroyed(Bullet* bullet)
{
	canSpawnBullet = true;
	Unit::onDestroyed(bullet);
}

void Player::onCollision(Bullet* bullet)
{
	if (noClipTime == 0)
	{
		Explosion* explosion = new Explosion(getBounds().getCenter());
		getWorld()->spawn(explosion);

		lives--;
		if (lives == -1)
		{
			noClipTime = 400;
		}
		else
		{
			noClipTime = 300;
		}
	}
}

void Player::onCollision(Enemy* enemy)
{
	if (noClipTime == 0)
	{
		Explosion* explosion = new Explosion(getBounds().getCenter());
		getWorld()->spawn(explosion);

		lives--;
		if (lives == -1)
		{
			noClipTime = 400;
		}
		else
		{
			noClipTime = 300;
		}
	}
}

int Player::getLives()
{
	return lives;
}

bool Player::isClipped()
{
	return noClipTime == 0;
}

void Player::tick(float delta)
{
	if (instanceGameHandler->isKeyDown(SCANCODE_LEFT_ARROW) && noClipTime < 180)
	{
		// move left
		bounds.getPos() += Vector2(-6, 0);
	}

	if (instanceGameHandler->isKeyDown(SCANCODE_RIGHT_ARROW) && noClipTime < 180)
	{
		// move right
		bounds.getPos() += Vector2(6, 0);
	}

	bounds.clamp(0.0f, 0.0f, 800.0f, 600.0f);

	if (instanceGameHandler->isKeyPressed(SCANCODE_SPACE) && canSpawnBullet && noClipTime < 180)
	{
		canSpawnBullet = false;

		// spawn bullet
		World* world = getWorld();

		Bullet* bullet = new Bullet(Resources::IMG_PLAYER_BULLET, this, (LinkedList<Unit*>*)(world->getEnemies()), Vector2((bounds.getCenter().getX()) - (Resources::IMG_PLAYER_BULLET->GetWidth() / 2) - 1, (bounds.getTop()) - Resources::IMG_PLAYER_BULLET->GetHeight()), Vector2(0, -10.0f));
		this->spawn(bullet);
		world->spawn(bullet);
	}

	if (noClipTime > 0)
	{
		noClipTime--;

		// respawn
		if (noClipTime == 200)
		{
			if (lives == -1)
			{
				instanceGameHandler->gotoGameState(new GameStateGameOver(getWorld()->getScore()));
				return;
			}

			bounds = Rectangle2(400.0f, 600.0f, (float)(img->GetWidth()), (float)(img->GetHeight()));
			bounds.getPos() -= bounds.getSize() / 2.0f;
			bounds.getPos().addY(-bounds.getSize().getY());
		}

		if (lives == -1)
		{
			for (int i = 0; i < 3; i++)
			{
				int randomX = (rand() % 200) - 100;
				int randomY = (rand() % 200) - 100;

				Explosion* explosion = new Explosion(getBounds().getCenter() + Vector2((float)(randomX), (float)(randomY)));
				getWorld()->spawn(explosion);
			}
		}
	}
}

void Player::draw(Graphics2D* g2d)
{
	if (noClipTime == 0 || (noClipTime % 30 > 12 && noClipTime < 180))
	{
		g2d->drawImage(img, (int)(bounds.getLeft()), (int)(bounds.getTop()));

		if (canSpawnBullet)
		{
			g2d->drawImage(Resources::IMG_PLAYER_BULLET, (int)(bounds.getCenter().getX()) - (Resources::IMG_PLAYER_BULLET->GetWidth() / 2) - 1, (int)(bounds.getTop()) - Resources::IMG_PLAYER_BULLET->GetHeight());
		}
	}
}
