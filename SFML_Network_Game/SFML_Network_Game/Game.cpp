#include "Game.h"

Game::Game(sf::RenderWindow* hwnd)
{
	window = hwnd;

	// Setup player
	tst.loadFromFile("ready.png");
	test.setTexture(tst);
	test.setScale(sf::Vector2f(0.1f, 0.1f));

}
Game::~Game()
{
}
void Game::update(float deltaTime)
{
	test.update(deltaTime);
}
void Game::handleInput(Input* input)
{
	if (input->isKeyDown(sf::Keyboard::W)) {
		test.setVelocity({ 0,-100 });
	}
	else if (input->isKeyDown(sf::Keyboard::S)) {
		test.setVelocity({ 0,100 });
	}
	else if (input->isKeyDown(sf::Keyboard::A)) {
		test.setVelocity({ -100,0 });
	}
	else if (input->isKeyDown(sf::Keyboard::D)) {
		test.setVelocity({ 100,0});
	}
}
void Game::render()
{
	beginDraw();

	window->draw(test);

	renderUI();
	endDraw();
}

bool Game::checkCollision(sf::Sprite* s1, sf::Sprite* s2)
{
	if (s1->getLocalBounds().left + s1->getLocalBounds().width < s2->getLocalBounds().left)
		return false;
	if (s1->getLocalBounds().left > s2->getLocalBounds().left + s2->getLocalBounds().width)
		return false;
	if (s1->getLocalBounds().top + s1->getLocalBounds().height < s2->getLocalBounds().top)
		return false;
	if (s1->getLocalBounds().top > s2->getLocalBounds().top + s2->getLocalBounds().height)
		return false;
	return true;
}

void Game::beginDraw()
{
	window->clear(sf::Color::Black);
}
void Game::endDraw()
{
	window->display();
}

void Game::renderUI() {

}

void Game::reset() {

}