/*	Event-based server example - by Henry Fortuna and Adam Sampson

	This works the same way as Lab 5's select-based server (initially).
	When a client connects, it gets sent a welcome message; otherwise,
	the server just prints out any messages it receives.
*/

#define WIN32_LEAN_AND_MEAN
#include <WinSock2.h>
#include <WS2tcpip.h>
#include <Windows.h>
#include <iostream>
#include <list>
#include <stdio.h>

#include "connection.h"
#include "protocol.h"
#include "utils.h"

#pragma comment(lib, "ws2_32.lib")


// The IP address for the server to listen on
#define SERVERIP "127.0.0.1"

// The TCP port number for the server to listen on
#define SERVERPORT "5555"

#define WM_SOCKET (WM_USER + 1)

HWND window;


void registerWindowClass(HINSTANCE hInstance);
void openWindow(HINSTANCE hInstance, int nCmdShow);
LRESULT CALLBACK windowProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);


// Entry point for the program.
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, PSTR szCmdLine, int nCmdShow)
{
	openConsoleWindow();
	startWinSock();

	printf("Server starting\n");

	// Translate SERVERIP/SERVERPORT above into an address structure.
	// (This is a more flexible equivalent of inet_aton etc. that we've used previously.)
	struct addrinfo *ai = nullptr;
	if (getaddrinfo(SERVERIP, SERVERPORT, nullptr, &ai) != 0)
	{
		die("getaddrinfo failed");
	}
	if (ai->ai_family != AF_INET) {
		die("getaddinfo didn't return an AF_INET address");
	}

	// Create a TCP socket that we'll use to listen for connections.
	SOCKET serverSocket = socket(AF_INET, SOCK_STREAM, 0);
	if (serverSocket == INVALID_SOCKET)
	{
		die("socket failed");
	}

	// Bind the server socket to its address.
	if (bind(serverSocket, ai->ai_addr, ai->ai_addrlen) != 0)
	{
		die("bind failed");
	}

	// Make the socket listen for connections.
	if (listen(serverSocket, 1) != 0)
	{
		die("listen failed");
	}

	printf("Server socket listening\n");

	// The list of clients currently connected to the server.
	std::list<Connection *> conns;

	// Accept a new connection to the server socket.
			// This gives us back a new socket connected to the client, and
			// also fills in an address structure with the client's address.
	sockaddr_in clientAddr;
	int addrSize = sizeof(clientAddr);
	SOCKET clientSocket = accept(serverSocket, (sockaddr *)&clientAddr, &addrSize);
	if (clientSocket == INVALID_SOCKET)
	{
		printf("accept failed\n");
	}

	// Create a new Connection object, and add it to the collection.
	Connection *conn = new Connection(clientSocket);
	conns.push_back(conn);

	// Send the new client a welcome message.
	NetMessage message;
	message.type = MT_WELCOME;
	conn->sendMessage(&message);

	registerWindowClass(hInstance);
	openWindow(hInstance, nCmdShow);

	// Enable WinSock events for the socket.
	// (This also puts the socket into non-blocking mode.)
	if (WSAAsyncSelect(clientSocket, window, WM_SOCKET, FD_CLOSE | FD_CONNECT | FD_READ) == SOCKET_ERROR)
	{
		die("WSAAsyncSelect failed");
	}


	//-------------------------------------------------------------------------------

	// The server's main loop, where we'll wait for new connections to
	// come in, or for new data to appear on our existing connections.
	//while (true)
	//{
	//	// The structures that describe the set of sockets we're interested in.
	//	fd_set readable, writeable;
	//	FD_ZERO(&readable);
	//	FD_ZERO(&writeable);

	//	// Add the server socket, which will become "readable" if there's a new
	//	// connection to accept.
	//	FD_SET(serverSocket, &readable);

	//	// Add all of the connected clients' sockets.
	//	for (auto conn : conns)
	//	{
	//		if (conn->wantRead())
	//		{
	//			FD_SET(conn->sock(), &readable);
	//		}
	//		if (conn->wantWrite())
	//		{
	//			FD_SET(conn->sock(), &writeable);
	//		}
	//	}

	//	// The structure that describes how long to wait for something to happen.
	//	timeval timeout;
	//	// We want a 2.5-second timeout.
	//	timeout.tv_sec = 2;
	//	timeout.tv_usec = 500000;

	//	// Wait for one of the sockets to become readable.
	//	// (We can only get away with passing 0 for the first argument here because
	//	// we're on Windows -- other sockets implementations need a proper value there.)
	//	int count = select(0, &readable, &writeable, nullptr, &timeout);
	//	if (count == SOCKET_ERROR)
	//	{
	//		die("select failed");
	//	}
	//	printf("%d clients; %d sockets are ready\n", conns.size(), count);
	//	// readable now tells us which sockets are ready.
	//	// If count == 0 (i.e. no sockets are ready) then the timeout occurred.

	//	// Is there a new connection to accept?
	//	if (FD_ISSET(serverSocket, &readable))
	//	{
	//		// Accept a new connection to the server socket.
	//		// This gives us back a new socket connected to the client, and
	//		// also fills in an address structure with the client's address.
	//		sockaddr_in clientAddr;
	//		int addrSize = sizeof(clientAddr);
	//		SOCKET clientSocket = accept(serverSocket, (sockaddr *)&clientAddr, &addrSize);
	//		if (clientSocket == INVALID_SOCKET)
	//		{
	//			printf("accept failed\n");
	//			continue;
	//		}

	//		// Create a new Connection object, and add it to the collection.
	//		Connection *conn = new Connection(clientSocket);
	//		conns.push_back(conn);

	//		// Send the new client a welcome message.
	//		NetMessage message;
	//		message.type = MT_WELCOME;
	//		conn->sendMessage(&message);
	//	}

	//	// Check each of the clients.
	//	for (auto it = conns.begin(); it != conns.end(); )  // note no ++it here
	//	{
	//		Connection *conn = *it;
	//		bool dead = false;

	//		// Is there data to read from this client's socket?
	//		if (FD_ISSET(conn->sock(), &readable))
	//		{
	//			dead |= conn->doRead();
	//		}

	//		// Can we write to this client's socket?
	//		if (FD_ISSET(conn->sock(), &writeable))
	//		{
	//			dead |= conn->doWrite();
	//		}

	//		if (dead)
	//		{
	//			// The client said it was dead -- so free the object,
	//			// and remove it from the conns list.
	//			delete conn;
	//			it = conns.erase(it);
	//		}
	//		else
	//		{
	//			++it;
	//		}
	//	}
	//}

	// We won't actually get here, but if we did then we'd want to clean up...
	printf("Quitting\n");
	closesocket(serverSocket);
	WSACleanup();
	return 0;
}


