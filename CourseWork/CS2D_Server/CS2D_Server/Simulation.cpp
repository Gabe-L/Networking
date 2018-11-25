#include "pch.h"
#include "Simulation.h"


Simulation::Simulation() : serverTime(0.0f)
{
	serverIp = "127.0.0.1";
	serverPort = 255;

	if (sock.bind(serverPort, serverIp) != sf::Socket::Done) {
		std::printf("Server socket bind failed\n");
	}

	sock.setBlocking(false);

}


Simulation::~Simulation()
{
}

bool Simulation::update(float deltaTime)
{
	serverTime += deltaTime;

	// Update server 64 times per second
	if (serverTime >= 1 / 64.f) {

		sf::Packet playersInfo;

		PlayerCount playerCount;
		playerCount.playerCount = players.size();

		playersInfo << playerCount.messageType << playerCount.playerCount;

		for (auto player : players) {
			PlayerInfo playerInfo;
			playerInfo.playerID = player->identity;
			playerInfo.positionX = player->position.x;
			playerInfo.positionY = player->position.x;

			playersInfo << playerInfo.playerID << playerInfo.positionX << playerInfo.positionX;

		}

		for (auto player : players) {
			if (sock.send(playersInfo, player->address, player->port) != sf::Socket::Done) {
				printf("Positions send failed\n");
			}
		}

	}



	// Receive incoming messages
	sf::Packet recvPack;
	sf::IpAddress recvAddr;
	unsigned short recvPort;
	SetUpMessage setUpMessage;
	NetMessage clientInfo;
	sf::Packet replyPack;

	if (sock.receive(recvPack, recvAddr, recvPort) == sf::Socket::Done) {
		int messageType;
		recvPack >> messageType;

		switch (messageType)
		{

		case MessageType::Position:

			recvPack
				>> clientInfo.enemyID
				>> clientInfo.positionX
				>> clientInfo.positionY
				>> clientInfo.mousePosX
				>> clientInfo.mousePosY
				>> clientInfo.time;

			for (auto player : players) {
				if (player->identity == clientInfo.enemyID) {
					player->position = sf::Vector2f(clientInfo.positionX, clientInfo.positionX);
					break;
				}
			}
			break;

		case MessageType::PingRequest:
			setUpMessage.pingType = messageType;
			
			recvPack
				>> setUpMessage.time
				>> setUpMessage.totalTime
				>> setUpMessage.playerIdentity;

			replyPack
				<< MessageType::PingInfo
				<< setUpMessage.totalTime
				<< serverTime;

			if (setUpMessage.playerIdentity >= 0) {
				for (auto player : players) {
					if (player->identity == setUpMessage.playerIdentity) {
						if (sock.send(replyPack, player->address, player->port) != sf::Socket::Done) {
							printf("Ping reply failed\n");
						}
						break;
					}
				}
			}

			ClientConnection* newPlayer = new ClientConnection();

			newPlayer->address = recvAddr;
			newPlayer->port = recvPort;
			newPlayer->identity = players.size();

			players.push_back(newPlayer);

			replyPack
				<< newPlayer->identity;

			if (sock.send(replyPack, newPlayer->address, newPlayer->port) != sf::Socket::Done) {
				printf("Ping reply failed for new player\n");
			}
			else {
				printf("New player accepted, identity: %i\n", newPlayer->identity);
			}

			break;
		}

	}

	return true;
}
