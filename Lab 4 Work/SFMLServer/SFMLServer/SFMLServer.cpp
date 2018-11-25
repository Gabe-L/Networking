// TCP/UDP server using SFML

#include <SFML/Network.hpp>
#include <iostream>

// The IP address for the server
#define SERVERIP "127.0.0.1"

// The UDP port number for the server
#define SERVERPORT 4444

// The (fixed) size of message that we send between the two programs
#define MESSAGESIZE 40

// Prototypes
void tcpServer();
void udpServer();
void talk_to_client_tcp(sf::TcpSocket& clientSocket);
void talk_to_client_udp(sf::UdpSocket& clientSocket);
void die(const char *message);

int main()
{
	//tcpServer();

	udpServer();
	
	return 0;
}

// Run a server using TCP sockets
void tcpServer()
{
	printf("Echo TCP SFML Server\n");

	// Initialise SFML TCP listener
	sf::TcpListener listener;

	// bind the listener to a port
	if (listener.listen(SERVERPORT, SERVERIP) != sf::Socket::Done)
	{
		// error...
		die("bind failed");
	}

	// Print IP and Port the server is bound to
	printf("Server socket bound to address %s, port %d\n", SERVERIP, SERVERPORT);

	printf("Server socket listening\n");

	while (true)
	{
		printf("Waiting for a connection...\n");

		// accept a new connection
		sf::TcpSocket client;
		if (listener.accept(client) != sf::Socket::Done)
		{
			// accept failed -- just try again.
			continue;
		}

		// use "client" to communicate with the connected client,
		// and continue to accept new connections with the listener
		printf("Client has connected from IP address %d, port %d!\n", client.getRemoteAddress(), client.getRemotePort());

		talk_to_client_tcp(client);

		printf("Client disconnected\n");

		// Close the connection.
		client.disconnect();
	}

	// We won't actually get here, but if we did then we'd want to clean up...
	printf("Quitting\n");
	listener.close();
}

// Run a server using UDP sockets.
void udpServer()
{
	printf("Echo UDP SFML Server\n");

	sf::UdpSocket socket;

	// bind the socket to a port
	if (socket.bind(SERVERPORT, SERVERIP) != sf::Socket::Done)
	{
		// error...
		die("bind failed");
	}

	// Print IP and Port the server is bound to
	printf("Server socket bound to address %s, port %d\n", SERVERIP, SERVERPORT);

	while (true)
	{
		printf("Waiting for a message...\n");

		talk_to_client_udp(socket);
	}

	// We won't actually get here, but if we did then we'd want to clean up...
	printf("Quitting\n");
	socket.unbind();
}

// Communicate with a client.
// The socket will be closed when this function returns.
void talk_to_client_tcp(sf::TcpSocket& clientSocket)
{
	char buffer[MESSAGESIZE];

	while (memcmp(buffer, "quit", 4) != 0)
	{
		// Receive data
		std::size_t received;

		// TCP socket: (blocking)
		if (clientSocket.receive(buffer, MESSAGESIZE, received) != sf::Socket::Done)
		{
			// error...
		}
		if (received == 0)
		{
			break;
		}

		std::cout << "Received " << received << " bytes: '";
		fwrite(buffer, 1, MESSAGESIZE, stdout);
		printf("'\n");

		// TCP socket:
		if (received > 0 && received <= MESSAGESIZE)
		{
			if (clientSocket.send(buffer, MESSAGESIZE) != sf::Socket::Done)
			{
				// error...
				printf("send failed\n");
				return;
			}
		}
	}
}

// Communicate with a client.
// The socket will be closed when this function returns.
void talk_to_client_udp(sf::UdpSocket& clientSocket)
{
	//char buffer[MESSAGESIZE];

	std::string s;

	//while (memcmp(buffer, "quit", 4) != 0)
	while (s != "quit")
	{
		// Receive data
		sf::Packet packet;
		
		std::size_t received;

		// UDP socket:
		sf::IpAddress sender;
		unsigned short port;
		//if (clientSocket.receive(buffer, MESSAGESIZE, received, sender, port) != sf::Socket::Done)
		//{
		//	// error...
		//	die("receive failed");
		//}
		if (clientSocket.receive(packet, sender, port) != sf::Socket::Done)
		{
			// error...
			die("receive failed");
		}
		/*if (received == 0)
		{
			break;
		}*/
		if (packet >> s)
		{
			// ok
			std::cout << "Received " << packet.getDataSize() << " bytes from " << sender << " on port " << port << std::endl;
			std::cout << "'" << s << "'" << std::endl;
		}
		else
		{
			// die
		}

		/*std::cout << "Received " << received << " bytes from " << sender << " on port " << port << std::endl;
		std::cout << "'";
		fwrite(buffer, 1, MESSAGESIZE, stdout);
		printf("'\n");*/

		// UDP socket:
		sf::IpAddress recipient = SERVERIP;
//		if (received > 0 && received <= MESSAGESIZE)
		{
			//if (clientSocket.send(buffer, MESSAGESIZE, recipient, port) != sf::Socket::Done)
			//{
			//	// error...
			//	die("sendto failed");
			//}
			if (clientSocket.send(packet, recipient, port) != sf::Socket::Done)
			{
				// error...
				die("sendto failed");
			}
		}
	}
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