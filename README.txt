Chat Server and Client
By Rasmus Graversen & Rasmus Munksgaard-Nielsen

HOW TO USE THE SYSTEM

STEP 1:
Go to -> ProgrammingMiniP/NewServer/NewServer/Server_main.cpp
Change the (#define IP_ADDRESS "192.168.1.123") to hold your own local IPv4 address (currently, it is my own local address).

STEP 2:
Open/start the server.
Go to -> ProgrammingMiniP/NewServer/Debug/NewServer.exe
This will open a command prompt where the server will be running.
If the server started up correctly it should say: "Server running."

STEP 3:
Go to -> ProgrammingMiniP/NewClient/NewClient/Client_main.cpp
Change the (#define IP_ADDRESS "192.168.1.123") to hold your own local IPv4 address (currently, it is my own local address).

STEP 4:
Open/start the client.
Go to -> Programming/NewClient/Debug/NewClient.exe
This will open a command prompt where the client will be running.
If the client started up correctly it should say: "Successfully connected". 
Also on the server prompt it should say: "Client #(0-4) joined the chat."

STEP 4:
Open up to five client prompts and start chatting with people on your local network. 
