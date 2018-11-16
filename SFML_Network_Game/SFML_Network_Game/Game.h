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
	sf::Vector2f checkLineCollision(sf::Vector2f originOne, sf::Vector2f pointOne, sf::Vector2f originTwo, sf::Vector2f pointTwo);
	sf::Vector2f HitScan(sf::Vector2f lineOrigin, sf::Vector2f lineEnd, sf::Sprite sprite);
	float Distance(sf::Vector2f pointOne, sf::Vector2f pointTwo);

	void beginDraw();
	void endDraw();
	void renderUI();
	void reset();

	// #TODO: Clear test vars
	Player test;
	sf::Texture tst;
	sf::Sprite collider;

	sf::VertexArray hitScanLine;
	sf::Color lineColour;
	
	sf::Vector2f MousePos;

};

