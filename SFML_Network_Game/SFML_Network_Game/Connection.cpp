#include "Connection.h"



Connection::Connection(sf::UdpSocket sock)// : sock_(sock), readCount_(0), writeCount_(0)
{
	printf("New connection\n");
}


Connection::~Connection()
{
}

