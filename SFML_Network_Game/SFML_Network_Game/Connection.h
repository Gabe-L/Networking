#pragma once
#include <SFML/Network.hpp>
#include "protocol.h"

class Connection
{
public:
	Connection(sf::UdpSocket sock);
	~Connection();

private:
	// The connected socket
	sf::UdpSocket sock_;

	// Data read from the connection
	char readBuffer_[sizeof(NetMessage)];
	int readCount_;

	// Data to send to the connection
	char writeBuffer_[sizeof(NetMessage)];
	int writeCount_;

};

