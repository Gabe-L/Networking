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

	doWrite();
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
void Connection::doRead()
{
	// Note the loop here -- we must keep reading until we get a WSAEWOULDBLOCK error.
	while (true)
	{
		// Receive into whatever's left of the receive buffer.
		int spaceLeft = (sizeof readBuffer_) - readCount_;
		int count = recv(sock_, readBuffer_ + readCount_, spaceLeft, 0);
		if (count == SOCKET_ERROR)
		{
			if (WSAGetLastError() == WSAEWOULDBLOCK)
			{
				// That's as much data as we're going to get this time!
				// We need to wait for another FD_READ message.
				break;
			}
			else
			{
				// Something went wrong.
				die("recv failed");
			}
		}
		else if (count == 0)
		{
			// The server closed the connection.
			die("connection closed");
		}
		else
		{
			// We got some data!
			printf("  Received %d bytes (total %d so far)\n", count, readCount_);
			readCount_ += count;

			// Have we received a complete message?
			if (readCount_ == sizeof(NetMessage))
			{
				processMessage((const NetMessage *)readBuffer_);
				readCount_ = 0;
			}
		}
	}
}

// Call this when the socket is ready to write.
// Returns true if the socket should be closed.
void Connection::doWrite()
{
	while (writeCount_ > 0)
	{
		// Send as much data as we can.
		int count = send(sock_, writeBuffer_, writeCount_, 0);
		if (count == SOCKET_ERROR)
		{
			if (WSAGetLastError() == WSAEWOULDBLOCK)
			{
				// We've sent as much data as we can.
				// We need to wait for another FD_WRITE message.
				break;
			}
			else
			{
				// Something went wrong.
				die("send failed");
			}
		}
		else if (count == 0)
		{
			// The server closed the connection.
			die("connection closed");
		}
		else
		{
			// We sent some data!
			writeCount_ -= count;
			printf("  Sent %d bytes (%d left)\n", count, writeCount_);

			// Remove the sent data from the start of the buffer.
			memmove(writeBuffer_, writeBuffer_ + count, writeCount_);
		}
	}
}