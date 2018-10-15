/*	AG0907 Lab 4 UDP server example - by Henry Fortuna and Adam Sampson

	When the user types a message, the client sends it to the server
	as a UDP packet. The server then sends a packet back to the
	client, and the client prints it out.
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <winsock2.h>
#include <iostream>
#include <vector>

#pragma comment(lib, "ws2_32.lib")


// The IP address for the server
#define SERVERIP "127.0.0.1"

// The UDP port number for the server
#define SERVERPORT 4444

// The (fixed) size of message that we send between the two programs
#define MESSAGESIZE 40

// Game message
struct Message
{
	int objectID; // the ID number of the object
	int x, y; // position
};

// Prototypes
void die(const char *message);


int main()
{
	printf("Echo Server\n");

	std::vector<sockaddr_in*> clientConnections;

	// Initialise the WinSock library -- we want version 2.2.
	WSADATA w;
	int error = WSAStartup(0x0202, &w);
	if (error != 0)
	{
		die("WSAStartup failed");
	}
	if (w.wVersion != 0x0202)
	{
		die("Wrong WinSock version");
	}

	// Create a UDP socket.
	SOCKET sock = socket(AF_INET, SOCK_DGRAM, 0);
	if (sock == INVALID_SOCKET)
	{
		die("socket failed");
	}

	// Fill out a sockaddr_in structure to describe the address we'll listen on.
	sockaddr_in serverAddr;
	serverAddr.sin_family = AF_INET;
	serverAddr.sin_addr.s_addr = inet_addr(SERVERIP);
	// htons converts the port number to network byte order (big-endian).
	serverAddr.sin_port = htons(SERVERPORT);

	// Bind the socket to that address.
	if (bind(sock, (const sockaddr *) &serverAddr, sizeof(serverAddr)) != 0)
	{
		die("bind failed");
	}

	// ntohs does the opposite of htons.
	printf("Server socket bound to address %s, port %d\n", inet_ntoa(serverAddr.sin_addr), ntohs(serverAddr.sin_port));

	// We'll use this array to hold the messages we exchange with the client.
	char buffer[MESSAGESIZE];

	while (true)
	{
		printf("Waiting for a message...\n");

		sockaddr_in fromAddr;
		int fromAddrSize = sizeof(fromAddr);

		if (std::find(clientConnections.begin(), clientConnections.end(), fromAddr) == clientConnections.end())
		{
			clientConnections.push_back(&fromAddr);
		}

		Message msg;

		/*msg.objectID = 0;
		msg.x = 0;
		msg.y = 0;*/

		int count = recvfrom(sock, (char *)&msg, sizeof(Message), 0,
			                 (sockaddr *) &fromAddr, &fromAddrSize);
		if (count < 0)
		{
			die("recvfrom failed");
		}
		if (count != sizeof(Message))
		{
			die("received odd-sized message");
		}

		printf("Received %d bytes from address %s port %d: '",
			   count, inet_ntoa(fromAddr.sin_addr), ntohs(fromAddr.sin_port));
		//fwrite(buffer, 1, count, stdout);
		
		std::cout << msg.objectID << " location - X: " << msg.x << " Y: " << msg.y << std::endl;

		printf("'\n");

		// Send the same data back to the address it came from.

		for (auto conn : clientConnections)
		{
			if (sendto(sock, (const char *)&msg, sizeof(Message), 0,
				(const sockaddr *)&conn, sizeof(conn)) != sizeof(Message))
			{
				die("sendto failed");
			}
		}

	}

	// We won't actually get here, but if we did then we'd want to clean up...
	printf("Quitting\n");
	closesocket(sock);
	WSACleanup();
	return 0;
}


// Print an error message and exit.
void die(const char *message)
{
	fprintf(stderr, "Error: %s (WSAGetLastError() = %d)\n", message, WSAGetLastError());

#ifdef _DEBUG
	// Debug build -- drop the program into the debugger.
	abort();
#else
	exit(1);
#endif
}