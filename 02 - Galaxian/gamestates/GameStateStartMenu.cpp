#include "GameStateStartMenu.h"

#include "GameStateInGame.h"
#include "game/Resources.h"

GameStateStartMenu::GameStateStartMenu()
{
}

GameStateStartMenu::~GameStateStartMenu()
{
}

void GameStateStartMenu::tick(float delta)
{
	if(instanceGameHandler->isKeyPressed(SCANCODE_SPACE))
	{
		instanceGameHandler->gotoGameState(new GameStateInGame());
	}
}

void GameStateStartMenu::draw(Graphics2D* g2d)
{
	g2d->drawImage(Resources::IMG_MENU_LOGO, 100, 10);

	g2d->drawCenteredText("We are the Galaxians", 400, 200, 0xFF0000, 4);
	g2d->drawCenteredText("Mission: Destroy Aliens", 400, 230, 0xFF0000, 4);

	g2d->drawCenteredText("-Score Advance Table-", 400, 300, 0xFFFFFF, 3);

	g2d->drawImage(Resources::IMG_ENEMY_00A, 310, 340);
	g2d->drawText("300 pts", 380, 340 + 10, 0xFFFFFF, 3);

	g2d->drawImage(Resources::IMG_ENEMY_01A, 310 + 3, 390);
	g2d->drawText("100 pts", 380, 380 + 10, 0xFFFFFF, 3);

	g2d->drawImage(Resources::IMG_ENEMY_02A, 310 + 3, 425);
	g2d->drawText("80  pts", 380, 420 + 10, 0xFFFFFF, 3);

	g2d->drawImage(Resources::IMG_ENEMY_03A, 310 + 3, 460);
	g2d->drawText("60  pts", 380, 460 + 10, 0xFFFFFF, 3);

	g2d->drawCenteredText("Press space to start", 400, 550, 0xFFFFFF, 6);
}
