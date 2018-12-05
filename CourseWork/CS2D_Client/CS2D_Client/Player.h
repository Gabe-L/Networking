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
	void LerpPosition(float time);
	void PredictPosition(float time, bool linear);

	sf::Vector2f getVelocity();

	float timeSinceLastMessage;
	int playerID;
	std::vector<PlayerInfo> messageHistory;
	bool hitEnemy = false;

protected:
	sf::Vector2f velocity;
	sf::Vector2f displacement;
};