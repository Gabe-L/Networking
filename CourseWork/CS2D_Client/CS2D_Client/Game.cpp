#include "pch.h"

#include "Game.h"

Game::Game(sf::RenderWindow* hwnd)
{
	window = hwnd;

	// Setup player
	playerTexture.loadFromFile("gfx/playerSprite.png");
	enemyTexture.loadFromFile("gfx/enemySprite.png");

	localPlayer.setTexture(playerTexture);
	localPlayer.OriginToCentre();
	localPlayer.setScale(0.5f, 0.5f);
	localIdentity = -1;

	// Setup test collider
	collider.setTexture(playerTexture);
	collider.setPosition(window->getSize().x / 2, window->getSize().y / 2);

	latency = 0.0f;
	tick = 0.f;
	totalTime = 0.0f;

	hitScanLine = sf::VertexArray(sf::LinesStrip, 2);
	lineColour = sf::Color::White;

	localAddr = "127.0.0.1";

	serverAddr = "127.0.0.1";
	serverPort = 255;
	clicked = false;

	if (localSock.bind(sf::Socket::AnyPort, localAddr) != sf::Socket::Done)
	{
		std::printf("Socket did not bind\n");
	}
	localPort = localSock.getLocalPort();
	
	// Join by pinging server
	pingServer(&localSock, serverAddr, serverPort);


	localSock.setBlocking(false);

}
Game::~Game()
{
}

void Game::update(float deltaTime)
{
	
	tick += deltaTime;
	totalTime += deltaTime;


	sf::Packet ipPack;
	sf::IpAddress recvAddr;
	unsigned short recvPort;

	if (localSock.receive(ipPack, recvAddr, recvPort) == sf::Socket::Done)
	{
		processMessage(ipPack);
	}

	if (tick >= 1.f / 64)
	{
		tick = 0.0f;
		// Send position data
		NetMessage sendData;
		sendData.messageType = MessageType::Position;
		sendData.enemyID = localIdentity;//localPlayer.playerID;
		sendData.positionX = localPlayer.getPosition().x;
		sendData.positionY = localPlayer.getPosition().y;
		sendData.mousePosX = MousePos.x;
		sendData.mousePosY = MousePos.y;
		sendData.time = totalTime;

		sf::Packet packetInfo;
		packetInfo
			<< sendData.messageType
			<< sendData.enemyID
			<< sendData.positionX
			<< sendData.positionY
			<< sendData.mousePosX
			<< sendData.mousePosY
			<< sendData.time;

		sendMessage(packetInfo, serverAddr, serverPort);

	}

	localPlayer.update(deltaTime);

	sf::Vector2f direction = MousePos;
	direction -= localPlayer.getPosition();

	sf::Vector2f collisionPos = HitScan(localPlayer.getPosition(), MousePos, collider);

	hitScanLine[0].position = localPlayer.getPosition();
	hitScanLine[1].position = MousePos;

	if (collisionPos.x > 0) {
		hitScanLine[1].position = collisionPos;
		MousePos = collisionPos;
	}

	hitScanLine[0].color = lineColour;
	hitScanLine[1].color = lineColour;

}
void Game::handleInput(Input* input)
{
	if (input->isKeyDown(sf::Keyboard::W)) {
		localPlayer.setVelocity({ localPlayer.getVelocity().x,localPlayer.getVelocity().y - 5 / 64.f });
	}
	else if (input->isKeyDown(sf::Keyboard::S)) {
		localPlayer.setVelocity({ localPlayer.getVelocity().x,localPlayer.getVelocity().y + 5 / 64.f });
	}
	else {
		//test.setVelocity({ test.getVelocity().x,0 });

	}

	if (input->isKeyDown(sf::Keyboard::A)) {
		localPlayer.setVelocity({ localPlayer.getVelocity().x - 5 / 64.f,localPlayer.getVelocity().y });
	}
	else if (input->isKeyDown(sf::Keyboard::D)) {
		localPlayer.setVelocity({ localPlayer.getVelocity().x + 5 / 64.f,localPlayer.getVelocity().y });
	}
	else {
		//test.setVelocity({ 0,test.getVelocity().y });
	}


	if (input->isLmbDown()) {
		lineColour = sf::Color::Red;
		clicked = true;
	}
	else {
		lineColour = sf::Color::White;
		clicked = false;
	}

	MousePos.x = input->getMouseX();
	MousePos.y = input->getMouseY();

}
void Game::render()
{
	beginDraw();

	window->draw(localPlayer);

	for (auto enemy : enemies) {
		window->draw(*enemy);
	}

	window->draw(collider);
	window->draw(hitScanLine);

	renderUI();
	endDraw();
}

