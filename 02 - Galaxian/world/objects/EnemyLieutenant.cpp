#include "EnemyLieutenant.h"

#include "game/Resources.h"
#include "game/world/World.h"

EnemyLieutenant::EnemyLieutenant(Vector2* gPos, Vector2& lPos) :
Enemy(gPos, Resources::IMG_ENEMY_01A, Resources::IMG_ENEMY_01B)
{
	bounds.getPos() = lPos;

	bounds.getSize() = Vector2((float)(img->GetWidth()), (float)(img->GetHeight()));
}

EnemyLieutenant::~EnemyLieutenant()
{

}

void EnemyLieutenant::onDestroyed(Bullet* bullet)
{
	Enemy::onDestroyed(bullet);
}

void EnemyLieutenant::onCollision(Bullet* bullet)
{
	Enemy::onCollision(bullet);
	getWorld()->remove(this);
	getWorld()->addScore(100);
	delete this;
}

void EnemyLieutenant::onCollision(Player* player)
{
	getWorld()->remove(this);
	Enemy::onCollision(player);
	delete this;
}

void EnemyLieutenant::tick(float delta)
{
	Enemy::tick(delta);
}

void EnemyLieutenant::draw(Graphics2D* g2d)
{
	Enemy::draw(g2d);
}
