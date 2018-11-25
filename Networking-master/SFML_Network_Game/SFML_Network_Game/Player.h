#pragma once
#include "SFML/Graphics.hpp"
class Player : public sf::Sprite
{
public:
	Player();
	~Player();

	void update(float dt);
	void setVelocity(sf::Vector2f vel);
	void OriginToCentre();

	sf::Vector2f getVelocity();

protected:
	sf::Vector2f velocity;

};

