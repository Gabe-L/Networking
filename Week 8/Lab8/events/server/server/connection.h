/* The Connection class */

#ifndef CONNECTION_H
#define CONNECTION_H

#include <WinSock2.h>

#include "protocol.h"

// Each instance of this class represents a connected client.
class Connection {
public:
	// Constructor.
	// sock: the socket that we've accepted the client connection on.
	Connection(SOCKET sock);

	// Destructor.
	~Connection();

	// Process an incoming message.
	void processMessage(const NetMessage *message);

	// Add an outgoing message to the send buffer.
	void sendMessage(const NetMessage *message);

	// Return the client's socket.
	SOCKET sock();

	// Return whether this connection is in a state where we want to try
	// reading from the socket.
	bool wantRead();

	// Return whether this connection is in a state where we want to try
	// writing to the socket.
	bool wantWrite();

	// Call this when the socket is ready to read.
	// Returns true if the socket should be closed.
	void doRead();

	// Call this when the socket is ready to write.
	// Returns true if the socket should be closed.
	void doWrite();

private:
	// This client's socket.
	SOCKET sock_;

	// The data we've read from the client.
	char readBuffer_[sizeof NetMessage];
	int readCount_;

	// Data we need to send to the client.
	char writeBuffer_[100 * sizeof NetMessage];
	int writeCount_;
};

#endif