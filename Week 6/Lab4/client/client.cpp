/*	AG0907 Lab 4 UDP client example - by Henry Fortuna and Adam Sampson

	When the user types a message, the client sends it to the server
	as a UDP packet. The server then sends a packet back to the
	client, and the client prints it out.
*/

#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <string>
#include <winsock2.h>

#pragma comment(lib, "ws2_32.lib")


// The IP address of the server
#define SERVERIP "127.0.0.1"

// The UDP port number on the server
#define SERVERPORT 4444

// The (fixed) size of message that we send between the two programs
#define MESSAGESIZE 40

// Game message
struct Message
{
	int objectID; // the ID number of the object
	int x, y; // position
};

struct Player
{
	int x, y;
};

// Prototypes
void die(const char *message);


int main()
{
	printf("Client Program\n");

	// Player & tracking info
	Player playerOne;
	playerOne.x = 20;
	playerOne.y = 30;

	int myID = 1;
	int timeTrack = 0;

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

	// Fill out a sockaddr_in structure with the address that
	// we want to send to.
	sockaddr_in toAddr;
	toAddr.sin_family = AF_INET;
	// htons converts the port number to network byte order (big-endian).
	toAddr.sin_port = htons(SERVERPORT);
	toAddr.sin_addr.s_addr = inet_addr(SERVERIP);

	// inet_ntoa formats an IP address as a string.
	printf("IP address to send to: %s\n", inet_ntoa(toAddr.sin_addr));
	// ntohs does the opposite of htons.
	printf("Port number to send to: %d\n\n", ntohs(toAddr.sin_port));

	// We'll use this buffer to hold the messages we exchange with the server.
	char buffer[MESSAGESIZE];

	do {

		fd_set readable;
		FD_ZERO(&readable);

		if (timeTrack >= 10)
		{
			playerOne.x += -10 + (std::rand() % (10 - (-10) + 1));
			playerOne.y += -10 + (std::rand() % (10 - (-10) + 1));
			timeTrack = 0;
		}

		Sleep(10000);

		// Send message to the server
		Message msg;
		msg.objectID = myID;
		msg.x = playerOne.x;
		msg.y = playerOne.y;

		sendto(sock, (const char *)&msg, sizeof(Message),
			0, (const sockaddr *)&toAddr, sizeof(toAddr));

		FD_SET(sock, &readable);

		// The structure that describes how long to wait for something to happen.
		timeval timeout;
		// We want a 2.5-second timeout.
		timeout.tv_sec = 0;
		timeout.tv_usec = 500000;

		int count = select(0, &readable, NULL, NULL, &timeout);

		if (count == SOCKET_ERROR)
		{
			die("Select failed");
		}

		if (FD_ISSET(sock, &readable))
		{
			int toAddrSize = sizeof(toAddr);

			int count = recvfrom(sock, (char *)&msg, sizeof(Message), 0,
				(sockaddr *)&toAddr, &toAddrSize);
			if (count < 0)
			{
				die("recvfrom failed");
			}
			if (count != sizeof(Message))
			{
				die("received odd-sized message");
			}

			std::cout << "Received back from server:\n";
			std::cout << "X: " << msg.x << " Y: " << msg.y << "\n";

		}
		timeTrack += 10;
	} while (memcmp(buffer, "quit", 4) != 0);

	printf("Quitting\n");

	// Close the socket and clean up the sockets library.
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