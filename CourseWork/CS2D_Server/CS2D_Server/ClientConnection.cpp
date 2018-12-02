#include "pch.h"
#include "ClientConnection.h"


ClientConnection::ClientConnection()
{
}


ClientConnection::~ClientConnection()
{
}

bool ClientConnection::takeDamage()
{
	health -= 10.0f;
	if (health <= 0.0f) {
		return true;
	}
	return false;
}