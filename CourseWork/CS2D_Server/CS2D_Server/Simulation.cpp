#include "pch.h"
#include "Simulation.h"


Simulation::Simulation() : serverTime(0.0f), tick(0.0f)
{
	serverIp = "127.0.0.1";
	serverPort = 255;

	collisionTexture.loadFromFile("gfx/playerSprite.png");

	if (sock.bind(serverPort, serverIp) != sf::Socket::Done) {
		std::printf("Server socket bind failed\n");
	}

	sock.setBlocking(false);

}


Simulation::~Simulation()
{
	TerminateMessage termMsg;
	sf::Packet termPack;
	termPack << termMsg.messageType;

	// Attempt to DC all players before quitting
	for (auto player : players) {
		if (sock.send(termPack, player->address, player->port) != sf::Socket::Done) {
			printf("Termination send failed\n");
		}
	}
}

bool Simulation::update(float deltaTime)
{
	serverTime += deltaTime;
	tick += deltaTime;

	// Update server 64 times per second
	if (tick >= 1 / 64.f)
	{
		tickCount++;

		//std::printf("Time: %.2f\n", serverTime);
		sf::Packet playersInfo;

		PlayerCount playerCount;
		playerCount.playerCount = players.size();

		playersInfo << playerCount.messageType << playerCount.playerCount;

		for (auto player : players) {

			if (tickCount % (64 * 60) == 0) {
				SetUpMessage setupMessage;
				setupMessage.messageType = MessageType::PingInfo;
				setupMessage.playerIdentity = player->identity;
				setupMessage.
			}
			player->shotCoolDown -= tick;

			for (auto enemy : players) {
				if (enemy->identity == player->identity) { continue; }
				if (enemy->mousePosition.x > 0) {
					if (enemy->shotCoolDown <= 0.0f)
					{
						if (HitScan(enemy->position, enemy->mousePosition, player).x > -1.0f) {
							// Enemy has hit player, reduce health
							std::printf("Hit detected.\n");
							enemy->shotCoolDown = 1.0f;
							enemy->mousePosition = sf::Vector2f(-1, -1);
							
						}
					}
				}
			}

			PlayerInfo playerInfo;
			playerInfo.playerID = player->identity;
			playerInfo.positionX = player->position.x;
			playerInfo.positionY = player->position.y;
			playerInfo.time = player->remoteTime;

			playersInfo 
				<< playerInfo.playerID
				<< playerInfo.positionX
				<< playerInfo.positionY
				<< playerInfo.time;

			tick = 0.0f;

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

	if (sock.receive(recvPack, recvAddr, recvPort) == sf::Socket::Done) {
		int messageType;
		recvPack >> messageType;

		switch (messageType)
		{

		case MessageType::Position:
		{
			ClientInfo clientInfo;
			recvPack
				>> clientInfo.enemyID
				>> clientInfo.positionX
				>> clientInfo.positionY
				>> clientInfo.mousePosX
				>> clientInfo.mousePosY
				>> clientInfo.time;

			for (auto player : players) {
				if (player->identity == clientInfo.enemyID) {

					if (clientInfo.time == player->previousMessageTime) {
						break;
					}

					player->position = sf::Vector2f(clientInfo.positionX, clientInfo.positionY);
					player->mousePosition = sf::Vector2f(clientInfo.mousePosX, clientInfo.mousePosY);
					player->remoteTime = clientInfo.time;
					player->previousMessageTime = clientInfo.time;
					break;
				}
			}
			break;
		}
		case MessageType::PingRequest:
		{
			SetUpMessage setUpMessage;
			sf::Packet replyPack;
			setUpMessage.messageType = messageType;

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
			else {
				ClientConnection* newPlayer = new ClientConnection();

				newPlayer->address = recvAddr;
				newPlayer->port = recvPort;
				newPlayer->identity = players.size();
				newPlayer->collisionSprite.setTexture(collisionTexture);
				newPlayer->collisionSprite.setScale(0.5f, 0.5f);

				players.push_back(newPlayer);

				replyPack
					<< newPlayer->identity;

				if (sock.send(replyPack, newPlayer->address, newPlayer->port) != sf::Socket::Done) {
					printf("Ping reply failed for new player\n");
				}
				else {
					printf("New player accepted, identity: %i\n", newPlayer->identity);
				}
			}
		}
			break;
		case MessageType::Terminate:
		{
			for (int i = 0; i < players.size(); i++) {
				if (players.at(i)->address == recvAddr) {
					players.erase(players.begin() + i);
				}
			}
		}
			break;
		}

	}

	return true;
}

sf::Vector2f Simulation::HitScan(sf::Vector2f lineOrigin, sf::Vector2f lineEnd, ClientConnection* otherPlayer)
{
	sf::Vector2f topLeft;
	sf::Vector2f topRight;
	sf::Vector2f botLeft;
	sf::Vector2f botRight;

	int diffX = (lineEnd.x - lineOrigin.x) > 0 ? 1 : -1;
	int diffY = (lineEnd.y - lineOrigin.y) > 0 ? 1 : -1;

	int width = otherPlayer->collisionSprite.getTextureRect().width;
	int height = otherPlayer->collisionSprite.getTextureRect().height;

	otherPlayer->collisionSprite.setOrigin(width / 2, height / 2);

	otherPlayer->collisionSprite.setPosition(otherPlayer->position);

	sf::Vector2f test = otherPlayer->collisionSprite.getPosition();

	topLeft.x = otherPlayer->collisionSprite.getGlobalBounds().left;
	topLeft.y = otherPlayer->collisionSprite.getGlobalBounds().top;

	topRight.x = otherPlayer->collisionSprite.getGlobalBounds().left + otherPlayer->collisionSprite.getGlobalBounds().width;
	topRight.y = otherPlayer->collisionSprite.getGlobalBounds().top;

	botLeft.x = otherPlayer->collisionSprite.getGlobalBounds().left;
	botLeft.y = otherPlayer->collisionSprite.getGlobalBounds().top + otherPlayer->collisionSprite.getGlobalBounds().height;

	botRight.x = otherPlayer->collisionSprite.getGlobalBounds().left + otherPlayer->collisionSprite.getGlobalBounds().width;
	botRight.y = otherPlayer->collisionSprite.getGlobalBounds().top + otherPlayer->collisionSprite.getGlobalBounds().height;

	sf::Vector2f leftCollision = checkLineCollision(lineOrigin, lineEnd, botLeft, topLeft);
	sf::Vector2f rightCollision = checkLineCollision(lineOrigin, lineEnd, topRight, botRight);
	sf::Vector2f topCollision = checkLineCollision(lineOrigin, lineEnd, topLeft, topRight);
	sf::Vector2f botCollision = checkLineCollision(lineOrigin, lineEnd, botLeft, botRight);

	sf::Vector2f outputCollision(-1, -1);
	float dist = 100000.f;

	// Checks if each collision is within bounds of line, in the right direction, and the nearest collision to the origin point
	if ((leftCollision.y > topLeft.y && leftCollision.y < botLeft.y) && ((leftCollision.y - lineOrigin.y) * diffY > 0)) {
		outputCollision = leftCollision;
		dist = Distance(outputCollision, lineOrigin);
	}
	if ((rightCollision.y > topRight.y && rightCollision.y < botLeft.y) && ((rightCollision.y - lineOrigin.y) * diffY > 0)) {
		if (Distance(rightCollision, lineOrigin) < dist) {
			outputCollision = rightCollision;
			dist = Distance(rightCollision, lineOrigin);
		}
	}
	if ((topCollision.x > topLeft.x && topCollision.x < topRight.x) && ((topCollision.x - lineOrigin.x) * diffX > 0)) {
		if (Distance(topCollision, lineOrigin) < dist) {
			outputCollision = topCollision;
			dist = Distance(topCollision, lineOrigin);
		}
	}
	if ((botCollision.x > botLeft.x && botCollision.x < botRight.x) && ((botCollision.x - lineOrigin.x) * diffX > 0)) {
		if (Distance(botCollision, lineOrigin) < dist) {
			outputCollision = botCollision;
			dist = Distance(botCollision, lineOrigin);
		}
	}

	return outputCollision;
}

sf::Vector2f Simulation::checkLineCollision(sf::Vector2f originOne, sf::Vector2f pointOne, sf::Vector2f originTwo, sf::Vector2f pointTwo)
{
	sf::Vector2f outputVector;
	// Line one equation (y = mx + c)
	float mOne = pointOne.y - originOne.y;
	mOne /= (pointOne.x - originOne.x);

	float cOne = originOne.y - (mOne * originOne.x);

	// Acount for vertical lines
	if (originTwo.x == pointTwo.x)
	{
		float y = (mOne * originTwo.x) + cOne;
		outputVector.x = originTwo.x;
		outputVector.y = y;

		return outputVector;
	}

	// Line Two equation (y = mx + c)
	float mTwo = pointTwo.y - originTwo.y;
	mTwo /= (pointTwo.x - originTwo.x);

	float cTwo = originTwo.y - (mTwo * originTwo.x);

	// Equate for intersection
	outputVector.x = (mTwo * originTwo.x) + (cTwo - cOne);
	outputVector.x /= mOne;

	outputVector.y = (mOne * outputVector.x) + cOne;

	return outputVector;
}

float Simulation::Distance(sf::Vector2f pointOne, sf::Vector2f pointTwo)
{
	float dist;
	dist = (std::pow(pointOne.x - pointTwo.x, 2) + std::pow(pointTwo.y - pointTwo.y, 2));
	dist = std::sqrtf(dist);
	return dist;
}
