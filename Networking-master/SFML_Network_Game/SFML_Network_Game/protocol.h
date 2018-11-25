#pragma once

enum MessageType {
	Setup,
	Position,
	PingRequest,
	PingInfo
};

struct SetUpMessage {
	int pingType;
	float time;
	float totalTime;

	SetUpMessage() :
		pingType(MessageType::Setup),
		time(0.0f),
		totalTime(0.0f)
	{
	}

};

struct NetMessage {
	int messageType;
	float positionX;
	float positionY;
	float mousePosX;
	float mousePosY;
	float time;

	NetMessage() :
		messageType(MessageType::Position),
		positionX(-1.0f),
		positionY(-1.0f),
		mousePosX(-1.0f),
		mousePosY(-1.0f),
		time(-1.0f)
	{
	}
};