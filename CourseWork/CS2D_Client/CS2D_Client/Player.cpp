#include "pch.h"

#include "Player.h"



Player::Player() : playerID(-1)
{
}


Player::~Player()
{
}

void Player::update(float dt)
{
	move(velocity * dt);
}

void Player::setVelocity(sf::Vector2f vel)
{
	velocity = vel;
}

void Player::OriginToCentre()
{
	int width = getTextureRect().width;
	int height = getTextureRect().height;

	setOrigin(width / 2, height / 2);

}

sf::Vector2f Player::getVelocity()
{
	return velocity;
}
