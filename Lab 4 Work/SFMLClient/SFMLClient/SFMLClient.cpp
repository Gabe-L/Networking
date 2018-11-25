// TCP/UDP client using SFML

#include <SFML/Network.hpp>
#include <iostream>
#include <algorithm>
#include <thread>

// The IP address of the server to connect to
#define SERVERIP "127.0.0.1"

// The TCP port number on the server to connect to
#define SERVERPORT 4444

// The (fixed) size of message that we send between the two programs
#define MESSAGESIZE 40

// Prototypes
void tcpClient();
void udpClient();
void talk_to_server_tcp(sf::TcpSocket& socket);
void talk_to_server_udp(sf::UdpSocket& socket);
void die(const char *message);

int main()
{
	//tcpClient();

	udpClient();

	return 0;
}

// Connect to a server using TCP sockets
void tcpClient()
{
	printf("Client TCP Program\n");

	// Initialise SFML TCP socket
	sf::TcpSocket socket;
	sf::Socket::Status status = socket.connect(SERVERIP, SERVERPORT);
	if (status != sf::Socket::Done)
	{
		// error...
		die("connect failed");
	}

	// Print the IP and Port we're connecting to.
	printf("IP address to connect to: %s\n", SERVERIP);
	printf("Port number to connect to: %d\n\n", SERVERPORT);

	printf("Connected to server\n");

	talk_to_server_tcp(socket);

	printf("Quitting\n");

	// Close the socket and clean up the sockets library.
	socket.disconnect();
}

// Connect to a server using UDP sockets
void udpClient()
{
	printf("Client UDP Program\n");

	sf::UdpSocket socket;

	// Print the IP and Port we're connecting to.
	printf("IP address to send to:: %s\n", SERVERIP);
	printf("Port number to send to: %d\n\n", SERVERPORT);

	talk_to_server_udp(socket);

	// We won't actually get here, but if we did then we'd want to clean up...
	printf("Quitting\n");
	socket.unbind();
}

// Communicate with a server.
// The socket will be closed when this function returns.
void talk_to_server_tcp(sf::TcpSocket& socket)
{
	// We'll use this buffer to hold what we receive from the server.
	char buffer[MESSAGESIZE];

	while (memcmp(buffer, "quit", 4) != 0)
	{
		printf("Type some text (\"quit\" to exit): ");
		fflush(stdout);

		// Read a line of text from the user.
		std::string line;
		std::getline(std::cin, line);
		int lineSize = line.size();
		// Now "line" contains what the user typed (without the trailing \n).

		// Copy the line into the buffer, filling the rest with dashes.
		// (We must be careful not to write past the end of the array.)
		memset(buffer, '-', MESSAGESIZE);
		memcpy(buffer, line.c_str(), std::min(lineSize, MESSAGESIZE));

		if (socket.send(buffer, MESSAGESIZE) != sf::Socket::Done)
		{
			// error...
			printf("send failed\n");
		}

		// Receive data
		std::size_t received;

		// TCP socket: (blocking)
		if (socket.receive(buffer, MESSAGESIZE, received) != sf::Socket::Done)
		{
			// error...
			die("receive failed");
		}
		std::cout << "Received " << received << " bytes: '";
		fwrite(buffer, 1, MESSAGESIZE, stdout);
		printf("'\n");
	}
}

// Communicate with a server.
// The socket will be closed when this function returns.
void talk_to_server_udp(sf::UdpSocket& socket)
{
	// We'll use this buffer to hold what we receive from the server.
	//char buffer[MESSAGESIZE];

	std::string s;

	std::thread *send_thread, *recv_thread;
	bool response_received = false;

	std::chrono::milliseconds time_track;

	std::chrono::steady_clock::time_point start = std::chrono::high_resolution_clock::now();
	std::chrono::steady_clock::time_point end = start;

	//while (memcmp(buffer, "quit", 4) != 0)
	do
	{
		printf("Type some text (\"quit\" to exit): ");
		fflush(stdout);

		// Read a line of text from the user.
		sf::Packet packet;
		
		std::string line;
		std::getline(std::cin, line);
		int lineSize = line.size();
		// Now "line" contains what the user typed (without the trailing \n).

		// Set packet contents to what user typed
		packet << line;

		// Copy the line into the buffer, filling the rest with dashes.
		// (We must be careful not to write past the end of the array.)
		/*memset(buffer, '-', MESSAGESIZE);
		memcpy(buffer, line.c_str(), std::min(lineSize, MESSAGESIZE));*/

		response_received = false;
		std::chrono::steady_clock::time_point end = std::chrono::high_resolution_clock::now();
		std::chrono::steady_clock::time_point start = end;
		time_track = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
		auto dur = std::chrono::milliseconds(200);
		bool first_run = true;

		

		send_thread = new std::thread([&]()
		{
			// UDP socket:
			sf::IpAddress recipient = SERVERIP;
			unsigned short port = SERVERPORT;
			while (true)
			{
				if (response_received)
				{
					break;
				}

				time_track = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);

				if (time_track > dur || first_run)
				{
					first_run = false;
					end = std::chrono::high_resolution_clock::now();
					start = end;

					// Simulate packet drop
					// Lose 40% of packets
					if ((rand() % 100) > 40)
					{
						//if (socket.send(buffer, MESSAGESIZE, recipient, port) != sf::Socket::Done)
						//{
						//	// error...
						//	die("sendto failed");
						//}
						if (socket.send(packet, recipient, port) != sf::Socket::Done)
						{
							// error...
							die("sendto failed");
						}
					}
				}
				std::this_thread::sleep_for(std::chrono::milliseconds(100));

				end = std::chrono::high_resolution_clock::now();
			}
			printf("%dms passed.\n", time_track);
		});

		

		recv_thread = new std::thread([&]()
		{
			std::this_thread::sleep_for(std::chrono::milliseconds(10));
			// Receive data
			std::size_t received;

			// UDP socket:
			sf::IpAddress sender;
			unsigned short prt;

			//if (socket.receive(buffer, MESSAGESIZE, received, sender, prt) != sf::Socket::Done)
			//{
			//	// error...
			//	die("receive failed");
			//}
			if (socket.receive(packet, sender, prt) != sf::Socket::Done)
			{
				// error...
				die("receive failed");
			}
			if (packet >> s)
			{
				// ok
				response_received = true;
				std::cout << "Received " << packet.getDataSize() << " bytes from " << sender << " on port " << prt << std::endl;
				std::cout << "'" << s << "'" << std::endl;
			}
			else
			{
				// die
				std::cout << "packet broke yo" << std::endl;
			}
			
		});
		/*std::cout << "Received " << received << " bytes from " << sender << " on port " << port << std::endl;
		std::cout << "'";
		fwrite(buffer, 1, MESSAGESIZE, stdout);
		printf("'\n");*/

		send_thread->join();
		recv_thread->join();
	} while (s != "quit");
}

// Print an error message and exit.
void die(const char *message)
{
	fprintf(stderr, "Error: %s (WSAGetLastError() = %d)\n", message, sf::Socket::Status());

#ifdef _DEBUG
	// Debug build -- drop the program into the debugger.
	abort();
#else
	exit(1);
#endif
}