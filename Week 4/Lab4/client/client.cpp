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
#include <thread>

#pragma comment(lib, "ws2_32.lib")


// The IP address of the server
#define SERVERIP "127.0.0.1"

// The UDP port number on the server
#define SERVERPORT 4444

// The (fixed) size of message that we send between the two programs
#define MESSAGESIZE 40


// Prototypes
void die(const char *message);


int main()
{
	printf("Client Program\n");

	std::thread *send_thread, *recv_thread;
	bool response_received = false;
	
	std::chrono::milliseconds time_track;

	std::chrono::steady_clock::time_point start = std::chrono::high_resolution_clock::now();
	std::chrono::steady_clock::time_point end = start;

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
		printf("Type some text (\"quit\" to exit): ");
		fflush(stdout);

		// Read a line of text from the user.
		std::string line;
		std::getline(std::cin, line);
		// Now "line" contains what the user typed (without the trailing \n).

		// Copy the line into the buffer, filling the rest with dashes.
		memset(buffer, '-', MESSAGESIZE);
		memcpy(buffer, line.c_str(), min(line.size(), MESSAGESIZE));

		response_received = false;
		std::chrono::steady_clock::time_point end = std::chrono::high_resolution_clock::now();
		std::chrono::steady_clock::time_point start = end;
		time_track = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
		auto dur = std::chrono::milliseconds(200);
		bool first_run = true;

		send_thread = new std::thread([&](){
			while (true){
				if (response_received){ break; }
				
				time_track = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
				if (time_track > dur || first_run)
				{
					first_run = false;
					end = std::chrono::high_resolution_clock::now();
					start = end;
					if ((rand() % 100) > 40){
						// Send the message to the server.
						if (sendto(sock, buffer, MESSAGESIZE, 0,
							(const sockaddr *)&toAddr, sizeof(toAddr)) != MESSAGESIZE)
						{
							die("sendto failed");
						}
					}
				}
				std::this_thread::sleep_for(std::chrono::milliseconds(100));

				end = std::chrono::high_resolution_clock::now();
			}
			printf("%dms passed.\n", time_track);
		});

		recv_thread = new std::thread([&](){
			// Read a response back from the server (or from anyone, in fact).
			sockaddr_in fromAddr;
			int fromAddrSize = sizeof(fromAddr);
			int count = recvfrom(sock, buffer, MESSAGESIZE, 0,
								 (sockaddr *) &fromAddr, &fromAddrSize);
			response_received = true;
			if (count < 0)
			{
				die("recvfrom failed");
			}
			if (count != MESSAGESIZE)
			{
				die("received odd-sized message");
			}

			printf("Received %d bytes from address %s port %d: '",
				   count, inet_ntoa(fromAddr.sin_addr), ntohs(fromAddr.sin_port));
			fwrite(buffer, 1, count, stdout);
			printf("'\n");
		});

		send_thread->join();
		recv_thread->join();
		
		// Keep going until we get a message starting with "quit".
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