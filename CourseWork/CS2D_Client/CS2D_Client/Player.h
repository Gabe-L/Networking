#pragma once
#include "pch.h"

class Player : public sf::Sprite
{
public:
	Player();
	~Player();

	void update(float dt);
	void setVelocity(sf::Vector2f vel);
	void OriginToCentre();

	sf::Vector2f getVelocity();

	int playerID;

protected:
	sf::Vector2f velocity;

};