bool Game::checkCollision(sf::Sprite* s1, sf::Sprite* s2)
{
	if (s1->getLocalBounds().left + s1->getLocalBounds().width < s2->getLocalBounds().left)
		return false;
	if (s1->getLocalBounds().left > s2->getLocalBounds().left + s2->getLocalBounds().width)
		return false;
	if (s1->getLocalBounds().top + s1->getLocalBounds().height < s2->getLocalBounds().top)
		return false;
	if (s1->getLocalBounds().top > s2->getLocalBounds().top + s2->getLocalBounds().height)
		return false;
	return true;
}

sf::Vector2f Game::checkLineCollision(sf::Vector2f originOne, sf::Vector2f pointOne, sf::Vector2f originTwo, sf::Vector2f pointTwo)
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

sf::Vector2f Game::HitScan(sf::Vector2f lineOrigin, sf::Vector2f lineEnd, sf::Sprite sprite)
{
	sf::Vector2f topLeft;
	sf::Vector2f topRight;
	sf::Vector2f botLeft;
	sf::Vector2f botRight;

	topLeft.x = sprite.getGlobalBounds().left;
	topLeft.y = sprite.getGlobalBounds().top;

	topRight.x = sprite.getGlobalBounds().left + collider.getGlobalBounds().width;
	topRight.y = sprite.getGlobalBounds().top;

	botLeft.x = sprite.getGlobalBounds().left;
	botLeft.y = sprite.getGlobalBounds().top + collider.getGlobalBounds().height;

	botRight.x = sprite.getGlobalBounds().left + collider.getGlobalBounds().width;
	botRight.y = sprite.getGlobalBounds().top + collider.getGlobalBounds().height;

	sf::Vector2f leftCollision = checkLineCollision(lineOrigin, lineEnd, botLeft, topLeft);
	sf::Vector2f rightCollision = checkLineCollision(lineOrigin, lineEnd, topRight, botRight);
	sf::Vector2f topCollision = checkLineCollision(lineOrigin, lineEnd, topLeft, topRight);
	sf::Vector2f botCollision = checkLineCollision(lineOrigin, lineEnd, botLeft, botRight);

	sf::Vector2f outputCollision(-1, -1);
	float dist = 100000.f;

	if (leftCollision.y > topLeft.y && leftCollision.y < botLeft.y) {
		outputCollision = leftCollision;
		dist = Distance(outputCollision, lineOrigin);
	}
	if (rightCollision.y > topRight.y && rightCollision.y < botLeft.y) {
		if (Distance(rightCollision, lineOrigin) < dist) {
			outputCollision = rightCollision;
			dist = Distance(rightCollision, lineOrigin);
		}
	}
	if (topCollision.x > topLeft.x && topCollision.x < topRight.x) {
		if (Distance(topCollision, lineOrigin) < dist) {
			outputCollision = topCollision;
			dist = Distance(topCollision, lineOrigin);
		}
	}
	if (botCollision.x > botLeft.x && botCollision.x < botRight.x) {
		if (Distance(botCollision, lineOrigin) < dist) {
			outputCollision = botCollision;
			dist = Distance(botCollision, lineOrigin);
		}
	}

	return outputCollision;
}

float Game::Distance(sf::Vector2f pointOne, sf::Vector2f pointTwo)
{
	float dist;
	dist = (std::pow(pointOne.x - pointTwo.x, 2) + std::pow(pointTwo.y - pointTwo.y, 2));
	dist = std::sqrtf(dist);
	return dist;
}

