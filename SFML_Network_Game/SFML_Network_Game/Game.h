#pragma once
#include <SFML/Graphics.hpp>
#include <Windows.h>
#include "Input.h"
#include "Player.h"

using namespace std;

class Game
{
public:
	Game(sf::RenderWindow* hwnd);
	~Game();

	void handleInput(Input* input);
	void update(float deltaTime);
	void render();


private:
	sf::RenderWindow* window;
	bool checkCollision(sf::Sprite* s1, sf::Sprite* s2);
	void beginDraw();
	void endDraw();
	void renderUI();
	void reset();

	// #TODO: Clear test vars
	Player test;
	sf::Texture tst;
};

