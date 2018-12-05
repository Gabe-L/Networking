#pragma once
#include "pch.h"

class ClientConnection
{
public:
	ClientConnection();
	~ClientConnection();

	bool takeDamage();

	int identity;
	sf::Sprite collisionSprite;
	sf::Vector2f position;
	sf::Vector2f mousePosition;
	float remoteTime = 0.0f;
	float previousMessageTime = -1.0f;
	float shotCoolDown = 0.0f;
	float timeSinceLastMessage = 0.0f;
	float lastRotation = 0.0f;
	int score = 0;

	// Network info
	sf::IpAddress address;
	unsigned short port;
	bool active;

private:
	float health = 100.0f;
};