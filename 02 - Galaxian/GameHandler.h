#pragma once

class GameHandler;
class GameState;
class Graphics2D;
class StarField;

namespace Tmpl8
{
	class Surface;
}

// ---------- KEY NOTES (SDL_SCANCODE) ---------- //
#define SCANCODE_LEFT_ARROW 80
#define SCANCODE_RIGHT_ARROW 79
#define SCANCODE_SPACE 44
#define SCANCODE_BACKSPACE 42

// ----------  ---------- //


extern GameHandler* instanceGameHandler;

class Database;

class GameHandler
{
public:
	GameHandler(Tmpl8::Surface* screen);
	~GameHandler();

	// --- events --- //
	void onKeyUp(int keyCode);
	void onKeyDown(int keyCode);
	// -- //

	bool isKeyDown(int keyCode);
	bool isKeyPressed(int keyCode);

	void start();
	void gotoGameState(GameState* newGameState);
	GameState* getGameState();

	int getHighScore();
	void setHighScore(int highScore);

	void tick(float delta);

private:
	GameState* gameState;
	GameState* _gotoGameState;

	Graphics2D* g2d;
	bool* buttons;
	bool* lastButtons;
	StarField* starField;

	Database* database;
	int highScore;

	void setGameState(GameState* newGameState);
};
