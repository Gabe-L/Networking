#pragma once
#include "pch.h"

#include "Player.h"
#include "Input.h"
#include <vector>

using namespace std;

class Game
{
public:
	Game(sf::RenderWindow* hwnd);
	~Game();

	void handleInput(Input* input, float dt);
	void update(float deltaTime);
	void render();


private:
	sf::RenderWindow* window;
	void predictPosition(float time, Player* enemy);
	bool checkCollision(sf::Sprite* s1, sf::Sprite* s2);
	sf::Vector2f checkLineCollision(sf::Vector2f originOne, sf::Vector2f pointOne, sf::Vector2f originTwo, sf::Vector2f pointTwo);
	sf::Vector2f HitScan(sf::Vector2f lineOrigin, sf::Vector2f lineEnd, sf::Sprite otherPlayer);
	float Distance(sf::Vector2f pointOne, sf::Vector2f pointTwo);

	void beginDraw();
	void endDraw();
	void renderUI();
	void reset();
	void processMessage(sf::Packet _packet);
	void sendMessage(sf::Packet _packet, sf::IpAddress _destAddr, unsigned short _destPort);
	void pingServer(sf::UdpSocket* _sock, sf::IpAddress _addr, unsigned short _port);

	// #TODO: Clear test vars
	Player localPlayer;
	int localIdentity;
	sf::Texture playerTexture;
	sf::Texture enemyTexture;
	
	sf::Sprite walls[4];
	sf::Texture wallTexture;

	sf::Texture mapTexture;
	sf::Sprite map;

	// Local network info
	sf::UdpSocket localSock;

	sf::IpAddress localAddr;
	unsigned short localPort;


	// Server network info
	sf::IpAddress serverAddr;
	unsigned short serverPort;

	// Enemies
	std::vector<Player*> enemies;

	bool clicked;
	int tickCount = 0;
	float tick;
	float totalTime;
	float previousTimeSend = -1.0f;
	float latency;

	sf::VertexArray hitScanLine;
	sf::Color lineColour;

	sf::Vector2f MousePos;

};

