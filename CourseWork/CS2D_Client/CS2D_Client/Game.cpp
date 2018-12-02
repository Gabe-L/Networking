#include "pch.h"

#include "Game.h"

Game::Game(sf::RenderWindow* hwnd)
{
	window = hwnd;

	// Setup player
	playerTexture.loadFromFile("gfx/playerSprite.png");
	enemyTexture.loadFromFile("gfx/enemySprite.png");
	
	mapTexture.loadFromFile("gfx/dust2.jpg");
	map.setTexture(mapTexture);
	map.setScale(2.0f, 2.0f);

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
	TerminateMessage termMsg;
	sf::Packet termPack;
	termPack << termMsg.messageType;

	sendMessage(termPack, serverAddr, serverPort);

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

	for (auto enemy : enemies) {
		int xpos = enemy->getPosition().x;
		if (enemy->messageHistory.size() >= 3) {
			predictPosition(totalTime + (latency), enemy);
		}
	}

	localPlayer.update(deltaTime);

	sf::Vector2f direction = MousePos;
	direction -= localPlayer.getPosition();

	//sf::Vector2f collisionPos = HitScan(localPlayer.getPosition(), MousePos, collider);
	sf::Vector2f collisionPos(-1, -1);

	for (auto enemy : enemies) {
		collisionPos = HitScan(localPlayer.getPosition(), MousePos, *enemy);
	}

	hitScanLine[0].position = localPlayer.getPosition();
	hitScanLine[1].position = MousePos;

	if (collisionPos.x > 0) {
		hitScanLine[1].position = collisionPos;
		MousePos = collisionPos;
	}

	hitScanLine[0].color = lineColour;
	hitScanLine[1].color = lineColour;

	if (tick >= 1.f / 64)
	{
		tickCount++;

		/*if (tickCount % (64 * 60) == 0) {
			printf("60 seconds have passed, pinging\n");
			pingServer(&localSock, serverAddr, serverPort);
		}*/

		//std::printf("Time: %.2f\n", totalTime);
		tick = 0.0f;
		// Send position data
		ClientInfo sendData(localIdentity, localPlayer.getPosition().x, localPlayer.getPosition().y, -1.0f, -1.0f, totalTime);

		if (clicked) {
			sendData.mousePosX = MousePos.x;
			sendData.mousePosY = MousePos.y;
		}

		if (sendData.time != previousTimeSend) {
			previousTimeSend = sendData.time;
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
	}

}
void Game::handleInput(Input* input, float dt)
{
	if (input->isKeyDown(sf::Keyboard::W) && localPlayer.getVelocity().y > -200) {
		
		float velMod = localPlayer.getVelocity().y > 0 ? 4 : 1;

		localPlayer.setVelocity({ localPlayer.getVelocity().x,localPlayer.getVelocity().y - 200 * dt * velMod });
	}
	else if (input->isKeyDown(sf::Keyboard::S) && localPlayer.getVelocity().y < 200) {

		float velMod = localPlayer.getVelocity().y < 0 ? 4 : 1;

		localPlayer.setVelocity({ localPlayer.getVelocity().x,localPlayer.getVelocity().y + 200 * dt * velMod});
	}
	else {
		//test.setVelocity({ test.getVelocity().x,0 });

	}

	if (input->isKeyDown(sf::Keyboard::A) && localPlayer.getVelocity().x > -200) {

		float velMod = localPlayer.getVelocity().x > 0 ? 4 : 1;

		localPlayer.setVelocity({ localPlayer.getVelocity().x - 200 * dt * velMod,localPlayer.getVelocity().y });
	}
	else if (input->isKeyDown(sf::Keyboard::D) && localPlayer.getVelocity().x < 200) {
		
		float velMod = localPlayer.getVelocity().x < 0 ? 4 : 1;

		localPlayer.setVelocity({ localPlayer.getVelocity().x + 200 * dt * velMod,localPlayer.getVelocity().y });
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
	
	window->draw(map);

	window->draw(localPlayer);

	for (auto enemy : enemies) {
		window->draw(*enemy);
	}

	window->draw(collider);
	window->draw(hitScanLine);

	renderUI();
	endDraw();
}

void Game::predictPosition(float time, Player* enemy) {


	const int msize = enemy->messageHistory.size();
	if (msize < 3) {
		return;
	}
	//assert(msize >= 3);
	const PlayerInfo& msg0 = enemy->messageHistory[msize - 1];
	const PlayerInfo& msg1 = enemy->messageHistory[msize - 2];
	const PlayerInfo& msg2 = enemy->messageHistory[msize - 3];
	
	// Linear
	
	/*float timePassed = msg0.time - msg1.time;
	if (timePassed == 0.0f) { return; }


	float x_speed = (msg0.positionX - msg1.positionX) / timePassed;
	float y_speed = (msg0.positionY - msg1.positionY) / timePassed;

	timePassed = time - msg0.time;
	if (timePassed == 0.0f) { return; }

	enemy->setPosition(sf::Vector2f(msg0.positionX + (x_speed * timePassed), msg0.positionY + (y_speed * timePassed)));*/

	// Quadratic
	float x_acc, y_acc;
	float x_v = 0.f, y_v = 0.f;
	float x_u = 0.f, y_u = 0.f;

	float timePassed = msg0.time - msg1.time;

	if (timePassed != 0.0f) {
		x_v = (msg0.positionX - msg1.positionX) / timePassed;
		y_v = (msg0.positionY - msg1.positionY) / timePassed;
	}
	else {
		x_v = 0.0f;
		y_v = 0.0f;
	}
	timePassed = msg1.time - msg2.time;

	if (timePassed != 0.0f) {
		x_u = (msg1.positionX - msg2.positionX) / timePassed;
		y_u = (msg1.positionY - msg2.positionY) / timePassed;
	}
	else {
		x_u = 0.0f;
		y_u = 0.0f;
	}


	x_acc = x_v - x_u;
	y_acc = y_v - y_u;

	if (msg0.time - msg2.time != 0.0f) {
		x_acc /= msg0.time - msg2.time;
		y_acc /= msg0.time - msg2.time;
	}
	else {
		x_acc = 0.0f;
		y_acc = 0.0f;
	}


	// Displacement
	// s = ut + 0.5 * at ^2

	float x_disp = 0.f;
	timePassed = time - msg0.time;

	x_disp += x_u * timePassed;
	x_disp += 0.5 * (x_acc * pow(timePassed, 2));

	float y_disp = 0.f;
	timePassed = time - msg0.time;

	y_disp += y_u * timePassed;
	y_disp += 0.5 * (y_acc * pow(timePassed, 2));

	if (x_disp && y_disp) {
		enemy->setPosition(sf::Vector2f(msg0.positionX + (x_disp ), msg0.positionY + (y_disp)));
	}
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

sf::Vector2f Game::HitScan(sf::Vector2f lineOrigin, sf::Vector2f lineEnd, Player otherPlayer)
{
	sf::Vector2f topLeft;
	sf::Vector2f topRight;
	sf::Vector2f botLeft;
	sf::Vector2f botRight;

	int diffX = (lineEnd.x - lineOrigin.x) > 0 ? 1 : -1;
	int diffY = (lineEnd.y - lineOrigin.y) > 0 ? 1 : -1;

	topLeft.x = otherPlayer.getGlobalBounds().left;
	topLeft.y = otherPlayer.getGlobalBounds().top;

	topRight.x = otherPlayer.getGlobalBounds().left + otherPlayer.getGlobalBounds().width;
	topRight.y = otherPlayer.getGlobalBounds().top;

	botLeft.x = otherPlayer.getGlobalBounds().left;
	botLeft.y = otherPlayer.getGlobalBounds().top + otherPlayer.getGlobalBounds().height;

	botRight.x = otherPlayer.getGlobalBounds().left + otherPlayer.getGlobalBounds().width;
	botRight.y = otherPlayer.getGlobalBounds().top + otherPlayer.getGlobalBounds().height;

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

	switch (messageType) {
	case MessageType::PingRequest:
	{
		SetUpMessage pingMessage;
		sf::Packet pingPack;
		float messageTime;
		_packet >> messageTime;

		pingMessage.messageType = MessageType::PingInfo;
		pingPack
			<< MessageType::PingInfo
			<< messageTime
			<< totalTime;

		if (localSock.send(pingPack, serverAddr, serverPort) != sf::Socket::Done) {
			printf("Data send failed\n");
		}
	}
		break;

	case MessageType::PingInfo:
	{
		SetUpMessage pingMessage;
		pingMessage.messageType = messageType;

		_packet
			>> pingMessage.time
			>> pingMessage.totalTime
			>> pingMessage.playerIdentity;

		if (localIdentity < 0) {
			localIdentity = pingMessage.playerIdentity;
		}

		latency = (totalTime - pingMessage.time);
		totalTime = pingMessage.totalTime + latency;
		printf("Ping successfull\nLatency: %f\n", latency);
	}
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
	{
		int playerCount;

		_packet >> playerCount;

		for (int i = 0; i < playerCount; i++) {

			PlayerInfo enemyInfo;

			bool found = false;
			_packet >> enemyInfo.playerID;
			_packet >> enemyInfo.positionX;
			_packet >> enemyInfo.positionY;
			_packet >> enemyInfo.time;

			if (enemyInfo.playerID == localIdentity || localIdentity == -1) {
				continue;
			}

			for (auto enemy : enemies) {
				if (enemy->playerID == enemyInfo.playerID) {
					//enemy->setPosition(enemyInfo.positionX, enemyInfo.positionY);
					found = true;
					int messagesSize = enemy->messageHistory.size();

					enemy->messageHistory.push_back(enemyInfo);
					if (messagesSize > 3) {
						enemy->messageHistory.erase(enemy->messageHistory.begin());
					}
				}
			}

			if (!found) {
				Player* newEnemy = new Player();

				newEnemy->setTexture(enemyTexture);
				newEnemy->playerID = enemyInfo.playerID;
				newEnemy->OriginToCentre();
				newEnemy->setScale(0.5f, 0.5f);
				newEnemy->setPosition(enemyInfo.positionX, enemyInfo.positionY);

				enemies.push_back(newEnemy);
			}

		}
	}
		break;

	}
}

void Game::sendMessage(sf::Packet _packet, sf::IpAddress _destAddr, unsigned short _destPort)
{
	// Simulate packet drop
	// Lose 40% of packets
	//if ((rand() % 100) > 20)
	{
		if (localSock.send(_packet, _destAddr, _destPort) != sf::Socket::Done) {
			printf("Data send failed\n");
		}
	}
}

void Game::pingServer(sf::UdpSocket* _sock, sf::IpAddress _addr, unsigned short _port)
{
	/*for (auto enemy : enemies) {
		enemy->messageHistory.clear();
	}

	sf::Packet pingInfo;
	SetUpMessage pingMsg;

	pingMsg.messageType = MessageType::PingRequest;
	pingMsg.time = totalTime;
	pingMsg.totalTime = totalTime;
	pingMsg.playerIdentity = localIdentity;

	pingInfo << pingMsg.messageType << pingMsg.time << pingMsg.totalTime << pingMsg.playerIdentity;

	if (_sock->send(pingInfo, _addr, _port) != sf::Socket::Done) {
		printf("Ping send failed\n");
	}*/

	return;
}