#include <iostream>			// Because I want to use cin/cout etc..
#include <winsock2.h>		// Allows me to work with sockets.
#include <ws2tcpip.h>		// This is to create a simple tcp server
#include <string>			// Allows me to use strings
#include <thread>			// For multithreading
#include <vector>			// For construction of containers

#pragma comment (lib, "Ws2_32.lib")		// To use the winsocket 32bit libary

#define IP_ADDRESS "192.168.1.38"  // Here I define the local IP for the server
#define DEFAULT_PORT "54000"		// Here I define the port the server is using
#define DEFAULT_BUFLEN 512			// Buffer length

struct client_type			// Client type struct, holding the clients ID and which socket they are using
{
	int id;					
	SOCKET socket;
};

const char OPTION_VALUE = 1;	// Constant char variable
const int MAX_CLIENTS = 5;		// Maximum amount of clients that can be on the server

//Creating the function process_client 
int process_client(client_type &new_client, std::vector<client_type> &client_array, std::thread &thread)
{
	std::string msg = "";		// String variable equal to nothing
	char tempmsg[DEFAULT_BUFLEN] = "";	// Same here, just a char with the length of the buffer

	//While loop
	while (1)
	{
		memset(tempmsg, 0, DEFAULT_BUFLEN);

		if (new_client.socket != 0) // if there is a new client, then it's not 0,
		{							// then it receives the new client socket with the message with the max length of the buffer, and stores it into clientResult
			int clientResult = recv(new_client.socket, tempmsg, DEFAULT_BUFLEN, 0);

			if (clientResult != SOCKET_ERROR) // If there is no error then:
			{
				if (strcmp("", tempmsg)) // Compares the tempmsg to nothing
					msg = "Client #" + std::to_string(new_client.id) + ": " + tempmsg; // creates the message that the client writes, and gives them the name client# and their ID.

				std::cout << msg.c_str() << std::endl; //prints the message

				//Sens the message to the other clients currently on.
				for (int i = 0; i < MAX_CLIENTS; i++)
				{
					if (client_array[i].socket != INVALID_SOCKET) // if the client is not an invalid socket -
						if (new_client.id != i)		// - then if the client's id is not equal to i, then it shows the message.
							clientResult = send(client_array[i].socket, msg.c_str(), strlen(msg.c_str()), 0);
				}	// - this means that if the clients id doesn't match the client who send the message, then the client can see it
			}
			else
			{		// Else if there is an error, then the client disconnected
				msg = "Client #" + std::to_string(new_client.id) + " disconnected";

				std::cout << msg << std::endl; 

				//closes the socket of the disconnected client
				closesocket(new_client.socket);
				closesocket(client_array[new_client.id].socket);
				client_array[new_client.id].socket = INVALID_SOCKET;

				//Sends the disconnection message to the other clients
				for (int i = 0; i < MAX_CLIENTS; i++)
				{
					if (client_array[i].socket != INVALID_SOCKET)
						clientResult = send(client_array[i].socket, msg.c_str(), strlen(msg.c_str()), 0);
				}	
					
				break;
			}
		}
	} //end of while loop

	thread.detach();

	return 0;
}

int main()
{	// These are all data and variables that I need to set up the server and sockets.
	WSADATA wsaData;		
	struct addrinfo hints;
	struct addrinfo *server = NULL;
	SOCKET server_socket = INVALID_SOCKET;
	std::string msg = "";
	std::vector<client_type> client(MAX_CLIENTS);
	int num_clients = 0;
	int temp_id = -1;
	std::thread my_thread[MAX_CLIENTS];

	//Initializing Winsock
	WSAStartup(MAKEWORD(2, 2), &wsaData); //Starting up winsock, using version 2.2

	//Setting up hints
	ZeroMemory(&hints, sizeof(hints));
	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_protocol = IPPROTO_TCP;
	hints.ai_flags = AI_PASSIVE;

	//Setting up the Server. Where it takes converts the IP_ADDRESS, since it is defined at the start as a string. 
	//it also takes the port and information about the server.
	getaddrinfo(static_cast<LPCTSTR>(IP_ADDRESS), DEFAULT_PORT, &hints, &server);

	//Creating a listening socket for connecting to the server
	server_socket = socket(server->ai_family, server->ai_socktype, server->ai_protocol);

	//Giving an address to the server socket.
	bind(server_socket, server->ai_addr, (int)server->ai_addrlen);

	//Listening for incoming connections, with a large queue length
	listen(server_socket, SOMAXCONN);

	//Tell the operator of the server, that the server is running. simple.
	std::cout << "Server Running." << std::endl;

	//Initializing the client list
	for (int i = 0; i < MAX_CLIENTS; i++)
	{
		client[i] = { -1, INVALID_SOCKET };
	}

	while (1) //while loop
	{
		// Creates a socket variable
		SOCKET incoming = INVALID_SOCKET; 
		//It is then set equal to an accept function, that takes the server_socket which is the client connecting.
		incoming = accept(server_socket, NULL, NULL); 

		//If the connection is an error, the program should just continue waiting for other connections, instead of breaking.
		if (incoming == INVALID_SOCKET) continue; 

		//Reset the number of clients
		num_clients = -1;

		//Create a temporary id for the next client
		temp_id = -1;
		for (int i = 0; i < MAX_CLIENTS; i++)
		{
			if (client[i].socket == INVALID_SOCKET && temp_id == -1)
			{
				client[i].socket = incoming;
				client[i].id = i;
				temp_id = i;
			}

			if (client[i].socket != INVALID_SOCKET)
				num_clients++;
		}

		if (temp_id != -1)
		{
			//Send the id to that client
			std::cout << "Client #" << client[temp_id].id << " joined the chat." << std::endl;
			msg = std::to_string(client[temp_id].id);
			send(client[temp_id].socket, msg.c_str(), strlen(msg.c_str()), 0);

			//Create a thread process for that client
			my_thread[temp_id] = std::thread(process_client, std::ref(client[temp_id]), std::ref(client), std::ref(my_thread[temp_id]));
		}
		else
		{
			//This is if the max client is exceeded, then it should not take any more clients.
			msg = "Server is full";
			send(incoming, msg.c_str(), strlen(msg.c_str()), 0);
			std::cout << msg << std::endl;
		}
	} //end while loop


	//At this stage the server is not running anymore so the I close down the sockets and clean up winsock.

	//Close listening socket
	closesocket(server_socket);

	//Close client socket
	for (int i = 0; i < MAX_CLIENTS; i++)
	{
		my_thread[i].detach();
		closesocket(client[i].socket);
	}

	//Clean up Winsock
	WSACleanup();

	system("pause");
	return 0;
}