void registerWindowClass(HINSTANCE hInstance)
{
	WNDCLASSEX wcex;

	wcex.cbSize = sizeof(wcex);
	wcex.style = CS_HREDRAW | CS_VREDRAW;
	// Use our window procedure to handle messages for this kind of window.
	wcex.lpfnWndProc = windowProc;
	wcex.cbClsExtra = 0;
	wcex.cbWndExtra = 0;
	wcex.hInstance = hInstance;
	wcex.hIcon = 0;
	wcex.hCursor = LoadCursor(nullptr, IDC_ARROW);

	wcex.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
	wcex.lpszMenuName = nullptr;
	wcex.lpszClassName = L"WindowClass";
	wcex.hIconSm = 0;

	RegisterClassEx(&wcex);
}

// Create and open our window.
void openWindow(HINSTANCE hInstance, int nCmdShow)
{
	window = CreateWindow(L"WindowClass",
		L"Client",
		WS_OVERLAPPEDWINDOW,
		600, 600,
		400, 200,
		nullptr,
		nullptr,
		hInstance,
		nullptr);
	if (!window)
	{
		die("CreateWindow failed");
	}
	ShowWindow(window, nCmdShow);
	UpdateWindow(window);
}

// Process messages for our window.
LRESULT CALLBACK windowProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	switch (message)
	{
	case WM_SOCKET:
		// A socket event message.
		// We can look at wParam to figure out which socket it was (but we only have one here anyway).
		printf("Event on socket %ld\n", (long)wParam);

		long error = WSAGETSELECTERROR(lParam);
		if (error != 0)
		{
			// Something went wrong with one of the asynchronous operations.
			fprintf(stderr, "WM_SOCKET error %ld on socket %ld\n", error, (long)wParam);
			die("asynchronous socket operation failed");
		}

		// What kind of event was it?
		switch (WSAGETSELECTEVENT(lParam))
		{
		case FD_ACCEPT:
			printf(" FD_ACCEPT\n");
		case FD_CONNECT:
			// connect() completed.
			printf("  FD_CONNECT\n");
			//connected = true;
			break;

		case FD_READ:
			// It may be possible to receive.
			printf("  FD_READ\n");
			//tryToRead();
			break;

		case FD_WRITE:
			// It may be possible to send.
			// We will only get this notification if we've already tried to send
			// and been told that it would block (which is different from select's behaviour).
			printf("  FD_WRITE\n");
			//tryToWrite();
			break;
		}

		break;
	}

	// Pass messages down to the default handler.
	return DefWindowProc(hWnd, message, wParam, lParam);
}