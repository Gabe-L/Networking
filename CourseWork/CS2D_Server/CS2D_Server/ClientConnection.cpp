#include "pch.h"
#include "ClientConnection.h"


ClientConnection::ClientConnection() : active(true)
{
}


ClientConnection::~ClientConnection()
{
}

bool ClientConnection::takeDamage()
{
	health -= 10.0f;
	if (health <= 0.0f) {
		health = 100.0f;
		score--;
		return true;
	}
	return false;
}