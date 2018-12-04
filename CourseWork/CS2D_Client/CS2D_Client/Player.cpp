#include "pch.h"

#include "Player.h"



Player::Player() : playerID(-1), timeSinceLastMessage(0.0f)
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

void Player::PredictPosition(float time, bool linear)
{
	const int msize = messageHistory.size();
	if (msize < 3) {
		return;
	}

	const PlayerInfo& msg0 = messageHistory[msize - 1];
	const PlayerInfo& msg1 = messageHistory[msize - 2];
	const PlayerInfo& msg2 = messageHistory[msize - 3];

	if (linear) {
		// Linear

		float timepassed = msg0.time - msg1.time;
		if (timepassed == 0.0f) { return; }


		float x_speed = (msg0.positionX - msg1.positionX) / timepassed;
		float y_speed = (msg0.positionY - msg1.positionY) / timepassed;

		timepassed = time - msg0.time;
		if (timepassed == 0.0f) { return; }

		setPosition(sf::Vector2f(msg0.positionX + (x_speed * timepassed), msg0.positionY + (y_speed * timepassed)));

		return;
	}

	// Quadratic
	float x_acc, y_acc;
	float x_v = 0.f, y_v = 0.f;
	float x_u = 0.f, y_u = 0.f;

	float finalTime = msg0.time - msg1.time;

	if (finalTime != 0.0f) {
		x_v = (msg0.positionX - msg1.positionX) / finalTime;
		y_v = (msg0.positionY - msg1.positionY) / finalTime;
	}
	else {
		return;
	}

	float initialTime = msg1.time - msg2.time;

	if (initialTime != 0.0f) {
		x_u = (msg1.positionX - msg2.positionX) / initialTime;
		y_u = (msg1.positionY - msg2.positionY) / initialTime;
	}
	else {
		return;
	}


	x_acc = x_v - x_u;
	y_acc = y_v - y_u;

	float overallTime = msg0.time - msg2.time;

	if (overallTime != 0.0f) {
		x_acc /= overallTime;
		y_acc /= overallTime;
	}
	else {
		return;
	}


	// Displacement
	// s = ut + 0.5 * at ^2

	float currentTime = time - msg0.time;

	displacement.x = 0.0f;
	displacement.x += x_u * currentTime;
	displacement.x += 0.5 * (x_acc * pow(currentTime, 2));

	displacement.y = 0.0f;
	displacement.y += y_u * currentTime;
	displacement.y += 0.5 * (y_acc * pow(currentTime, 2));

	setPosition(sf::Vector2f(msg0.positionX + (displacement.x), msg0.positionY + (displacement.y)));
	return;
}

sf::Vector2f Player::getVelocity()
{
	return velocity;
}
