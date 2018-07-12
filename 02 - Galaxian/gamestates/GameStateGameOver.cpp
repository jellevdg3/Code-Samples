#include "GameStateGameOver.h"

#include <string>

#include "GameStateStartMenu.h"
#include "GameStateInGame.h"
#include "game/Resources.h"

GameStateGameOver::GameStateGameOver(int score)
{
	this->score = score;
}

GameStateGameOver::~GameStateGameOver()
{
}

void GameStateGameOver::tick(float delta)
{
	if (instanceGameHandler->isKeyPressed(SCANCODE_SPACE))
	{
		instanceGameHandler->gotoGameState(new GameStateInGame());
	}
}

void GameStateGameOver::draw(Graphics2D* g2d)
{
	g2d->drawImage(Resources::IMG_GAMEOVER_LOGO, 50, 50);

	g2d->drawCenteredText((char*)(std::string("Score: ").append(std::to_string(score)).c_str()), 400, 300, 0xFFFFFF, 6);
	g2d->drawCenteredText((char*)(std::string("HighScore: ").append(std::to_string(instanceGameHandler->getHighScore())).c_str()), 400, 340, 0xFFFFFF, 6);

	g2d->drawCenteredText("Press space to retry", 400, 500, 0xFFFF00, 3);
	g2d->drawCenteredText("Press backspace to go back to menu", 400, 520, 0x0000FF, 3);
	g2d->drawCenteredText("Press escape to exit", 400, 540, 0xFF0000, 3);
}
