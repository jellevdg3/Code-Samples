#include "GameHandler.h"

#include <stdlib.h>
#include <time.h>

#include "template/surface.h"
#include "Resources.h"
#include "gamestates/GameStateStartMenu.h"
#include "util/Graphics2D.h"
#include "world/objects/StarField.h"
#include "util/Database.h"

GameHandler* instanceGameHandler = 0;

GameHandler::GameHandler(Tmpl8::Surface* screen)
{
	// Singleton
	if (instanceGameHandler != 0)
	{
		return;
	}
	instanceGameHandler = this;

	srand((unsigned int)(time(NULL)));

	database = new Database("Assets/Save.dat");
	highScore = database->readInt();
	database->writeInt(highScore);

	buttons = new bool[256];
	for (int i = 0; i < 256; i++)
	{
		buttons[i] = false;
	}
	lastButtons = new bool[256];
	for (int i = 0; i < 256; i++)
	{
		lastButtons[i] = false;
	}

	gameState = 0;
	_gotoGameState = 0;

	// Create the Graphics
	g2d = new Graphics2D(screen);

	// Start loading the resources
	Resources::load();

	// Scale the images so we can actually see them on the screen
	Resources::scale(3);

	// Start the game
	this->start();
}

GameHandler::~GameHandler()
{
	if (buttons) delete[] buttons; buttons = 0;
	if (lastButtons) delete[] lastButtons; lastButtons = 0;
	if (gameState) delete gameState; gameState = 0;
	Resources::clean();
}

void GameHandler::start()
{
	starField = new StarField();

	gotoGameState(new GameStateStartMenu());
}

void GameHandler::gotoGameState(GameState* newGameState)
{
	_gotoGameState = newGameState;
}

void GameHandler::setGameState(GameState* newGameState)
{
	if (gameState)
	{
		delete gameState;
	}

	gameState = newGameState;
}

GameState* GameHandler::getGameState()
{
	return gameState;
}

void GameHandler::tick(float delta)
{
	g2d->clear();

	if (instanceGameHandler->isKeyPressed(SCANCODE_BACKSPACE))
	{
		instanceGameHandler->gotoGameState(new GameStateStartMenu());
	}

	if (_gotoGameState != 0)
	{
		setGameState(_gotoGameState);

		_gotoGameState = 0;
	}

	starField->tick(delta);
	starField->draw(g2d);

	g2d->drawText("Made by Jelle van der Gulik", 400, 588, 0x0000FF, 2);

	gameState->tick(delta);
	gameState->draw(g2d);
}

int GameHandler::getHighScore()
{
	return highScore;
}

void GameHandler::setHighScore(int highScore)
{
	this->highScore = highScore;
	database->writeInt(highScore);
}

void GameHandler::onKeyUp(int keyCode)
{
	if (keyCode >= 0 && keyCode < 256)
	{
		buttons[keyCode] = false;
		lastButtons[keyCode] = false;
	}
}

void GameHandler::onKeyDown(int keyCode)
{
	if (keyCode >= 0 && keyCode < 256)
	{
		buttons[keyCode] = true;
	}
}

bool GameHandler::isKeyDown(int keyCode)
{
	if (keyCode >= 0 && keyCode < 256)
	{
		return buttons[keyCode];
	}

	return false;
}

bool GameHandler::isKeyPressed(int keyCode)
{
	if (keyCode >= 0 && keyCode < 256)
	{
		bool lastKeyButton = lastButtons[keyCode];
		lastButtons[keyCode] = buttons[keyCode];
		return !lastKeyButton && buttons[keyCode];
	}

	return false;
}
