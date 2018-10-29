/* The Connection class */

#include <cstdio>
#include <WinSock2.h>

#include "connection.h"
#include "utils.h"

#pragma comment(lib, "ws2_32.lib")

// Constructor.
// sock: the socket that we've accepted the client connection on.
Connection::Connection(SOCKET sock)
	: sock_(sock), readCount_(0), writeCount_(0)
{
	printf("New connection\n");
}

// Destructor.
Connection::~Connection()
{
	printf("Closing connection\n");
	closesocket(sock_);
}

// Process an incoming message.
void Connection::processMessage(const NetMessage *message)
{
	printf("Got network message: type %d, data %d\n", message->type, message->data);
	printf("Message said: %c \n", char(message->data));
}

// Add an outgoing message to the send buffer.
void Connection::sendMessage(const NetMessage *message)
{
	if (writeCount_ + sizeof(NetMessage) > sizeof(writeBuffer_))
	{
		die("writeBuffer_ full");
	}

	memcpy(writeBuffer_ + writeCount_, message, sizeof(NetMessage));
	writeCount_ += sizeof(NetMessage);
}

// Return the client's socket.
SOCKET Connection::sock()
{
	return sock_;
}

// Return whether this connection is in a state where we want to try
// reading from the socket.
bool Connection::wantRead()
{
	// At present, we always do.
	return true;
}

// Return whether this connection is in a state where we want to try
// writing to the socket.
bool Connection::wantWrite()
{
	// Only if we've got data to send.
	return writeCount_ > 0;
}

// Call this when the socket is ready to read.
// Returns true if the socket should be closed.
bool Connection::doRead()
{
	// Receive as much data from the client as will fit in the buffer.
	int spaceLeft = (sizeof readBuffer_) - readCount_;
	int count = recv(sock_, readBuffer_ + readCount_, spaceLeft, 0);
	if (count <= 0)
	{
		printf("Client connection closed or broken\n");
		return true;
	}

	// We've successfully read some more data into the buffer.
	readCount_ += count;

	if (readCount_ < sizeof NetMessage)
	{
		// ... but we've not received a complete message yet.
		// So we can't do anything until we receive some more.
		return false;
	}

	// We've got a complete message.
	processMessage((const NetMessage *)readBuffer_);

	// Clear the buffer, ready for the next message.
	readCount_ = 0;

	return false;
}

// Call this when the socket is ready to write.
// Returns true if the socket should be closed.
bool Connection::doWrite()
{
	int count = send(sock_, writeBuffer_, writeCount_, 0);
	if (count <= 0)
	{
		printf("Client connection closed or broken\n");
		return true;
	}

	writeCount_ -= count;

	// Remove the sent data from the start of the buffer.
	memmove(writeBuffer_, writeBuffer_ + count, writeCount_);

	return false;
}