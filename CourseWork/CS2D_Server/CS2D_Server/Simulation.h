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

private:

	float serverTime;
	std::vector<ClientConnection*> players;

	// Server network info
	sf::UdpSocket sock;
	sf::IpAddress serverIp;
	unsigned short serverPort;

};

