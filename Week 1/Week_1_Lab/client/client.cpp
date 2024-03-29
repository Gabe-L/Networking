/*	AG0907 Lab 2 TCP client example - by Henry Fortuna and Adam Sampson

	A simple client that connects to a server and waits for
	a response. The server sends "hello" when the client first
	connects. Text typed is then sent to the server which echos
	it back, and the response is printed out.
*/

#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <string>
#include <winsock2.h>

#pragma comment(lib, "ws2_32.lib")


// The IP address of the server to connect to
#define SERVERIP "127.0.0.1"

// The TCP port number on the server to connect to
#define SERVERPORT 5555

// The message the server will send when the client connects
#define WELCOME "hello"

// The (fixed) size of message that we send between the two programs
#define MESSAGESIZE 40


// Prototypes
void die(const char *message);


int main()
{
	printf("Client Program\n");

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

	// Create a TCP socket that we'll connect to the server
	SOCKET sock = socket(AF_INET, SOCK_STREAM, 0);

	if (sock == INVALID_SOCKET){
		die("Invalid socket");
	}

	// FIXME: check for errors from socket //FIXED

	// Fill out a sockaddr_in structure with the address that
	// we want to connect to.
	sockaddr_in addr;
	addr.sin_family = AF_INET;
	// htons converts the port number to network byte order (big-endian).
	addr.sin_port = htons(SERVERPORT);
	addr.sin_addr.s_addr = inet_addr(SERVERIP);
	
	// inet_ntoa formats an IP address as a string.
	printf("IP address to connect to: %s\n", inet_ntoa(addr.sin_addr));
	// ntohs does the opposite of htons.
	printf("Port number to connect to: %d\n\n", ntohs(addr.sin_port));

	while (true){
		// Connect the socket to the server.
		if (connect(sock, (const sockaddr *)&addr, sizeof addr) == SOCKET_ERROR)
		{
			printf("Connect failed, retrying...\n");
		}
		else{
			printf("Connected to server\n");
			break;
		}
	}

	// We'll use this buffer to hold what we receive from the server.
	char buffer[MESSAGESIZE];

	// We expect the server to send us a welcome message (WELCOME) when we connect.

	// Receive a message.
	int count = recv(sock, buffer, MESSAGESIZE, 0);

	if (count == SOCKET_ERROR){
		die("Recv failed");
	}

	// FIXME: check for errors, or for unexpected message size // FIXED

	// Check it's what we expected.
	if (memcmp(buffer, WELCOME, strlen(WELCOME)) != 0)
	{
		die("Expected \"" WELCOME "\" upon connection, but got something else");
	}

	while (true)
	{
		printf("Type some text (\"quit\" to exit): ");
		fflush(stdout);

		// Read a line of text from the user.
		std::string line;
		std::getline(std::cin, line);
		// Now "line" contains what the user typed (without the trailing \n).

		// Copy the line into the buffer, filling the rest with dashes.
		memset(buffer, '-', MESSAGESIZE);
		if (line.size() > MESSAGESIZE){
			printf("Message too large, cancelling...\n");
			continue;
		}
		memcpy(buffer, line.c_str(), line.size());
		// FIXME: if line.size() is bigger than the buffer it'll overflow (and likely corrupt memory) // FIXED

		// Send the message to the server.
		if (send(sock, buffer, MESSAGESIZE, 0) == SOCKET_ERROR){
			printf("Send failed...\n");
			continue;
		}
		// FIXME: check for error from send // FIXED

		// Read a response back from the server.
		int count = recv(sock, buffer, MESSAGESIZE, 0);

		if (count == SOCKET_ERROR){
			printf("Recv failed..\n");
			continue;
		}

		// FIXME: check for error from recv // Fixed
		if (count <= 0)
		{
			printf("Server closed connection\n");
			break;
		}

		printf("Received %d bytes from the server: '", count);
		fwrite(buffer, 1, count, stdout);
		printf("'\n");
	}

	printf("Quitting\n");

	// Close the socket and clean up the sockets library.
	closesocket(sock);
	WSACleanup();

	return 0;
}


// Print an error message and exit.
void die(const char *message) {
	fprintf(stderr, "Error: %s (WSAGetLastError() = %d)\n", message, WSAGetLastError());

#ifdef _DEBUG
	// Debug build -- drop the program into the debugger.
	abort();
#else
	exit(1);
#endif
}