void Game::beginDraw()
{
	window->clear(sf::Color::Black);
}
void Game::endDraw()
{
	window->display();
}

void Game::renderUI() {

}

void Game::reset() {

}

void Game::processMessage(sf::Packet _packet)
{
	int messageType;

	_packet >> messageType;
	sf::Packet pingPack;
	SetUpMessage pingMessage;

	switch (messageType) {
	case MessageType::PingRequest:

		float messageTime;
		_packet >> messageTime;

		pingMessage.pingType = MessageType::PingInfo;
		pingPack
			<< MessageType::PingInfo
			<< messageTime
			<< totalTime;

		if (localSock.send(pingPack, serverAddr, serverPort) != sf::Socket::Done) {
			printf("Data send failed\n");
		}
		break;

	case MessageType::PingInfo:

		pingMessage.pingType = messageType;

		_packet
			>> pingMessage.time
			>> pingMessage.totalTime
			>> pingMessage.playerIdentity;

		if (localIdentity < 0) {
			localIdentity = pingMessage.playerIdentity;
		}

		latency = (totalTime - pingMessage.time);
		totalTime = pingMessage.totalTime;
		printf("Latency: %f\n", latency);
		break;

	case MessageType::Position:

		//NetMessage incomingPosition;
		//incomingPosition.messageType = messageType;

		//_packet
		//	<< incomingPosition.enemyID
		//	<< incomingPosition.positionX
		//	<< incomingPosition.positionY
		//	<< incomingPosition.mousePosX
		//	<< incomingPosition.mousePosY
		//	<< incomingPosition.time;

		//for (auto enemy : enemies) {
		//	if (enemy->playerID == incomingPosition.enemyID) {
		//		
		//		enemy->setPosition(incomingPosition.positionX, incomingPosition.positionY);
		//		break;
		//	}
		//}

		//// New enemy info received

		//Player* newEnemy = new Player();
		//
		//newEnemy->setTexture(enemyTexture);
		//newEnemy->setPosition(incomingPosition.positionX, incomingPosition.positionY);

		//enemies.push_back(newEnemy);

		break;

	case MessageType::PlayerPositions:
		int playerCount;

		_packet >> playerCount;

		for (int i = 0; i < playerCount; i++) {
			
			PlayerInfo enemyInfo;

			bool found = false;
			_packet >> enemyInfo.playerID;
			_packet >> enemyInfo.positionX;
			_packet >> enemyInfo.positionY;

			if (enemyInfo.playerID == localIdentity || localIdentity == -1) {
				continue;
			}

			for (auto enemy : enemies) {
				if (enemy->playerID == enemyInfo.playerID) {
					enemy->setPosition(enemyInfo.positionX, enemyInfo.positionY);
					found = true;
				}
			}

			if (!found) {
				Player* newEnemy = new Player();

				newEnemy->setTexture(enemyTexture);
				newEnemy->playerID = enemyInfo.playerID;
				newEnemy->setPosition(enemyInfo.positionX, enemyInfo.positionY);

				enemies.push_back(newEnemy);
			}

		}

		break;

	}
}

void Game::sendMessage(sf::Packet _packet, sf::IpAddress _destAddr, unsigned short _destPort)
{
	// Simulate packet drop
	// Lose 40% of packets
	if ((rand() % 100) > 20)
	{
		if (localSock.send(_packet, _destAddr, _destPort) != sf::Socket::Done) {
			printf("Data send failed\n");
		}
	}
}

void Game::pingServer(sf::UdpSocket* _sock, sf::IpAddress _addr, unsigned short _port)
{
	sf::Packet pingInfo;
	SetUpMessage pingMsg;

	pingMsg.pingType = MessageType::PingRequest;
	pingMsg.time = totalTime;
	pingMsg.totalTime = totalTime;
	pingMsg.playerIdentity = localIdentity;

	pingInfo << pingMsg.pingType << pingMsg.time << pingMsg.totalTime << pingMsg.playerIdentity;

	if (_sock->send(pingInfo, _addr, _port) != sf::Socket::Done) {
		printf("Ping send failed\n");
	}

	return;
}