#pragma once

#include "GameState.h"

class GameStateGameOver : public GameState
{
public:
	GameStateGameOver(int score);
	~GameStateGameOver();	

	void tick(float delta);
	void draw(Graphics2D* g2d);

private:
	int score;
};
