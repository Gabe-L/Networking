#include "pch.h"

#include "Game.h"

Game::Game(sf::RenderWindow* hwnd)
{
	window = hwnd;

	// Setup player
	playerTexture.loadFromFile("gfx/playerSoldSprite.png");
	enemyTexture.loadFromFile("gfx/enemySoldSprite.png");
	
	mapTexture.loadFromFile("gfx/dust2.jpg");
	map.setTexture(mapTexture);
	map.setScale(2.0f, 2.0f);

	localPlayer.setTexture(playerTexture);
	localPlayer.OriginToCentre();
	localPlayer.setScale(0.5f, 0.5f);
	localIdentity = -1;

	// Setup walls
	
	sf::Vector2f wallPos[4] = {
		sf::Vector2f(-1, 0),
		sf::Vector2f(0, 1),
		sf::Vector2f(1, 0),
		sf::Vector2f(0, -1)
	};

	wallTexture.loadFromFile("gfx/wallSprite.png");

	for (int i = 0; i < 4; i++)
	{
		walls[i].setTexture(wallTexture);
		int width = walls[i].getTextureRect().width;
		int height = walls[i].getTextureRect().height;
		walls[i].setOrigin(width / 2, height / 2);
		walls[i].setPosition(window->getSize().x / 2 + ((window->getSize().x / 4) * wallPos[i].x), window->getSize().y / 2 + ((window->getSize().x / 4) * wallPos[i].y));
		int next = i + 1;
		next = next > 3 ? 0 : next;
		walls[i].scale(abs(wallPos[next].x) + 0.5, abs(wallPos[next].y) + 0.5);
	}

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
	sf::Vector2f rotVector(MousePos.x - localPlayer.getPosition().x, MousePos.y - localPlayer.getPosition().y);
	float mag = pow(rotVector.x, 2) + pow(rotVector.y, 2);
	mag = sqrtf(mag);
	rotVector /= mag;
	
	float rotation = (atan2(rotVector.y, rotVector.x)) * 180 / 3.141;
	localPlayer.setRotation(rotation);


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
		if (enemy->messageHistory.size() >= 3) {
			enemy->PredictPosition(totalTime + latency, false);
			//enemy->setRotation(enemy->messageHistory[enemy->messageHistory.size() - 1].angle);
		}
	}

	for (int i = 0; i < enemies.size(); i++) {
		if (enemies.at(i)->messageHistory.size() > 0) {
			enemies.at(i)->setRotation(enemies.at(i)->messageHistory[enemies.at(i)->messageHistory.size() - 1].rotation);
		}
	}

	localPlayer.update(deltaTime);

	sf::Vector2f direction = MousePos;
	direction -= localPlayer.getPosition();

	sf::Vector2f collisionPos(-1, -1);
	sf::Vector2f tempCollision(-1, -1);
	float distance = 100000.0f;

	for (auto enemy : enemies) {
		tempCollision = HitScan(localPlayer.getPosition(), MousePos, *enemy);
		if (tempCollision.x != -1) {
			float tempDistance = Distance(localPlayer.getPosition(), tempCollision);
			if (tempDistance < distance) {
				localPlayer.hitEnemy = true;
				collisionPos = tempCollision;
				distance = tempDistance;
			}
		}
	}

	for (int i = 0; i < 4; i++) {
		tempCollision = HitScan(localPlayer.getPosition(), MousePos, walls[i]);
		if (tempCollision.x != -1) {
			float tempDistance = Distance(localPlayer.getPosition(), tempCollision);
			if (tempDistance < distance) {
				localPlayer.hitEnemy = false;
				collisionPos = tempCollision;
				distance = tempDistance;
			}
		}
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

		for (int i = 0; i < enemies.size(); i++) {
			enemies.at(i)->timeSinceLastMessage += tick;
			if (enemies.at(i)->timeSinceLastMessage > 5.0f) {
				printf("Enemy %i timeout\n", enemies.at(i)->playerID);
				enemies.erase(enemies.begin() + i);
			}
		}

		/*if (tickCount % (64 * 10) == 0) {
			printf("10 seconds have passed, pinging\n");
			pingServer(&localSock, serverAddr, serverPort);
		}*/

		//std::printf("Time: %.2f\n", totalTime);
		tick = 0.0f;
		// Send position data
		ClientInfo sendData(localIdentity, localPlayer.getPosition().x, localPlayer.getPosition().y, -1.0f, -1.0f, localPlayer.getRotation(), totalTime);

		if (clicked && localPlayer.hitEnemy) {
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
				<< sendData.rotation
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

	for (int i = 0; i < 4; i++) {
		window->draw(walls[i]);
	}
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

	//// Quadratic
	float x_acc, y_acc;
	float x_v = 0.f, y_v = 0.f;
	float x_u = 0.f, y_u = 0.f;

	float finalTime = msg0.time - msg1.time;

	if (finalTime != 0.0f) {
		x_v = (msg0.positionX - msg1.positionX) / finalTime;
		y_v = (msg0.positionY - msg1.positionY) / finalTime;
	}
	else {
		return;
	}
	
	float initialTime = msg1.time - msg2.time;

	if (initialTime != 0.0f) {
		x_u = (msg1.positionX - msg2.positionX) / initialTime;
		y_u = (msg1.positionY - msg2.positionY) / initialTime;
	}
	else {
		return;
	}


	x_acc = x_v - x_u;
	y_acc = y_v - y_u;

	float overallTime = msg0.time - msg2.time;

	if (overallTime != 0.0f) {
		x_acc /= overallTime;
		y_acc /= overallTime;
	}
	else {
		return;
	}


	// Displacement
	// s = ut + 0.5 * at ^2

	float x_disp = 0.f;
	float currentTime = time - msg0.time;

	x_disp += x_u * currentTime;
	x_disp += 0.5 * (x_acc * pow(currentTime, 2));

	float y_disp = 0.f;

	y_disp += y_u * currentTime;
	y_disp += 0.5 * (y_acc * pow(currentTime, 2));

	if (x_disp && y_disp) {
		enemy->setPosition(sf::Vector2f(msg0.positionX + (x_disp), msg0.positionY + (y_disp)));
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

sf::Vector2f Game::HitScan(sf::Vector2f lineOrigin, sf::Vector2f lineEnd, sf::Sprite otherPlayer)
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

		latency = (totalTime - pingMessage.time) / 2;
		totalTime = (pingMessage.totalTime + latency);
		printf("Ping successfull\nLatency: %f\n", latency);
	}
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
			_packet >> enemyInfo.rotation;
			_packet >> enemyInfo.time;

			if (enemyInfo.playerID == localIdentity || localIdentity == -1) {
				continue;
			}

			for (auto enemy : enemies) {
				if (enemy->playerID == enemyInfo.playerID) {
					enemy->timeSinceLastMessage = 0.0f;
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

	case MessageType::Terminate:
	{
		int playerID;
		_packet >> playerID;

		if (playerID == -1) {
			printf("Server disconnected\n");
			enemies.clear();
		}
		else {
			for (int i = 0; i < enemies.size(); i++) {
				if (enemies.at(i)->playerID == playerID) {
					printf("Player %i disconnected from the server\n", playerID);
					enemies.erase(enemies.begin() + i);
					break;
				}
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
	sf::Packet pingInfo;
	SetUpMessage pingMsg;

	pingMsg.messageType = MessageType::PingRequest;
	pingMsg.time = totalTime;
	pingMsg.totalTime = totalTime;
	pingMsg.playerIdentity = localIdentity;

	pingInfo << pingMsg.messageType << pingMsg.time << pingMsg.totalTime << pingMsg.playerIdentity;

	if (_sock->send(pingInfo, _addr, _port) != sf::Socket::Done) {
		printf("Ping send failed\n");
	}

	return;
}