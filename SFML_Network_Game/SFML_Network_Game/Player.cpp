#include "Player.h"



Player::Player()
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
