#include <winsock2.h>	// Allows me to work with sockets.
#include <ws2tcpip.h>	// This is to create a simple tcp server
#include <iostream>		// Because I want to use cin/cout etc..
#include <string>		// Allows me to use strings
#include <thread>		// For multithreading

using namespace std;

#pragma comment (lib, "Ws2_32.lib")		// To use the winsocket 32bit libary

#define DEFAULT_BUFLEN 512				// Buffer length
#define IP_ADDRESS "192.168.1.38"		// Here I define the local IP for the server
#define DEFAULT_PORT "54000"			// Here I define the port the server is using

struct client_type // Client type struct, holding the clients ID and which socket they are using
{
	SOCKET socket;
	int id;
	char recvMessage[DEFAULT_BUFLEN]; 
};

int process_client(client_type &new_client); //Declaration of the process function
int main();

int process_client(client_type &new_client) //Creates the function of the clients process
{
	while (1) //Initiate the while loop
	{
		memset(new_client.recvMessage, 0, DEFAULT_BUFLEN);

		if (new_client.socket != 0) //If there is a client, then it receives the new client socket with the message
		{								//with the max length of the buffer and stores it into clientResult
			int clientResult = recv(new_client.socket, new_client.recvMessage, DEFAULT_BUFLEN, 0);

			if (clientResult != SOCKET_ERROR) //If there is no error, then...
				cout << new_client.recvMessage << endl;

			else //If there is an error, then...
			{
				cout << "recv() failed: " << WSAGetLastError() << endl;
				break;
			}
		}
	}
	//End of while loop

	if (WSAGetLastError() == WSAECONNRESET) 
		cout << "The server has disconnected" << endl;

	return 0;
}

int main()
{	//This is the variables i need in order to setup the client
	WSAData wsaData;
	struct addrinfo *result = NULL, *ptr = NULL, hints;
	string sent_message = "";
	client_type client = { INVALID_SOCKET, -1, "" };
	int clientResult = 0;
	string message;

	cout << "Starting Client...\n";

	// Initialize Winsock
	clientResult = WSAStartup(MAKEWORD(2, 2), &wsaData);
	if (clientResult != 0) { //If client gets a result, then...
		cout << "WSAStartup() failed with error: " << clientResult << endl;
		return 1;
	}

	ZeroMemory(&hints, sizeof(hints));
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_protocol = IPPROTO_TCP;

	// Resolve the server address and port
	clientResult = getaddrinfo(static_cast<LPCTSTR>(IP_ADDRESS), DEFAULT_PORT, &hints, &result);
	if (clientResult != 0) { //If client gets a result, then...
		cout << "getaddrinfo() failed with error: " << clientResult << endl;
		WSACleanup();
		system("pause");
		return 1;
	}

	// Attempt to connect to an address until one succeeds
	for (ptr = result; ptr != NULL; ptr = ptr->ai_next) {

		// Create a SOCKET for connecting to server
		client.socket = socket(ptr->ai_family, ptr->ai_socktype,
			ptr->ai_protocol);
		if (client.socket == INVALID_SOCKET) { //If the socket is invalid then a small message pops up with the problem
			cout << "socket() failed with error: " << WSAGetLastError() << endl;
			WSACleanup();
			system("pause");
			return 1;
		}

		// Connect to server.
		clientResult = connect(client.socket, ptr->ai_addr, (int)ptr->ai_addrlen);
		if (clientResult == SOCKET_ERROR) { //If clientResult has a socket error, then...
			closesocket(client.socket);
			client.socket = INVALID_SOCKET;
			continue;
		}
		break;
	}

	freeaddrinfo(result);

	if (client.socket == INVALID_SOCKET) { //If client.socket is invalid, then...
		cout << "Unable to connect to server!" << endl;
		WSACleanup();
		system("pause");
		return 1;
	}

	cout << "Successfully Connected" << endl;

	//Obtain id from server for this client;
	recv(client.socket, client.recvMessage, DEFAULT_BUFLEN, 0);
	message = client.recvMessage;

	if (message != "Server is full") //If server is not full, then...
	{
		client.id = atoi(client.recvMessage);

		thread my_thread(process_client, ref(client));

		while (1) //Initiate the while loop
		{
			getline(cin, sent_message);
			clientResult = send(client.socket, sent_message.c_str(), strlen(sent_message.c_str()), 0);

			if (clientResult <= 0) //If client result in lower or equal to 0 then...
			{
				cout << "send() failed: " << WSAGetLastError() << endl;
				break;
			}
		}

		//Shutdown the connection since no more data will be sent
		my_thread.detach();
	}
	else
		cout << client.recvMessage << endl;

	cout << "Shutting down socket..." << endl;
	clientResult = shutdown(client.socket, SD_SEND);
	if (clientResult == SOCKET_ERROR) { //If client receives an error, then...
		cout << "shutdown() failed with error: " << WSAGetLastError() << endl;
		closesocket(client.socket);
		WSACleanup();
		system("pause");
		return 1;
	}

	//Closing process
	closesocket(client.socket);
	WSACleanup();
	system("pause");
	return 0;
}