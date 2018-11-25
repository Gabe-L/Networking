#pragma once

enum MessageType {
	Setup,
	Position,
	PingRequest,
	PingInfo,
	PlayerPositions
};

struct BaseMessage {
	int messageType;

	BaseMessage() :
		messageType(0)
	{
	}
};

struct SetUpMessage {
	int pingType;
	float time;
	float totalTime;
	int playerIdentity;

	SetUpMessage() :
		pingType(MessageType::Setup),
		time(0.0f),
		totalTime(0.0f),
		playerIdentity(-1)
	{
	}

};

struct PlayerCount : BaseMessage{
	int playerCount;

	PlayerCount() :
		//messageType(MessageType::PlayerPositions),
		playerCount(0)
	{
		messageType = MessageType::PlayerPositions;
	}

};

struct PlayerInfo {
	int playerID;
	float positionX;
	float positionY;
};

struct NetMessage {
	int messageType;
	int enemyID;
	float positionX;
	float positionY;
	float mousePosX;
	float mousePosY;
	float time;

	NetMessage() :
		messageType(MessageType::Position),
		enemyID(-1),
		positionX(-1.0f),
		positionY(-1.0f),
		mousePosX(-1.0f),
		mousePosY(-1.0f),
		time(-1.0f)
	{
	}
};