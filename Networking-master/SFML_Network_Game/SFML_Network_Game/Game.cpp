#include "Game.h"

Game::Game(sf::RenderWindow* hwnd)
{
	window = hwnd;

	// Setup player
	tst.loadFromFile("gfx/playerSprite.png");
	test.setTexture(tst);
	test.OriginToCentre();
	test.setScale(0.5f, 0.5f);

	// Setup test collider
	collider.setTexture(tst);
	collider.setPosition(window->getSize().x / 2, window->getSize().y / 2);

	hitScanLine = sf::VertexArray(sf::LinesStrip, 2);
	lineColour = sf::Color::White;

	totalTime = 0.0f;
	tick = 0.0f;

	addr = "127.0.0.1";
	port = 255;
	
	clientAddr = "127.0.0.1";
	clientport = 256;
	sock.setBlocking(false);

	if (sock.bind(port, addr) != sf::Socket::Done)
	{
		std::printf("Socket did not bind\n");
	}
	latency = 0.0f;

}
Game::~Game()
{
}

void Game::predictPosition(float time) {
	const int msize = messsageHistory.size();
	assert(msize >= 3);
	const NetMessage& msg0 = messsageHistory[msize - 1];
	const NetMessage& msg1 = messsageHistory[msize - 2];
	const NetMessage& msg2 = messsageHistory[msize - 3];

	float x_acc, y_acc;
	float x_v = 0.f, y_v = 0.f;
	float x_u = 0.f, y_u = 0.f;

	float timePassed = msg0.time - msg1.time;

	x_v = (msg0.positionX - msg1.positionX) / timePassed;
	y_v = (msg0.positionY - msg1.positionY) / timePassed;

	timePassed = msg1.time - msg2.time;

	x_u = (msg1.positionX - msg2.positionX) / timePassed;
	y_u = (msg1.positionY - msg2.positionY) / timePassed;

	x_acc = x_v - x_u;
	y_acc = y_v - y_u;

	x_acc /= time - msg2.time;
	y_acc /= time - msg2.time;

	// Displacement
	// s = ut + 0.5 * at ^2

	float x_disp = 0.f;
	timePassed = time - msg0.time;

	x_disp += x_u * timePassed;
	x_disp += 0.5 * pow(x_acc * timePassed, 2);

	float y_disp = 0.f;
	timePassed = time - msg0.time;

	y_disp += y_u * timePassed;
	y_disp += 0.5 * pow(y_acc * timePassed, 2);

	test.setPosition(sf::Vector2f(msg0.positionX + x_disp, msg0.positionY + y_disp));

}

void Game::update(float deltaTime)
{

	//test.update(deltaTime);

	sf::Vector2f newPos;

	//size_t dataSize = sizeof(newPos);
	sf::Packet posPack;

	sf::IpAddress recvAddr;
	unsigned short recvPort;

	totalTime += deltaTime;
	tick += deltaTime;

	if (messsageHistory.size() >= 3) {
		predictPosition(totalTime + latency);
		if (tick >= 1.f /64)
		{
			tick = 0.0f;
			pingServer(&sock, clientAddr, clientport);
		}
	}

	if (sock.receive(posPack, recvAddr, recvPort) == sf::Socket::Done)
	{

		int messageType;
		NetMessage recvMessage;
		SetUpMessage pingMessage;

		posPack >> messageType;
		switch (messageType) {
		case MessageType::Position:

			posPack
				>> recvMessage.positionX
				>> recvMessage.positionY
				>> recvMessage.mousePosX
				>> recvMessage.mousePosY
				>> recvMessage.time;

			messsageHistory.push_back(recvMessage);
			break;
		case MessageType::PingInfo:
			posPack
				>> pingMessage.time
				>> pingMessage.totalTime;
			latency = (totalTime - pingMessage.time);
			totalTime = pingMessage.totalTime;
			printf("Latency: %f\n", latency);
			break;
		}

	}

	sf::Vector2f direction = MousePos;
	direction -= test.getPosition();

	sf::Vector2f collisionPos = HitScan(test.getPosition(), MousePos, collider);

	hitScanLine[0].position = test.getPosition();
	hitScanLine[1].position = MousePos;

	if (collisionPos.x > 0) {
		//hitScanLine[1].position = collisionPos;
	}

	hitScanLine[0].color = lineColour;
	hitScanLine[1].color = lineColour;

}
void Game::handleInput(Input* input)
{
	if (input->isKeyDown(sf::Keyboard::W)) {
		test.setVelocity({ test.getVelocity().x,-100 });
	}
	else if (input->isKeyDown(sf::Keyboard::S)) {
		test.setVelocity({ test.getVelocity().x,100 });
	}
	else {
		test.setVelocity({ test.getVelocity().x,0 });

	}

	if (input->isKeyDown(sf::Keyboard::A)) {
		test.setVelocity({ -100,test.getVelocity().y });
	}
	else if (input->isKeyDown(sf::Keyboard::D)) {
		test.setVelocity({ 100,test.getVelocity().y });
	}
	else {
		test.setVelocity({ 0,test.getVelocity().y });
	}


	if (input->isLmbDown()) {
		lineColour = sf::Color::Red;
	}
	else {
		lineColour = sf::Color::White;
	}

	MousePos.x = input->getMouseX();
	MousePos.y = input->getMouseY();

}
void Game::render()
{
	beginDraw();

	window->draw(test);
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