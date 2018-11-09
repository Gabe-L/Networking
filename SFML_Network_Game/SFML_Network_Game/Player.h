#pragma once
#include "SFML/Graphics.hpp"
class Player : public sf::Sprite
{
public:
	Player();
	~Player();

	void update(float dt);
	void setVelocity(sf::Vector2f vel);

protected:
	sf::Vector2f velocity;

};

