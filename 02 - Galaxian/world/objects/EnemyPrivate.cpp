#include "EnemyPrivate.h"

#include "game/Resources.h"
#include "game/world/World.h"

EnemyPrivate::EnemyPrivate(Vector2* gPos, Vector2& lPos) :
Enemy(gPos, Resources::IMG_ENEMY_03A, Resources::IMG_ENEMY_03B)
{
	bounds.getPos() = lPos;

	bounds.getSize() = Vector2((float)(img->GetWidth()), (float)(img->GetHeight()));
}

EnemyPrivate::~EnemyPrivate()
{

}

void EnemyPrivate::onDestroyed(Bullet* bullet)
{
	Enemy::onDestroyed(bullet);
}

void EnemyPrivate::onCollision(Bullet* bullet)
{
	Enemy::onCollision(bullet);
	getWorld()->remove(this);
	getWorld()->addScore(60);
	delete this;
}

void EnemyPrivate::onCollision(Player* player)
{
	getWorld()->remove(this);
	Enemy::onCollision(player);
	delete this;
}

void EnemyPrivate::tick(float delta)
{
	Enemy::tick(delta);
}

void EnemyPrivate::draw(Graphics2D* g2d)
{
	Enemy::draw(g2d);
}
