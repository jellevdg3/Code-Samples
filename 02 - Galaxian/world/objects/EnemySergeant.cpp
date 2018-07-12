#include "EnemySergeant.h"

#include "game/Resources.h"
#include "game/world/World.h"

EnemySergeant::EnemySergeant(Vector2* gPos, Vector2& lPos) :
Enemy(gPos, Resources::IMG_ENEMY_02A, Resources::IMG_ENEMY_02B)
{
	bounds.getPos() = lPos;

	bounds.getSize() = Vector2((float)(img->GetWidth()), (float)(img->GetHeight()));

	shootingChance = 25;
}

EnemySergeant::~EnemySergeant()
{

}

void EnemySergeant::onDestroyed(Bullet* bullet)
{
	Enemy::onDestroyed(bullet);
}

void EnemySergeant::onCollision(Bullet* bullet)
{
	Enemy::onCollision(bullet);
	getWorld()->remove(this);
	getWorld()->addScore(80);
	delete this;
}

void EnemySergeant::onCollision(Player* player)
{
	getWorld()->remove(this);
	Enemy::onCollision(player);
	delete this;
}

void EnemySergeant::tick(float delta)
{
	Enemy::tick(delta);
}

void EnemySergeant::draw(Graphics2D* g2d)
{
	Enemy::draw(g2d);
}
