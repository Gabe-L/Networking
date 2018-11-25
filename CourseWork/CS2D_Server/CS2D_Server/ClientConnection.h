#pragma once
#include "pch.h"

class ClientConnection
{
public:
	ClientConnection();
	~ClientConnection();

	int identity;
	sf::Vector2f position;

	// Network info
	sf::IpAddress address;
	unsigned short port;

};