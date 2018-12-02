#pragma once
#include <vector>
#include "ClientConnection.h"
#include "protocol.h"

class Simulation
{
public:
	Simulation();
	~Simulation();

	bool update(float deltaTime);
	sf::Vector2f HitScan(sf::Vector2f lineOrigin, sf::Vector2f lineEnd, ClientConnection* otherPlayer);
	sf::Vector2f checkLineCollision(sf::Vector2f originOne, sf::Vector2f pointOne, sf::Vector2f originTwo, sf::Vector2f pointTwo);
	float Distance(sf::Vector2f pointOne, sf::Vector2f pointTwo);

private:

	float serverTime;
	float tick;
	int tickCount = 0;
	std::vector<ClientConnection*> players;
	sf::Texture collisionTexture;

	// Server network info
	sf::UdpSocket sock;
	sf::IpAddress serverIp;
	unsigned short serverPort;

};

