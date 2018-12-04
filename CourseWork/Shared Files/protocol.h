#pragma once

enum MessageType {
	Position,
	PingRequest,
	PingInfo,
	PlayerPositions,
	Shot,
	Terminate
};

struct BaseMessage {
	int messageType;

	BaseMessage() :
		messageType(-1)
	{
	}
};

struct ShotMessage : BaseMessage {
	float fromX;
	float fromY;
	float toX;
	float toY;

	ShotMessage(float _fromX, float _fromY, float _toX, float _toY):
		fromX(_fromX),
		fromY(_fromY),
		toX(_toX),
		toY(_toY)
	{
		messageType = MessageType::Shot;
	}
};

struct TerminateMessage : BaseMessage{
	int palyerID;

	TerminateMessage()
	{
		messageType = MessageType::Terminate;
	}
};

struct SetUpMessage : BaseMessage {
	float time;
	float totalTime;
	int playerIdentity;

	SetUpMessage()
	{
	}

	SetUpMessage(float _time, float _totalTime, int _playerIdentity) :
		time(_time),
		totalTime(_totalTime),
		playerIdentity(_playerIdentity)
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
	float rotation;
	float time;
};

struct ClientInfo : BaseMessage {
	int enemyID;
	float positionX;
	float positionY;
	float mousePosX;
	float mousePosY;
	float rotation;
	float time;

	ClientInfo()
	{
	}

	ClientInfo(int _enemyID, float _positionX, float _positionY, float _mousePosX, float mousePosY, float _rotation, float _time) :
		enemyID(_enemyID),
		positionX(_positionX),
		positionY(_positionY),
		mousePosX(_mousePosX),
		mousePosY(mousePosY),
		rotation(_rotation),
		time(_time)
	{
		messageType = MessageType::Position;
	}
};