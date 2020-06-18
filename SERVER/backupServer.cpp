	#include <iostream>
#include <thread>
#include <mutex>
#include <chrono>
#include <vector>
#include <random>
using namespace std;

#define NWLD 10

#define DL this_thread::sleep_for(chrono::microseconds(NWLD));
#define SAY cout << "Some lol";

mutex sync_mutex;

int64_t iter = INT64_MIN;

void SendRecvSomeFunc() {
	while (true) {
		DL
			lock_guard<mutex> guard(sync_mutex);
		cout << "FIRST\t" << ++iter << endl;
	}
}

void EraseSomeFunc() {
	while (true) {
		DL
			lock_guard<mutex> guard(sync_mutex);
		cout << "SECOND\t" << ++iter << endl;
	}
}


int func() {
	return 5;
}

int main()
{
	thread thF = thread(SendRecvSomeFunc);
	thread thS = thread(EraseSomeFunc);
	thF.join();
	thS.join();
}
#pragma comment(lib, "ws2_32.lib")
#include <winsock2.h>
#include <iostream>
#include <string>
#define _WINSOCK_DEPRECATED_NO_WARNINGS
#pragma warning(disable: 4996)
SOCKET Connection;

enum Packet {
	P_ChatMessage,
	P_Test
};

bool ProcessPacket(Packet packettype) {
	switch (packettype) {
	case P_ChatMessage:
	{
		int msg_size;
		recv(Connection, (char*)&msg_size, sizeof(int), NULL);
		char* msg = new char[msg_size + 1];
		msg[msg_size] = '\0';
		recv(Connection, msg, msg_size, NULL);
		std::cout << msg << std::endl;
		delete[] msg;
		break;
	}
	case P_Test:
		std::cout << "Test packet.\n";
		break;
	default:
		std::cout << "Unrecognized packet: " << packettype << std::endl;
		break;
	}

	return true;
}

void ClientHandler() {
	Packet packettype;
	while (true) {
		recv(Connection, (char*)&packettype, sizeof(Packet), NULL);

		if (!ProcessPacket(packettype)) {
			break;
		}
	}
	closesocket(Connection);
}

int main(int argc, char* argv[]) {
	//WSAStartup
	WSAData wsaData;
	WORD DLLVersion = MAKEWORD(2, 1);
	if (WSAStartup(DLLVersion, &wsaData) != 0) {
		std::cout << "Error" << std::endl;
		exit(1);
	}

	SOCKADDR_IN addr;
	int sizeofaddr = sizeof(addr);
	addr.sin_addr.s_addr = inet_addr("127.0.0.1");
	addr.sin_port = htons(1111);
	addr.sin_family = AF_INET;

	Connection = socket(AF_INET, SOCK_STREAM, NULL);
	if (connect(Connection, (SOCKADDR*)&addr, sizeof(addr)) != 0) {
		std::cout << "Error: failed connect to server.\n";
		return 1;
	}
	std::cout << "Connected!\n";

	CreateThread(NULL, NULL, (LPTHREAD_START_ROUTINE)ClientHandler, NULL, NULL, NULL);

	std::string msg1;
	while (true) {
		std::getline(std::cin, msg1);
		int msg_size = msg1.size();
		Packet packettype = P_ChatMessage;
		send(Connection, (char*)&packettype, sizeof(Packet), NULL);
		send(Connection, (char*)&msg_size, sizeof(int), NULL);
		send(Connection, msg1.c_str(), msg_size, NULL);
		Sleep(10);
	}

	system("pause");
	return 0;
}
	#pragma once
#include <chrono>
#include <mutex>
#include <thread>
#include<iostream>
#include <winsock2.h>
#include <iostream>
#include <windows.h>
#include <WS2tcpip.h>
#include "CServer.cpp"
#include<cstring>
#pragma comment(lib, "ws2_32.lib")
using std::cout;
struct ConfigServer {
	const char* Port;
	int MaxPlayer;
};
struct SClient {
public:
	char Buffer[512];
	sockaddr_in CAddr;
	socklen_t CAddrlen;
	SOCKET ClientSocket;

	SClient() {
		CAddrlen = sizeof(CAddr);
	}

};
class CServer {

	ConfigServer ServerConfig;
	addrinfo* ServerAddr = NULL;
	WSADATA WsaData;
	SOCKET MainSocket;
	SClient Clients[4];

	struct {
		bool TrigerHandle = true;
	}Triggers;

public:

	int InitServer(ConfigServer ServerConfig) {

		int initResult = 0;
		this->ServerConfig = ServerConfig;
		//std::cout<< this->ServerConfig.MaxPlayer;
		addrinfo  ServerAddrh;

		ZeroMemory(&ServerAddrh, sizeof(ServerAddrh));

		ServerAddrh.ai_family = AF_INET;
		ServerAddrh.ai_socktype = SOCK_STREAM;
		ServerAddrh.ai_protocol = IPPROTO_TCP;
		ServerAddrh.ai_flags = AI_PASSIVE;

		if (WSAStartup(MAKEWORD(2, 2), &this->WsaData)) {
			std::cout << "Error" << std::endl;
			exit(1);
			WSACleanup();
		}

		initResult = getaddrinfo(NULL, this->ServerConfig.Port, &ServerAddrh, &(this->ServerAddr));
		if (initResult != NULL) {
			cout << "getaddrinfo failed: %d: " << initResult << "\n";
			WSACleanup();
			return 1;
		}

		this->MainSocket = socket(this->ServerAddr->ai_family, this->ServerAddr->ai_socktype, this->ServerAddr->ai_protocol);
		if (this->MainSocket == INVALID_SOCKET) {
			cout << "Server socket failed with error: %ld" << WSAGetLastError() << '\n';
			freeaddrinfo(this->ServerAddr);
			WSACleanup();
			return 1;
		}

		initResult = bind(this->MainSocket, this->ServerAddr->ai_addr, this->ServerAddr->ai_addrlen);
		if (initResult == SOCKET_ERROR) {
			cout << "bind failed with error:" << WSAGetLastError() << "\n";
			freeaddrinfo(this->ServerAddr);
			closesocket(this->MainSocket);
			WSACleanup();
			return 1;
		}

		return EXIT_SUCCESS;

	}
	int StartServer() {

		int initResult = 0;
		initResult = listen(this->MainSocket, this->ServerConfig.MaxPlayer * 2);
		if (initResult == SOCKET_ERROR) {
			cout << "listen failed with error:" << WSAGetLastError() << "\n";
			freeaddrinfo(this->ServerAddr);
			closesocket(this->MainSocket);
			WSACleanup();
			return 1;

		}
		std::thread AcceptThread = std::thread(&CServer::AcceptConnections, this);
		AcceptThread.detach();
		return 0;

	}
	void HandleConnections() {

		while (this->Triggers.TrigerHandle) {
			for (int i = 0; i < this->ServerConfig.MaxPlayer; i++) {

				int byteslenrecv;
				byteslenrecv = recv(this->Clients[i].ClientSocket, this->Clients[i].Buffer, sizeof(this->Clients[i].Buffer), NULL);
				if ((byteslenrecv == SOCKET_ERROR) or (byteslenrecv == 0)) {
					//cout << "recv Error\n";
					strcpy_s(this->Clients[i].Buffer, "ClientBuffer0");
					
				}
				cout << "Hey it's client buffer: " << this->Clients[i].Buffer << "\n";
				for (int j = 0; j < this->ServerConfig.MaxPlayer; j++) {
					if (j == i) continue;
					send(this->Clients[j].ClientSocket, this->Clients[i].Buffer, byteslenrecv, NULL);

				}

			}

		}
	}
	void AcceptConnections() {

		//std::cout << "main sokcet: " << this->MainSocket<< "\n";
		for (int i = 0; i < this->ServerConfig.MaxPlayer; i++) {
			//std::cout << "i: "<<i << "\n";			
			this->Clients[i].ClientSocket = accept(this->MainSocket, (sockaddr*)&this->Clients[i].CAddr, &this->Clients[i].CAddrlen);
			//std::cout << 1 << "\n";
			if (this->Clients[i].ClientSocket == INVALID_SOCKET) {
				cout << "Client socket failed with error: %ld\n" << WSAGetLastError() << "\n";
				closesocket(Clients[i].ClientSocket);
			}
			else {
				cout << "Client Connected!\n";
				cout << "Client Connected!\n";
				cout << "Client Connected!\n";
				cout << "Client Connected!\n";
				cout << "Client Connected!\n";
				cout << "Client Connected!\n";
				cout << "Client Connected!\n";
				std::thread HandleThread = std::thread(&CServer::HandleConnections, this);
				HandleThread.detach();

			}


		}

	}
	void StopServer() {
		Triggers.TrigerHandle = false;
		for (int i = 0; i < this->ServerConfig.MaxPlayer; i++) {

			closesocket(this->Clients[i].ClientSocket);
		}
		closesocket(MainSocket);
	}
	void RestartServer(ConfigServer ServerConfig) {
		Triggers.TrigerHandle = false;
		for (int i = 0; i < this->ServerConfig.MaxPlayer; i++) {
			closesocket(this->Clients[i].ClientSocket);
			memset(&this->Clients[i], 0, sizeof(Clients[i]));
			memset(&this->Clients[i].CAddr, 0, sizeof(Clients[i].CAddr));
			this->Clients[i].Buffer[512]=' ';			
			this->Clients[i].CAddrlen=sizeof(this->Clients[i].CAddr);WSACleanup();
		}
		
		ServerAddr = NULL;
		closesocket(this->MainSocket);
		InitServer(ServerConfig);
		StartServer();
	}
};

	#undef UNICODE
		
		#define WIN32_LEAN_AND_MEAN		
		#include <windows.h>
		#include <winsock2.h>
		#include <ws2tcpip.h>
		#include <stdlib.h>
		#include <stdio.h>
		
		// Need to link with Ws2_32.lib
		#pragma comment (lib, "Ws2_32.lib")
		// #pragma comment (lib, "Mswsock.lib")
		
		#define DEFAULT_BUFLEN 512
		#define DEFAULT_PORT "27015"
		
		int __cdecl main(void)
		{
		    WSADATA wsaData;
		    int iResult;
		
		    SOCKET ListenSocket = INVALID_SOCKET;
		    SOCKET ClientSocket = INVALID_SOCKET;
		
		    struct addrinfo* result = NULL;
		    struct addrinfo hints;
		
		    int iSendResult;
		    char recvbuf[DEFAULT_BUFLEN];
		    int recvbuflen = DEFAULT_BUFLEN;
		
		    // Initialize Winsock
		    iResult = WSAStartup(MAKEWORD(2, 2), &wsaData);
		    if (iResult != 0) {
		        printf("WSAStartup failed with error: %d\n", iResult);
		        return 1;
		    }
		
		    ZeroMemory(&hints, sizeof(hints));
		    hints.ai_family = AF_INET;
		    hints.ai_socktype = SOCK_STREAM;
		    hints.ai_protocol = IPPROTO_TCP;
		    hints.ai_flags = AI_PASSIVE;
		
		    // Resolve the server address and port
		    iResult = getaddrinfo(NULL, DEFAULT_PORT, &hints, &result);
		    if (iResult != 0) {
		        printf("getaddrinfo failed with error: %d\n", iResult);
		        WSACleanup();
		        return 1;
		    }
		
		    // Create a SOCKET for connecting to server
		    ListenSocket = socket(result->ai_family, result->ai_socktype, result->ai_protocol);
		    if (ListenSocket == INVALID_SOCKET) {
		        printf("socket failed with error: %ld\n", WSAGetLastError());
		        freeaddrinfo(result);
		        WSACleanup();
		        return 1;
		    }
		
		    // Setup the TCP listening socket
		    iResult = bind(ListenSocket, result->ai_addr, (int)result->ai_addrlen);
		    if (iResult == SOCKET_ERROR) {
		        printf("bind failed with error: %d\n", WSAGetLastError());
		        freeaddrinfo(result);
		        closesocket(ListenSocket);
		        WSACleanup();
		        return 1;
		    }
		
		    freeaddrinfo(result);
		
		    iResult = listen(ListenSocket, SOMAXCONN);
		    if (iResult == SOCKET_ERROR) {
		        printf("listen failed with error: %d\n", WSAGetLastError());
		        closesocket(ListenSocket);
		        WSACleanup();
		        return 1;
		    }
		
		    // Accept a client socket
		    ClientSocket = accept(ListenSocket, NULL, NULL);
		    if (ClientSocket == INVALID_SOCKET) {
		        printf("accept failed with error: %d\n", WSAGetLastError());
		        closesocket(ListenSocket);
		        WSACleanup();
		        return 1;
		    }
		
		    // No longer need server socket
		    closesocket(ListenSocket);
		
		    // Receive until the peer shuts down the connection
		    do {
		
		        iResult = recv(ClientSocket, recvbuf, recvbuflen, 0);
		        if (iResult > 0) {
		            printf("Bytes received: %d\n", iResult);
		
		            // Echo the buffer back to the sender
		            iSendResult = send(ClientSocket, recvbuf, iResult, 0);
		            if (iSendResult == SOCKET_ERROR) {
		                printf("send failed with error: %d\n", WSAGetLastError());
		                closesocket(ClientSocket);
		                WSACleanup();
		                return 1;
		            }
		            printf("Bytes sent: %d\n", iSendResult);
		        }
		        else if (iResult == 0)
		            printf("Connection closing...\n");
		        else {
		            printf("recv failed with error: %d\n", WSAGetLastError());
		            closesocket(ClientSocket);
		            WSACleanup();
		            return 1;
		        }
		
		    } while (iResult > 0);
		
		    // shutdown the connection since we're done
		    iResult = shutdown(ClientSocket, SD_SEND);
		    if (iResult == SOCKET_ERROR) {
		        printf("shutdown failed with error: %d\n", WSAGetLastError());
		        closesocket(ClientSocket);
		        WSACleanup();
		        return 1;
		    }
		
		    // cleanup
		    closesocket(ClientSocket);
		    WSACleanup();
		
		    return 0;
		}
#pragma comment(lib, "ws2_32.lib")
#include <winsock2.h>
#include <iostream>
#include <windows.h>

#define DEFAULT_PORT "27015"

struct addrinfo* result = NULL, * ptr = NULL, hints;

SOCKET Connections[100];
int Counter = 0;

enum Packet {
	P_ChatMessage,
	P_Test
};

//bool ProcessPacket(int index, Packet packettype) {
//	switch (packettype) {
//	case P_ChatMessage:
//	{
//		int msg_size;
//		recv(Connections[index], (char*)&msg_size, sizeof(int), NULL);
//		char* msg = new char[msg_size + 1];
//		msg[msg_size] = '\0';
//		recv(Connections[index], msg, msg_size, NULL);
//		for (int i = 0; i < Counter; i++) {
//			if (i == index) {
//				continue;
//			}
//
//			Packet msgtype = P_ChatMessage;
//			send(Connections[i], (char*)&msgtype, sizeof(Packet), NULL);
//			send(Connections[i], (char*)&msg_size, sizeof(int), NULL);
//			send(Connections[i], msg, msg_size, NULL);
//		}
//		delete[] msg;
//		break;
//	}
//	default:
//		std::cout << "Unrecognized packet: " << packettype << std::endl;
//		break;
//	}
//
//	return true;
//}

//void ClientHandler(int index) {
//	Packet packettype;
//	while (true) {
//		recv(Connections[index], (char*)&packettype, sizeof(Packet), NULL);
//
//		if (!ProcessPacket(index, packettype)) {
//			break;
//		}
//	}
//	closesocket(Connections[index]);
//}

int main(int argc, char* argv[]) {
	//WSAStartup
	int iResult = 0;
	WSAData wsaData;
	WORD DLLVersion = MAKEWORD(2, 1);
	if (WSAStartup(DLLVersion, &wsaData) != 0) {
		std::cout << "Error" << std::endl;
		exit(1);
	}

	SOCKADDR_IN serverAddr;
	iResult = WSAStartup(MAKEWORD(2, 2), &wsaData);
	if (iResult != NO_ERROR) {
		wprintf(L"Error at WSAStartup()\n");
		return 1;
	}
	ZeroMemory(&hints, sizeof(hints));
	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_protocol = IPPROTO_TCP;
	hints.ai_flags = AI_PASSIVE;

	// Resolve the local address and port to be used by the server
	iResult = getaddrinfo(NULL, DEFAULT_PORT, &hints, &result);
	if (iResult != 0) {
		printf("getaddrinfo failed: %d\n", iResult);
		WSACleanup();
		return 1;
	}		
	
	SOCKET mainSocket = socket(AF_INET, SOCK_STREAM, NULL);
	bind(mainSocket, (SOCKADDR*)&serverAddr, sizeof(serverAddr));
	listen(mainSocket, 4);

	SOCKET newConnection;
	for (int i = 0; i < 100; i++) {
		newConnection = accept(mainSocket, (SOCKADDR*)&serverAddr, &sizeofaddr);

		if (newConnection == 0) {
			std::cout << "Error #2\n";
		}
		else {
			std::cout << "Client Connected!\n";
			std::string msg = "Hello. It`s my first network program!";
			int msg_size = msg.size();
			Packet msgtype = P_ChatMessage;
			send(newConnection, (char*)&msgtype, sizeof(Packet), NULL);
			send(newConnection, (char*)&msg_size, sizeof(int), NULL);
			send(newConnection, msg.c_str(), msg_size, NULL);

			Connections[i] = newConnection;
			Counter++;
			CreateThread(NULL, NULL, (LPTHREAD_START_ROUTINE)ClientHandler, (LPVOID)(i), NULL, NULL);

			Packet testpacket = P_Test;
			send(newConnection, (char*)&testpacket, sizeof(Packet), NULL);
		}
	}


	system("pause");
	return 0;
}
#pragma once
#include <chrono>
#include <mutex>
#include <thread>
#include<iostream>
#include <winsock2.h>
#include <iostream>
#include <windows.h>
#include <WS2tcpip.h>
#include<cstring>
#include "nlohmann/json.hpp"
#pragma comment(lib, "ws2_32.lib")
using std::cout;
struct ConfigServer {
	const char* Port;
	int MaxPlayer;
};
struct SClient {
public:
	char Buffer[512];
	sockaddr_in CAddr;
	socklen_t CAddrlen;
	SOCKET ClientSocket;
	nlohmann::json ConfigGame = {
		{"XPlayer" , 0.0 },
		{"YPlayer" , 0.0},
		{"XBullet" , 0.0},
		{"YBullet" , 0.0 },
		{"Direction" , {
			{"RIGHT", 0},
			{"LEFT" , 1}
		}}
	};
	SClient() {
		CAddrlen = sizeof(CAddr);
	}

};
class CServer {

	ConfigServer ServerConfig;
	addrinfo* ServerAddr = NULL;
	WSADATA WsaData;
	SOCKET MainSocket;
	SClient Clients[4];


	struct {
		bool TrigerHandle = true;
	}Triggers;

public:

	int InitServer(ConfigServer ServerConfig) {

		int initResult = 0;
		this->ServerConfig = ServerConfig;
		//std::cout<< this->ServerConfig.MaxPlayer;
		addrinfo  ServerAddrh;

		ZeroMemory(&ServerAddrh, sizeof(ServerAddrh));

		ServerAddrh.ai_family = AF_INET;
		ServerAddrh.ai_socktype = SOCK_STREAM;
		ServerAddrh.ai_protocol = IPPROTO_TCP;
		ServerAddrh.ai_flags = AI_PASSIVE;

		if (WSAStartup(MAKEWORD(2, 2), &this->WsaData)) {
			std::cout << "Error" << std::endl;
			exit(1);
			WSACleanup();
		}

		initResult = getaddrinfo(NULL, this->ServerConfig.Port, &ServerAddrh, &(this->ServerAddr));
		if (initResult != NULL) {
			cout << "getaddrinfo failed: %d: " << initResult << "\n";
			WSACleanup();
			return 1;
		}

		this->MainSocket = socket(this->ServerAddr->ai_family, this->ServerAddr->ai_socktype, this->ServerAddr->ai_protocol);
		if (this->MainSocket == INVALID_SOCKET) {
			cout << "Server socket failed with error: %ld" << WSAGetLastError() << '\n';
			freeaddrinfo(this->ServerAddr);
			WSACleanup();
			return 1;
		}

		initResult = bind(this->MainSocket, this->ServerAddr->ai_addr, this->ServerAddr->ai_addrlen);
		if (initResult == SOCKET_ERROR) {
			cout << "bind failed with error:" << WSAGetLastError() << "\n";
			freeaddrinfo(this->ServerAddr);
			closesocket(this->MainSocket);
			WSACleanup();
			return 1;
		}

		return EXIT_SUCCESS;

	}
	int StartServer() {

		int initResult = 0;
		initResult = listen(this->MainSocket, this->ServerConfig.MaxPlayer * 2);
		if (initResult == SOCKET_ERROR) {
			cout << "listen failed with error:" << WSAGetLastError() << "\n";
			freeaddrinfo(this->ServerAddr);
			closesocket(this->MainSocket);
			WSACleanup();
			return 1;

		}
		std::thread AcceptThread = std::thread(&CServer::AcceptConnections, this);
		AcceptThread.detach();
		return 0;

	}
	void HandleConnections() {

		while (this->Triggers.TrigerHandle) {
			for (int i = 0; i < this->ServerConfig.MaxPlayer; i++) {

				int byteslenrecv;
				byteslenrecv = recv(this->Clients[i].ClientSocket, this->Clients[i].Buffer, sizeof(this->Clients[i].Buffer), NULL);
				if ((byteslenrecv == SOCKET_ERROR) or (byteslenrecv == 0)) {
					//cout << "recv Error\n";
					strcpy_s(this->Clients[i].Buffer, "ClientBuffer0");
				}
				else {
					cout << "Hey it's client buffer: " << this->Clients[i].Buffer << "\n";
					Clients[i].ConfigGame = nlohmann::json::parse(this->Clients[i].Buffer);
					cout << "Hey it's client buffer: " << Clients[i].ConfigGame["XPlayer"] << "\n";
				}
				for (int j = 0; j < this->ServerConfig.MaxPlayer; j++) {

					if (j == i) continue;
					send(this->Clients[j].ClientSocket, this->Clients[i].Buffer, byteslenrecv, NULL);

				}

			}

		}
	}
	void AcceptConnections() {

		//std::cout << "main sokcet: " << this->MainSocket<< "\n";
		for (int i = 0; i < this->ServerConfig.MaxPlayer; i++) {
			//std::cout << "i: "<<i << "\n";			
			this->Clients[i].ClientSocket = accept(this->MainSocket, (sockaddr*)&this->Clients[i].CAddr, &this->Clients[i].CAddrlen);
			//std::cout << 1 << "\n";
			if (this->Clients[i].ClientSocket == INVALID_SOCKET) {
				cout << "Client socket failed with error: %ld\n" << WSAGetLastError() << "\n";
				closesocket(Clients[i].ClientSocket);
			}
			else {
				cout << "Client Connected!\n";
				std::thread HandleThread = std::thread(&CServer::HandleConnections, this);
				HandleThread.detach();

			}


		}

	}
	void StopServer() {
		Triggers.TrigerHandle = false;
		for (int i = 0; i < this->ServerConfig.MaxPlayer; i++) {

			closesocket(this->Clients[i].ClientSocket);
		}
		closesocket(MainSocket);
	}
	void RestartServer(ConfigServer ServerConfig) {
		Triggers.TrigerHandle = false;
		for (int i = 0; i < this->ServerConfig.MaxPlayer; i++) {
			closesocket(this->Clients[i].ClientSocket);
			memset(&this->Clients[i], 0, sizeof(Clients[i]));
			//memset(&this->Clients[i].CAddr, 0, sizeof(Clients[i].CAddr));
			//this->Clients[i].Buffer[512]=' ';			
			this->Clients[i].CAddrlen = sizeof(this->Clients[i].CAddr);
		}
		WSACleanup();
		ServerAddr = NULL;
		closesocket(this->MainSocket);
		InitServer(ServerConfig);
		StartServer();
	}
};
//std::string msg = "Hello. It`s my first network program!";
//int msg_size = msg.size();
//Packet msgtype = P_ChatMessage;
//send(newConnection, (char*)&msgtype, sizeof(Packet), NULL);
//send(newConnection, (char*)&msg_size, sizeof(int), NULL);
//send(newConnection, msg.c_str(), msg_size, NULL);

//CreateThread(NULL, NULL, (LPTHREAD_START_ROUTINE)ClientHandler, (LPVOID)(i), NULL, NULL);

//Packet testpacket = P_Test;
//send(newConnection, (char*)&testpacket, sizeof(Packet), NULL);




#pragma once
#include <chrono>
#include <mutex>
#include <thread>
#include<iostream>
#include <winsock2.h>
#include <iostream>
#include <windows.h>
#include <WS2tcpip.h>
#include<cstring>
#pragma comment(lib, "ws2_32.lib")
using std::cout;

struct ClientConfig {
	const char* Port;
	int Maxplayer;
	int id;
};
class CClient {

	struct addrinfo* ClientAddr = NULL;
	ClientConfig ConfigClient;
	WSADATA WsaData;
	SOCKET ConnectSocket;
	std::string AllDataClients[3];
	char BufferSend[512];
	char BufferRecv[512];
	std::string CanSendData = "OK";
	struct {
		bool TrigerHandle = true;
	}Triggers;
	nlohmann::json ConfigGame = {
		{"XP"		, 0.0f},
		{"YP"		, 0.0f},
		{"XB"		, 0.0f},
		{"YB"		, 0.0f},
		{"Dir"		, 0	 },
		{"CharacP"	, 0  }
	};


public:

	int InitClient(ClientConfig& ConfigClient) {

		int initResult = 0;
		this->ConfigClient = ConfigClient;

		//std::cout<< this->ServerConfig.MaxPlayer;
		addrinfo   ClientAddrH;


		ZeroMemory(&ClientAddrH, sizeof(ClientAddrH));
		ClientAddrH.ai_family = AF_INET;
		ClientAddrH.ai_socktype = SOCK_STREAM;
		ClientAddrH.ai_protocol = IPPROTO_TCP;


		if (WSAStartup(MAKEWORD(2, 2), &this->WsaData)) {
			std::cout << "Error" << std::endl;
			exit(1);
			WSACleanup();
		}

		initResult = getaddrinfo(NULL, "9889", &ClientAddrH, &this->ClientAddr);
		if (initResult != 0) {
			std::cout << "getaddrinfo failed: " << initResult << "\n";
			WSACleanup();
			return 1;
		}


		this->ConnectSocket = socket(ClientAddrH.ai_family, this->ClientAddr->ai_socktype, this->ClientAddr->ai_protocol);
		if (this->ConnectSocket == INVALID_SOCKET) {
			cout << "Error at socket: " << WSAGetLastError() << '\n';
			freeaddrinfo(this->ClientAddr);
			WSACleanup();
			return 1;
		}
		initResult = connect(this->ConnectSocket, this->ClientAddr->ai_addr, (int)this->ClientAddr->ai_addrlen);
		if (initResult == SOCKET_ERROR) {
			closesocket(this->ConnectSocket);
			cout << "Error at connect: " << WSAGetLastError() << '\n';
			this->ConnectSocket = INVALID_SOCKET;
		}
		freeaddrinfo(this->ClientAddr);
		if (this->ConnectSocket == INVALID_SOCKET) {
			cout << "Unable to connect to server!\n";
			WSACleanup();
			return 1;
		}
		else if (this->ConnectSocket != INVALID_SOCKET) cout << "Client connected to server!\n";

		return EXIT_SUCCESS;

	};
	void FillJson(float x, float y, int dir, int charcl) {
		this->ConfigGame["XP"] = x;
		this->ConfigGame["YP"] = y;
		this->ConfigGame["Dir"] = dir;
		this->ConfigGame["CharacP"] = charcl;

	}
	void StartClient() {
		while (this->Triggers.TrigerHandle) {
			SendtoServer();
			RecvData();
		}
	}	
	void SendtoServer() {
		int initResult = 0;
		initResult = send(this->ConnectSocket, this->ConfigGame.dump().c_str(), (nlohmann::to_string(this->ConfigGame)).length(), 0);
		if (initResult == SOCKET_ERROR) {
			cout<<"send failed: "<< WSAGetLastError()<<"\n";
			closesocket(this->ConnectSocket);
			WSACleanup();
		}
		else cout <<"SEND: "<< this->ConfigGame.dump().c_str()<<"\n";
	}
	// Receive data until the server closes the connection
	void RecvData() {
	
		//while (flagOK < 0) { 
		//	readBytes = recv(this->ConnectSocket, this->BufferRecv, sizeof(this->BufferRecv), 0);
		//	if ((readBytes == SOCKET_ERROR) || (readBytes == 0)) {
		//		cout << "recv Error or connect closed\n";
		//		//strcpy_s(this->Clients[i].Buffer, " ");
		//	}
		//	else if (this->BufferRecv == "OK")flagOK = 1;
		//}
		//readBytes = recv(this->ConnectSocket, this->BufferRecv, sizeof(this->BufferRecv), 0);
		//if ((readBytes == SOCKET_ERROR) || (readBytes == 0)) {
		//	closesocket(this->ConnectSocket);
		//	cout << "recv Error\n";
		//	//strcpy_s(this->Clients[i].Buffer, " ");
		//}
		//else {
		//	while (this->BufferRecv != "OK") {}
		//}

		cout << "RECVEND\n";

	};
	std::string GetAllDataClients(int i) {

		return this->AllDataClients[i].empty() ? " " : this->AllDataClients[i];
	}
	void ShutdownClient() {
		int initResult = 0;
		initResult = shutdown(this->ConnectSocket, SD_SEND);
		if (initResult == SOCKET_ERROR) {
			printf("shutdown failed: %d\n", WSAGetLastError());
			closesocket(this->ConnectSocket);
			WSACleanup();

		}
	}

	void StopConnect() {
		Triggers.TrigerHandle = false;
		closesocket(this->ConnectSocket);
		WSACleanup();

	};
};


#pragma once
#include <chrono>
#include <mutex>
#include <thread>
#include<iostream>
#include <winsock2.h>
#include <iostream>
#include <windows.h>
#include <WS2tcpip.h>
#include<cstring>
#include "nlohmann/json.hpp"
#pragma comment(lib, "ws2_32.lib")
using std::cout;
struct ConfigServer {
	const char* Port;
	int MaxPlayer;
};
struct SClient {
public:
	char Buffer[150];
	sockaddr_in CAddr;
	socklen_t CAddrlen;
	SOCKET ClientSocket;
	nlohmann::json ConfigGame = {
	{"XP"		, 0.0f},
	{"YP"		, 0.0f},
	{"XB"		, 0.0f},
	{"YB"		, 0.0f},
	{"Dir"		, 0	 },
	{"CharacP"	, 0  }
	};
	SClient() {
		CAddrlen = sizeof(CAddr);
	}

};
class CServer {

	ConfigServer ServerConfig;
	addrinfo* ServerAddr = NULL;
	WSADATA WsaData;
	SOCKET MainSocket;
	SClient Clients[4];
	std::string CanSendData = "OK";

	struct {
		bool TrigerHandle = true;
	}Triggers;

public:

	int InitServer(ConfigServer ServerConfig) {

		int initResult = 0;
		this->ServerConfig = ServerConfig;
		//std::cout<< this->ServerConfig.MaxPlayer;
		addrinfo  ServerAddrh;

		ZeroMemory(&ServerAddrh, sizeof(ServerAddrh));

		ServerAddrh.ai_family = AF_INET;
		ServerAddrh.ai_socktype = SOCK_STREAM;
		ServerAddrh.ai_protocol = IPPROTO_TCP;
		ServerAddrh.ai_flags = AI_PASSIVE;

		if (WSAStartup(MAKEWORD(2, 2), &this->WsaData)) {
			std::cout << "Error" << std::endl;
			exit(1);
			WSACleanup();
		}

		initResult = getaddrinfo(NULL, this->ServerConfig.Port, &ServerAddrh, &(this->ServerAddr));
		if (initResult != NULL) {
			cout << "getaddrinfo failed: %d: " << initResult << "\n";
			WSACleanup();
			return 1;
		}

		this->MainSocket = socket(this->ServerAddr->ai_family, this->ServerAddr->ai_socktype, this->ServerAddr->ai_protocol);
		if (this->MainSocket == INVALID_SOCKET) {
			cout << "Server socket failed with error: %ld" << WSAGetLastError() << '\n';
			freeaddrinfo(this->ServerAddr);
			WSACleanup();
			return 1;
		}

		initResult = bind(this->MainSocket, this->ServerAddr->ai_addr, this->ServerAddr->ai_addrlen);
		if (initResult == SOCKET_ERROR) {
			cout << "bind failed with error:" << WSAGetLastError() << "\n";
			freeaddrinfo(this->ServerAddr);
			closesocket(this->MainSocket);
			WSACleanup();
			return 1;
		}

		return EXIT_SUCCESS;

	}
	int StartServer() {

		int initResult = 0;
		initResult = listen(this->MainSocket, this->ServerConfig.MaxPlayer * 2);
		if (initResult == SOCKET_ERROR) {
			cout << "listen failed with error:" << WSAGetLastError() << "\n";
			freeaddrinfo(this->ServerAddr);
			closesocket(this->MainSocket);
			WSACleanup();
			return 1;

		}
		std::thread AcceptThread = std::thread(&CServer::AcceptConnections, this);
		AcceptThread.detach();
		return 0;

	}
	void HandleConnections() {

		while (this->Triggers.TrigerHandle) {
			for (int i = 0; i < this->ServerConfig.MaxPlayer; i++) {

				int byteslenrecv;
				byteslenrecv = recv(this->Clients[i].ClientSocket, this->Clients[i].Buffer, sizeof(this->Clients[i].Buffer), NULL);
				if ((byteslenrecv == SOCKET_ERROR) || (byteslenrecv == 0)) {
					//closesocket(this->Clients[i].ClientSocket);
					cout << "recv Error\n";
					strcpy_s(this->Clients[i].Buffer, "Oh no");
				}
				else {					
					for (int j = 0; j < this->ServerConfig.MaxPlayer; j++) {
						if (i == j) continue;
						cout << "BUF: " << this->Clients[i].Buffer <<"\t" << "recvlen"<< byteslenrecv << "\n" ;
						send(this->Clients[j].ClientSocket, this->Clients[i].Buffer, sizeof(this->Clients[i].Buffer), NULL);
					}

					send(this->Clients[i].ClientSocket, this->CanSendData.c_str(), this->CanSendData.length(), NULL);
				}

			}

		}
	}
	void AcceptConnections() {

		//std::cout << "main sokcet: " << this->MainSocket<< "\n";
		for (int i = 0; i < this->ServerConfig.MaxPlayer; i++) {
			//std::cout << "i: "<<i << "\n";			
			this->Clients[i].ClientSocket = accept(this->MainSocket, (sockaddr*)&this->Clients[i].CAddr, &this->Clients[i].CAddrlen);
			//std::cout << 1 << "\n";
			if (this->Clients[i].ClientSocket == INVALID_SOCKET) {
				cout << "Client socket failed with error: %ld\n" << WSAGetLastError() << "\n";
				closesocket(Clients[i].ClientSocket);
			}
			else {
				cout << "Client Connected!\n";
				std::thread HandleThread = std::thread(&CServer::HandleConnections, this);
				HandleThread.detach();

			}


		}

	}
	void StopServer() {
		Triggers.TrigerHandle = false;
		for (int i = 0; i < this->ServerConfig.MaxPlayer; i++) {

			closesocket(this->Clients[i].ClientSocket);
		}
		closesocket(MainSocket);
	}
	void RestartServer(ConfigServer ServerConfig) {
		Triggers.TrigerHandle = false;
		for (int i = 0; i < this->ServerConfig.MaxPlayer; i++) {
			closesocket(this->Clients[i].ClientSocket);
			memset(&this->Clients[i], 0, sizeof(Clients[i]));
			//memset(&this->Clients[i].CAddr, 0, sizeof(Clients[i].CAddr));
			//this->Clients[i].Buffer[512]=' ';			
			this->Clients[i].CAddrlen = sizeof(this->Clients[i].CAddr);
		}
		WSACleanup();
		ServerAddr = NULL;
		closesocket(this->MainSocket);
		InitServer(ServerConfig);
		StartServer();
	}
};
//std::string msg = "Hello. It`s my first network program!";
//int msg_size = msg.size();
//Packet msgtype = P_ChatMessage;
//send(newConnection, (char*)&msgtype, sizeof(Packet), NULL);
//send(newConnection, (char*)&msg_size, sizeof(int), NULL);
//send(newConnection, msg.c_str(), msg_size, NULL);

//CreateThread(NULL, NULL, (LPTHREAD_START_ROUTINE)ClientHandler, (LPVOID)(i), NULL, NULL);

//Packet testpacket = P_Test;
//send(newConnection, (char*)&testpacket, sizeof(Packet), NULL);


#pragma once
#include <chrono>
#include <mutex>
#include <thread>
#include<iostream>
#include <winsock2.h>
#include <iostream>
#include <windows.h>
#include <WS2tcpip.h>
#include<cstring>
#pragma comment(lib, "ws2_32.lib")
using std::cout;

struct ClientConfig {
	const char* Port;
	int Maxplayer;
	int id;
};
class CClient {

	struct addrinfo* ClientAddr = NULL;
	ClientConfig ConfigClient;
	WSADATA WsaData;
	SOCKET ConnectSocket;
	std::vector<std::string> AllDataClients;
	char BufferSend[512];
	char BufferRecv[512];
	std::string CanSendData = "OK";
	struct {
		bool TrigerHandle = true;
	}Triggers;
	nlohmann::json ConfigGame = {
		{"XP"		, 0.0f},
		{"YP"		, 0.0f},
		{"XB"		, 0.0f},
		{"YB"		, 0.0f},
		{"Dir"		, 0	 },
		{"CharacP"	, 0  }
	};


public:

	int InitClient(ClientConfig& ConfigClient) {

		int initResult = 0;
		this->ConfigClient = ConfigClient;

		//std::cout<< this->ServerConfig.MaxPlayer;
		addrinfo   ClientAddrH;


		ZeroMemory(&ClientAddrH, sizeof(ClientAddrH));
		ClientAddrH.ai_family = AF_INET;
		ClientAddrH.ai_socktype = SOCK_STREAM;
		ClientAddrH.ai_protocol = IPPROTO_TCP;


		if (WSAStartup(MAKEWORD(2, 2), &this->WsaData)) {
			std::cout << "Error" << std::endl;
			exit(1);
			WSACleanup();
		}

		initResult = getaddrinfo(NULL, "9889", &ClientAddrH, &this->ClientAddr);
		if (initResult != 0) {
			std::cout << "getaddrinfo failed: " << initResult << "\n";
			WSACleanup();
			return 1;
		}


		this->ConnectSocket = socket(ClientAddrH.ai_family, this->ClientAddr->ai_socktype, this->ClientAddr->ai_protocol);
		if (this->ConnectSocket == INVALID_SOCKET) {
			cout << "Error at socket: " << WSAGetLastError() << '\n';
			freeaddrinfo(this->ClientAddr);
			WSACleanup();
			return 1;
		}
		initResult = connect(this->ConnectSocket, this->ClientAddr->ai_addr, (int)this->ClientAddr->ai_addrlen);
		if (initResult == SOCKET_ERROR) {
			closesocket(this->ConnectSocket);
			cout << "Error at connect: " << WSAGetLastError() << '\n';
			this->ConnectSocket = INVALID_SOCKET;
		}
		freeaddrinfo(this->ClientAddr);
		if (this->ConnectSocket == INVALID_SOCKET) {
			cout << "Unable to connect to server!\n";
			WSACleanup();
			return 1;
		}
		else if (this->ConnectSocket != INVALID_SOCKET) cout << "Client connected to server!\n";

		return EXIT_SUCCESS;

	};
	void FillJson(float x, float y, int dir, int charcl) {
		this->ConfigGame["XP"] = x;
		this->ConfigGame["YP"] = y;
		this->ConfigGame["Dir"] = dir;
		this->ConfigGame["CharacP"] = charcl;

	}
	void StartClient() {
		while (this->Triggers.TrigerHandle) {
			SendtoServer();
			RecvData();
		}
	}
	void SendtoServer() {
		int initResult = 0;
		initResult = send(this->ConnectSocket, this->ConfigGame.dump().c_str(), (nlohmann::to_string(this->ConfigGame)).length(), 0);
		if (initResult == SOCKET_ERROR) {
			cout << "send failed: " << WSAGetLastError() << "\n";

			WSACleanup();
		}
		else cout << "SEND: " << this->ConfigGame.dump().c_str() << "\n";
	}
	// Receive data until the server closes the connection
	void RecvData() {
		int readBytes = 0;
		bool flagOK = 0;
		//		strcpy_s(this->BufferRecv, BufferRecv);
		readBytes = recv(this->ConnectSocket, this->BufferRecv, sizeof(this->BufferRecv), 0);
		if ((readBytes == SOCKET_ERROR) || (readBytes == 0)) {

			cout << "recv Error or connect closed: " << WSAGetLastError() << "\n";

		}
		else {
			if (this->AllDataClients.size() > 4)this->AllDataClients.clear();
			this->AllDataClients.push_back( this->BufferRecv);
			GetAllDataClients();
		}


	/*	for (int i = 0; i < this->ConfigClient.Maxplayer-1; i++) {

			readBytes = recv(this->ConnectSocket, this->BufferRecv, sizeof(this->BufferRecv), 0);
			if ((readBytes == SOCKET_ERROR) || (readBytes == 0)) {

				cout << "recv Error or connect closed: " << WSAGetLastError() << "\n";

			}
			else {
				this->AllDataClients[i] = this->BufferRecv;
				cout << "RECV: " << GetAllDataClients(i) << "\n";
			}
		}
		while (this->BufferRecv != CanSendData) {
			readBytes = recv(this->ConnectSocket, this->BufferRecv, sizeof(this->BufferRecv), 0);
			if ((readBytes == SOCKET_ERROR) || (readBytes == 0)) {
				cout << "recv Error or connect closed: " << WSAGetLastError() << "\n";
			}
		}*/

		//while (flagOK < 0) { 
		//	readBytes = recv(this->ConnectSocket, this->BufferRecv, sizeof(this->BufferRecv), 0);
		//	if ((readBytes == SOCKET_ERROR) || (readBytes == 0)) {
		//		cout << "recv Error or connect closed\n";
		//		//strcpy_s(this->Clients[i].Buffer, " ");
		//	}
		//	else if (this->BufferRecv == "OK")flagOK = 1;
		//}
		//readBytes = recv(this->ConnectSocket, this->BufferRecv, sizeof(this->BufferRecv), 0);
		//if ((readBytes == SOCKET_ERROR) || (readBytes == 0)) {
		//	closesocket(this->ConnectSocket);
		//	cout << "recv Error\n";
		//	//strcpy_s(this->Clients[i].Buffer, " ");
		//}
		//else {
		//	while (this->BufferRecv != "OK") {}
		//}

		cout << "RECVEND\n";

	};
	std::vector<std::string> GetAllDataClients() {
		for (auto const& element : this->AllDataClients)
			std::cout << element << ' ';
		return this->AllDataClients;
	}
	void ShutdownClient() {
		int initResult = 0;
		initResult = shutdown(this->ConnectSocket, SD_SEND);
		if (initResult == SOCKET_ERROR) {
			printf("shutdown failed: %d\n", WSAGetLastError());
			closesocket(this->ConnectSocket);
			WSACleanup();

		}
	}

	void StopConnect() {
		Triggers.TrigerHandle = false;
		closesocket(this->ConnectSocket);
		WSACleanup();

	};
};




#pragma once
#include <chrono>
#include <mutex>
#include <thread>
#include<iostream>
#include <winsock2.h>
#include <iostream>
#include <windows.h>
#include <WS2tcpip.h>
#include<cstring>
#pragma comment(lib, "ws2_32.lib")
using std::cout;

struct ClientConfig {
	const char* Port;
	int Maxplayer;
	int id;
};
class CClient {

	struct addrinfo* ClientAddr = NULL;
	ClientConfig ConfigClient;
	WSADATA WsaData;
	SOCKET ConnectSocket;
	std::string AllDataClients;
	char BufferSend[512];
	char BufferRecv[512];
	std::string CanSendData = "OK";
	struct {
		bool TrigerHandle = true;
	}Triggers;
	nlohmann::json ConfigGame = {

		{"XP"		, 0.0f},
		{"YP"		, 0.0f},
		{"XB"		, 0},
		{"YB"		, 0},
		{"Dir"		, 0	 },
		{"CharacP"	, 0  }
	};


public:

	int InitClient(ClientConfig& ConfigClient) {

		int initResult = 0;
		this->ConfigClient = ConfigClient;

		//std::cout<< this->ServerConfig.MaxPlayer;
		addrinfo   ClientAddrH;


		ZeroMemory(&ClientAddrH, sizeof(ClientAddrH));
		ClientAddrH.ai_family = AF_INET;
		ClientAddrH.ai_socktype = SOCK_STREAM;
		ClientAddrH.ai_protocol = IPPROTO_TCP;


		if (WSAStartup(MAKEWORD(2, 2), &this->WsaData)) {
			std::cout << "Error" << std::endl;
			exit(1);
			WSACleanup();
		}

		initResult = getaddrinfo(NULL, "9889", &ClientAddrH, &this->ClientAddr);
		if (initResult != 0) {
			std::cout << "getaddrinfo failed: " << initResult << "\n";
			WSACleanup();
			return 1;
		}


		this->ConnectSocket = socket(ClientAddrH.ai_family, this->ClientAddr->ai_socktype, this->ClientAddr->ai_protocol);
		if (this->ConnectSocket == INVALID_SOCKET) {
			cout << "Error at socket: " << WSAGetLastError() << '\n';
			freeaddrinfo(this->ClientAddr);
			WSACleanup();
			return 1;
		}
		initResult = connect(this->ConnectSocket, this->ClientAddr->ai_addr, (int)this->ClientAddr->ai_addrlen);
		if (initResult == SOCKET_ERROR) {
			closesocket(this->ConnectSocket);
			cout << "Error at connect: " << WSAGetLastError() << '\n';
			this->ConnectSocket = INVALID_SOCKET;
		}
		freeaddrinfo(this->ClientAddr);
		if (this->ConnectSocket == INVALID_SOCKET) {
			cout << "Unable to connect to server!\n";
			WSACleanup();
			return 1;
		}
		else if (this->ConnectSocket != INVALID_SOCKET) cout << "Client connected to server!\n";

		return EXIT_SUCCESS;

	};
	void FillJson(float x, float y, int dir, int charcl) {
		this->ConfigGame["XP"] = x;
		this->ConfigGame["YP"] = y;
		this->ConfigGame["Dir"] = dir;
		this->ConfigGame["CharacP"] = charcl;

	}
	void StartClient() {
		while (this->Triggers.TrigerHandle) {
			SendtoServer();
			RecvData();
		}
	}
	void SendtoServer() {
		int initResult = 0;
		initResult = send(this->ConnectSocket, this->ConfigGame.dump().c_str(), (nlohmann::to_string(this->ConfigGame)).length(), 0);
		if (initResult == SOCKET_ERROR) {
			cout << "send failed: " << WSAGetLastError() << "\n";

			WSACleanup();
		}
	//	else cout << "SEND: " << this->ConfigGame.dump().c_str() << "\n";
	}
	// Receive data until the server closes the connection
	void RecvData() {
		int readBytes = 0;
	
		readBytes = recv(this->ConnectSocket, this->BufferRecv, sizeof(this->BufferRecv), 0);
		if ((readBytes == SOCKET_ERROR) || (readBytes == 0)) {
			
			cout << "recv Error or connect closed: " << WSAGetLastError() << "\n";
		}
		else {
			
			this->AllDataClients = this->BufferRecv;
			this->AllDataClients = this->AllDataClients.substr(0, readBytes);
			//cout << GetAllDataClients();
		}
		cout << "RECVEND\n";

	};
	std::string GetAllDataClients() {

		return AllDataClients;
	}
	void ShutdownClient() {
		int initResult = 0;
		initResult = shutdown(this->ConnectSocket, SD_SEND);
		if (initResult == SOCKET_ERROR) {
			printf("shutdown failed: %d\n", WSAGetLastError());
			closesocket(this->ConnectSocket);
			WSACleanup();

		}
	}

	void StopConnect() {
		Triggers.TrigerHandle = false;
		closesocket(this->ConnectSocket);
		WSACleanup();

	};
};

#pragma once
#include <chrono>
#include <mutex>
#include <thread>
#include<iostream>
#include <winsock2.h>
#include <iostream>
#include <windows.h>
#include <WS2tcpip.h>
#include<string.h>
#include "nlohmann/json.hpp"
#pragma comment(lib, "ws2_32.lib")
using std::cout;
struct ConfigServer {
	const char* Port;
	int MaxPlayer;
};
struct SClient {
public:
	char Buffer[512];
	sockaddr_in CAddr;
	socklen_t CAddrlen;
	SOCKET ClientSocket;
	std::string AllDataClients;
	//nlohmann::json ConfigGame{
	//{"XP"		, 0.0f},
	//{"YP"		, 0.0f},
	//{"XB"		, 0.0f},
	//{"YB"		, 0.0f},
	//{"Dir"		, 0	 },
	//{"CharacP"	, 0  }
	//};
	std::string DataClients;
	SClient() {
		CAddrlen = sizeof(CAddr);
		DataClients = "L";
	}


};
class CServer {

	ConfigServer ServerConfig;
	addrinfo* ServerAddr = NULL;
	WSADATA WsaData;
	SOCKET MainSocket;
	SClient Clients[4];

	struct {
		bool TrigerHandle = true;
	}Triggers;

public:
	void FillDataClients(int i, int maxclients) {

		for (int counter = 0; counter < maxclients; counter++)
		{
			if (counter == i)continue;
			this->Clients[i].AllDataClients = this->Clients[i].AllDataClients + this->Clients[counter].DataClients;
		}
	}
	int InitServer(ConfigServer ServerConfig) {

		int initResult = 0;
		this->ServerConfig = ServerConfig;
		//std::cout<< this->ServerConfig.MaxPlayer;
		addrinfo  ServerAddrh;

		ZeroMemory(&ServerAddrh, sizeof(ServerAddrh));

		ServerAddrh.ai_family = AF_INET;
		ServerAddrh.ai_socktype = SOCK_STREAM;
		ServerAddrh.ai_protocol = IPPROTO_TCP;
		ServerAddrh.ai_flags = AI_PASSIVE;

		if (WSAStartup(MAKEWORD(2, 2), &this->WsaData)) {
			std::cout << "Error" << std::endl;
			exit(1);
			WSACleanup();
		}

		initResult = getaddrinfo(NULL, this->ServerConfig.Port, &ServerAddrh, &(this->ServerAddr));
		if (initResult != NULL) {
			cout << "getaddrinfo failed: %d: " << initResult << "\n";
			WSACleanup();
			return 1;
		}

		this->MainSocket = socket(this->ServerAddr->ai_family, this->ServerAddr->ai_socktype, this->ServerAddr->ai_protocol);
		if (this->MainSocket == INVALID_SOCKET) {
			cout << "Server socket failed with error: %ld" << WSAGetLastError() << '\n';
			freeaddrinfo(this->ServerAddr);
			WSACleanup();
			return 1;
		}

		initResult = bind(this->MainSocket, this->ServerAddr->ai_addr, this->ServerAddr->ai_addrlen);
		if (initResult == SOCKET_ERROR) {
			cout << "bind failed with error:" << WSAGetLastError() << "\n";
			freeaddrinfo(this->ServerAddr);
			closesocket(this->MainSocket);
			WSACleanup();
			return 1;
		}

		return EXIT_SUCCESS;

	}
	int StartServer() {

		int initResult = 0;
		initResult = listen(this->MainSocket, this->ServerConfig.MaxPlayer * 2);
		if (initResult == SOCKET_ERROR) {
			cout << "listen failed with error:" << WSAGetLastError() << "\n";
			freeaddrinfo(this->ServerAddr);
			closesocket(this->MainSocket);
			WSACleanup();
			return 1;

		}
		std::thread AcceptThread = std::thread(&CServer::AcceptConnections, this);
		AcceptThread.detach();
		return 0;

	}
	void HandleConnections() {

		while (this->Triggers.TrigerHandle) {
			for (int i = 0; i < this->ServerConfig.MaxPlayer; i++) {

				int byteslenrecv;
				byteslenrecv = recv(this->Clients[i].ClientSocket, this->Clients[i].Buffer ,sizeof(this->Clients[i].Buffer), NULL);
				if ((byteslenrecv == SOCKET_ERROR) || (byteslenrecv == 0)) {
					//closesocket(this->Clients[i].ClientSocket);
					//cout << "recv Error\n";
				}
				else {
					
					std::string temp=this->Clients[i].Buffer;
					temp= temp.substr(0 , byteslenrecv);
					//cout << "buf: " << this->Clients[i].AllDataClients.c_str() << "\n";
					this->Clients[i].DataClients = temp;
					FillDataClients(i, this->ServerConfig.MaxPlayer);
					//cout << "buf: " << this->Clients[i].AllDataClients.c_str() << "\n";
					send(this->Clients[i].ClientSocket, this->Clients[i].AllDataClients.c_str(), this->Clients[i].AllDataClients.length(), NULL);
					this->Clients[i].AllDataClients = "";
				}
				strcpy_s(this->Clients[i].Buffer, " \0");
			}

		}
	}
	void AcceptConnections() {

		//std::cout << "main sokcet: " << this->MainSocket<< "\n";
		for (int i = 0; i < this->ServerConfig.MaxPlayer; i++) {
			//std::cout << "i: "<<i << "\n";			
			this->Clients[i].ClientSocket = accept(this->MainSocket, (sockaddr*)&this->Clients[i].CAddr, &this->Clients[i].CAddrlen);
			//std::cout << 1 << "\n";
			if (this->Clients[i].ClientSocket == INVALID_SOCKET) {
				cout << "Client socket failed with error: %ld\n" << WSAGetLastError() << "\n";
				closesocket(Clients[i].ClientSocket);
			}
			else {
				cout << "Client Connected!\n";
				std::thread HandleThread = std::thread(&CServer::HandleConnections, this);
				HandleThread.detach();

			}


		}

	}
	void StopServer() {
		Triggers.TrigerHandle = false;
		for (int i = 0; i < this->ServerConfig.MaxPlayer; i++) {

			closesocket(this->Clients[i].ClientSocket);
		}
		closesocket(MainSocket);
	}
	void RestartServer(ConfigServer ServerConfig) {
		Triggers.TrigerHandle = false;
		for (int i = 0; i < this->ServerConfig.MaxPlayer; i++) {
			closesocket(this->Clients[i].ClientSocket);
			memset(&this->Clients[i], 0, sizeof(Clients[i]));
			//memset(&this->Clients[i].CAddr, 0, sizeof(Clients[i].CAddr));
			//this->Clients[i].Buffer[512]=' ';			
			this->Clients[i].CAddrlen = sizeof(this->Clients[i].CAddr);
		}
		WSACleanup();
		ServerAddr = NULL;
		closesocket(this->MainSocket);
		InitServer(ServerConfig);
		StartServer();
	}
};
//std::string msg = "Hello. It`s my first network program!";
//int msg_size = msg.size();
//Packet msgtype = P_ChatMessage;
//send(newConnection, (char*)&msgtype, sizeof(Packet), NULL);
//send(newConnection, (char*)&msg_size, sizeof(int), NULL);
//send(newConnection, msg.c_str(), msg_size, NULL);

//CreateThread(NULL, NULL, (LPTHREAD_START_ROUTINE)ClientHandler, (LPVOID)(i), NULL, NULL);

//Packet testpacket = P_Test;
//send(newConnection, (char*)&testpacket, sizeof(Packet), NULL);




{"CharacP":1,"Dir":1,"XP":1300.3333740234375,"YP":512.245849609375}
{"CharacP":1,"Dir":1,"XP":1298.09619140625,"YP":508.5269470214844}
{"CharacP":4,"Dir":1,"XP":121.9009780883789,"YP":1143.30078125}
{"CharacP":0,"Dir":0,"XP":1448.90234375,"YP":480.0}
{"CharacP":1,"Dir":1,"XP":-109.08417510986328,"YP":1074.69189453125}
{"CharacP":1,"Dir":1,"XP":853.6015625,"YP":-550.63037109375}
#pragma once
#include <chrono>
#include <mutex>
#include <thread>
#include<iostream>
#include <winsock2.h>
#include <iostream>
#include <windows.h>
#include <WS2tcpip.h>
#include<string.h>
#include "nlohmann/json.hpp"
#pragma comment(lib, "ws2_32.lib")
using std::cout;
struct ConfigServer {
	const char* Port;
	int MaxPlayer;
};
struct SClient {
public:
	char Buffer[256];
	sockaddr_in CAddr;
	socklen_t CAddrlen;
	SOCKET ClientSocket;
	std::string AllDataClients;
	std::string DataClients;
	SClient() {
		CAddrlen = sizeof(CAddr);
		DataClients = "Fail";
	}


};
class CServer {

	ConfigServer ServerConfig;
	addrinfo* ServerAddr = NULL;
	WSADATA WsaData;
	SOCKET MainSocket;
	SClient Clients[4];

	struct {
		bool TrigerHandle = true;
	}Triggers;

public:
	void FillDataClients(int i, int maxclients) {

		for (int counter = 0; counter < maxclients; counter++)
		{
			if (counter == i)continue;
			this->Clients[i].AllDataClients = this->Clients[i].AllDataClients + this->Clients[counter].DataClients;
		}
	}
	int InitServer(ConfigServer ServerConfig) {

		int initResult = 0;
		this->ServerConfig = ServerConfig;
		//std::cout<< this->ServerConfig.MaxPlayer;
		addrinfo  ServerAddrh;

		ZeroMemory(&ServerAddrh, sizeof(ServerAddrh));

		ServerAddrh.ai_family = AF_INET;
		ServerAddrh.ai_socktype = SOCK_STREAM;
		ServerAddrh.ai_protocol = IPPROTO_TCP;
		ServerAddrh.ai_flags = AI_PASSIVE;

		if (WSAStartup(MAKEWORD(2, 2), &this->WsaData)) {
			std::cout << "Error" << std::endl;
			exit(1);
			WSACleanup();
		}

		initResult = getaddrinfo(NULL, this->ServerConfig.Port, &ServerAddrh, &(this->ServerAddr));
		if (initResult != NULL) {
			cout << "getaddrinfo failed: %d: " << initResult << "\n";
			WSACleanup();
			return 1;
		}

		this->MainSocket = socket(this->ServerAddr->ai_family, this->ServerAddr->ai_socktype, this->ServerAddr->ai_protocol);
		if (this->MainSocket == INVALID_SOCKET) {
			cout << "Server socket failed with error: %ld" << WSAGetLastError() << '\n';
			freeaddrinfo(this->ServerAddr);
			WSACleanup();
			return 1;
		}

		initResult = bind(this->MainSocket, this->ServerAddr->ai_addr, this->ServerAddr->ai_addrlen);
		if (initResult == SOCKET_ERROR) {
			cout << "bind failed with error:" << WSAGetLastError() << "\n";
			freeaddrinfo(this->ServerAddr);
			closesocket(this->MainSocket);
			WSACleanup();
			return 1;
		}

		return EXIT_SUCCESS;

	}
	int StartServer() {

		int initResult = 0;
		initResult = listen(this->MainSocket, this->ServerConfig.MaxPlayer * 2);
		if (initResult == SOCKET_ERROR) {
			cout << "listen failed with error:" << WSAGetLastError() << "\n";
			freeaddrinfo(this->ServerAddr);
			closesocket(this->MainSocket);
			WSACleanup();
			return 1;

		}
		std::thread AcceptThread = std::thread(&CServer::AcceptConnections, this);
		AcceptThread.detach();
		return 0;

	}
	void HandleConnections() {

		while (this->Triggers.TrigerHandle) {
			for (int i = 0; i < this->ServerConfig.MaxPlayer; i++) {

				int byteslenrecv;
				cout << "RECVBEG\n";
				byteslenrecv = recv(this->Clients[i].ClientSocket, this->Clients[i].Buffer, sizeof(this->Clients[i].Buffer), NULL);
				cout << "byteslenrecv:" << byteslenrecv << "\n";
				if ((byteslenrecv == SOCKET_ERROR) || (byteslenrecv == 0)) {
					closesocket(this->Clients[i].ClientSocket);
					cout << "recv Error\n";
				}
				else {
					cout << "RECVSERV :" << this->Clients[i].Buffer << "\n";

					std::string temp = this->Clients[i].Buffer;
					if (byteslenrecv > temp.length())byteslenrecv = temp.length();
					cout << "bytlenandlenght " << byteslenrecv << "        :" << temp.length() << "\n";
					if ((temp[0] == '{') && (temp[byteslenrecv - 1] == '}') && (byteslenrecv > 0) && (temp.length() > 0)) {
						cout << "bytlenandlenght " << byteslenrecv << "        :" << temp.length() << "\n";

						cout << "bytlenandlenght " << byteslenrecv << "        :" << temp.length() << "\n";
						this->Clients[i].DataClients = temp.substr(0, byteslenrecv);

						cout << "cutbufer:" << temp << "sss\n";

						FillDataClients(i, this->ServerConfig.MaxPlayer);
						//cout << "SEND: " << this->Clients[i].AllDataClients.c_str() << "bufsend\n";
						send(this->Clients[i].ClientSocket, this->Clients[i].AllDataClients.c_str(), this->Clients[i].AllDataClients.length(), NULL);

					}
					else send(this->Clients[i].ClientSocket, "Fail", 4, NULL);
					//cout << "SEND: " << this->Clients[i].AllDataClients.c_str() << "\n";
				}
				strcpy_s(this->Clients[i].Buffer, "");
				this->Clients[i].AllDataClients = "";
			}

		}
	}
	void AcceptConnections() {

		for (int i = 0; i < this->ServerConfig.MaxPlayer; i++) {

			this->Clients[i].ClientSocket = accept(this->MainSocket, (sockaddr*)&this->Clients[i].CAddr, &this->Clients[i].CAddrlen);

			if (this->Clients[i].ClientSocket == INVALID_SOCKET) {
				cout << "Client socket failed with error: %ld\n" << WSAGetLastError() << "\n";
				closesocket(Clients[i].ClientSocket);
			}
			else {
				cout << "Client Connected!\n";
				std::thread HandleThread = std::thread(&CServer::HandleConnections, this);
				HandleThread.detach();

			}


		}

	}
	void StopServer() {
		Triggers.TrigerHandle = false;
		for (int i = 0; i < this->ServerConfig.MaxPlayer; i++) {

			closesocket(this->Clients[i].ClientSocket);
		}
		closesocket(MainSocket);
	}
	void RestartServer(ConfigServer ServerConfig) {
		Triggers.TrigerHandle = false;
		for (int i = 0; i < this->ServerConfig.MaxPlayer; i++) {
			closesocket(this->Clients[i].ClientSocket);
			memset(&this->Clients[i], 0, sizeof(Clients[i]));
			//memset(&this->Clients[i].CAddr, 0, sizeof(Clients[i].CAddr));
			//this->Clients[i].Buffer[512]=' ';			
			this->Clients[i].CAddrlen = sizeof(this->Clients[i].CAddr);
		}
		WSACleanup();
		ServerAddr = NULL;
		closesocket(this->MainSocket);
		InitServer(ServerConfig);
		StartServer();
	}
};
//std::string msg = "Hello. It`s my first network program!";
//int msg_size = msg.size();
//Packet msgtype = P_ChatMessage;
//send(newConnection, (char*)&msgtype, sizeof(Packet), NULL);
//send(newConnection, (char*)&msg_size, sizeof(int), NULL);
//send(newConnection, msg.c_str(), msg_size, NULL);

//CreateThread(NULL, NULL, (LPTHREAD_START_ROUTINE)ClientHandler, (LPVOID)(i), NULL, NULL);

//Packet testpacket = P_Test;
//send(newConnection, (char*)&testpacket, sizeof(Packet), NULL);

#pragma once
#include <chrono>
#include <mutex>
#include <thread>
#include<iostream>
#include <winsock2.h>
#include <iostream>
#include <windows.h>
#include <WS2tcpip.h>
#include<string.h>
#include "nlohmann/json.hpp"
#pragma comment(lib, "ws2_32.lib")
using std::cout;
struct ConfigServer {
	const char* Port;
	int MaxPlayer;
};
struct SClient {
public:
	char Buffer[256];
	sockaddr_in CAddr;
	socklen_t CAddrlen;
	SOCKET ClientSocket;
	std::string AllDataClients;
	std::string DataClients;
	SClient() {
		CAddrlen = sizeof(CAddr);
		DataClients = "Fail";
	}


};
class CServer {

	ConfigServer ServerConfig;
	addrinfo* ServerAddr = NULL;
	WSADATA WsaData;
	SOCKET MainSocket;
	SClient Clients[4];

	struct {
		bool TrigerHandle = true;
	}Triggers;

public:
	void FillDataClients(int i, int maxclients) {

		for (int counter = 0; counter < maxclients; counter++)
		{
			if (counter == i)continue;
			this->Clients[i].AllDataClients = this->Clients[i].AllDataClients + this->Clients[counter].DataClients;
		}
	}
	int InitServer(ConfigServer ServerConfig) {

		int initResult = 0;
		this->ServerConfig = ServerConfig;
		//std::cout<< this->ServerConfig.MaxPlayer;
		addrinfo  ServerAddrh;

		ZeroMemory(&ServerAddrh, sizeof(ServerAddrh));

		ServerAddrh.ai_family = AF_INET;
		ServerAddrh.ai_socktype = SOCK_STREAM;
		ServerAddrh.ai_protocol = IPPROTO_TCP;
		ServerAddrh.ai_flags = AI_PASSIVE;

		if (WSAStartup(MAKEWORD(2, 2), &this->WsaData)) {
			std::cout << "Error" << std::endl;
			exit(1);
			WSACleanup();
		}

		initResult = getaddrinfo(NULL, this->ServerConfig.Port, &ServerAddrh, &(this->ServerAddr));
		if (initResult != NULL) {
			cout << "getaddrinfo failed: %d: " << initResult << "\n";
			WSACleanup();
			return 1;
		}

		this->MainSocket = socket(this->ServerAddr->ai_family, this->ServerAddr->ai_socktype, this->ServerAddr->ai_protocol);
		if (this->MainSocket == INVALID_SOCKET) {
			cout << "Server socket failed with error: %ld" << WSAGetLastError() << '\n';
			freeaddrinfo(this->ServerAddr);
			WSACleanup();
			return 1;
		}

		initResult = bind(this->MainSocket, this->ServerAddr->ai_addr, this->ServerAddr->ai_addrlen);
		if (initResult == SOCKET_ERROR) {
			cout << "bind failed with error:" << WSAGetLastError() << "\n";
			freeaddrinfo(this->ServerAddr);
			closesocket(this->MainSocket);
			WSACleanup();
			return 1;
		}

		return EXIT_SUCCESS;

	}
	int StartServer() {

		int initResult = 0;
		initResult = listen(this->MainSocket, this->ServerConfig.MaxPlayer * 2);
		if (initResult == SOCKET_ERROR) {
			cout << "listen failed with error:" << WSAGetLastError() << "\n";
			freeaddrinfo(this->ServerAddr);
			closesocket(this->MainSocket);
			WSACleanup();
			return 1;

		}
		std::thread AcceptThread = std::thread(&CServer::AcceptConnections, this);
		AcceptThread.detach();
		return 0;

	}
	void HandleConnections() {

		while (this->Triggers.TrigerHandle) {
			for (int i = 0; i < this->ServerConfig.MaxPlayer; i++) {

				int byteslenrecv;
			//	cout << "RECVBEG\n";
				byteslenrecv = recv(this->Clients[i].ClientSocket, this->Clients[i].Buffer, sizeof(this->Clients[i].Buffer), NULL);
			//	cout << "byteslenrecv:" << byteslenrecv << "\n";
				if ((byteslenrecv == SOCKET_ERROR) || (byteslenrecv == 0)) {
					closesocket(this->Clients[i].ClientSocket);
					//cout << "recv Error\n";
				}
				else {
				//	cout << "RECVSERV :" << this->Clients[i].Buffer << "\n";

					std::string temp = this->Clients[i].Buffer;
					if (byteslenrecv > temp.length())byteslenrecv = temp.length();
					cout << "bytlenandlenght " << byteslenrecv << "        :" << temp.length() << "\n";
					if ((temp[0] == '{') && (temp[byteslenrecv - 1] == '}') && (byteslenrecv > 0) && (temp.length() > 0)) {
					//	cout << "bytlenandlenght " << byteslenrecv << "        :" << temp.length() << "\n";

					//	cout << "bytlenandlenght " << byteslenrecv << "        :" << temp.length() << "\n";
						this->Clients[i].DataClients = temp.substr(0, byteslenrecv);

					//	cout << "cutbufer:" << temp << "sss\n";

						FillDataClients(i, this->ServerConfig.MaxPlayer);
						//cout << "SEND: " << this->Clients[i].AllDataClients.c_str() << "bufsend\n";
						send(this->Clients[i].ClientSocket, this->Clients[i].AllDataClients.c_str(), this->Clients[i].AllDataClients.length(), NULL);

					}
					else send(this->Clients[i].ClientSocket, "Fail", 4, NULL);
					//cout << "SEND: " << this->Clients[i].AllDataClients.c_str() << "\n";
				}
				strcpy_s(this->Clients[i].Buffer, "");
				this->Clients[i].AllDataClients = "";
			}

		}
	}
	void AcceptConnections() {

		for (int i = 0; i < this->ServerConfig.MaxPlayer; i++) {

			this->Clients[i].ClientSocket = accept(this->MainSocket, (sockaddr*)&this->Clients[i].CAddr, &this->Clients[i].CAddrlen);

			if (this->Clients[i].ClientSocket == INVALID_SOCKET) {
				cout << "Client socket failed with error: %ld\n" << WSAGetLastError() << "\n";
				closesocket(Clients[i].ClientSocket);
			}
			else {
				cout << "Client Connected!\n";
				std::thread HandleThread = std::thread(&CServer::HandleConnections, this);
				HandleThread.detach();

			}


		}

	}
	void StopServer() {
		Triggers.TrigerHandle = false;
		for (int i = 0; i < this->ServerConfig.MaxPlayer; i++) {

			closesocket(this->Clients[i].ClientSocket);
		}
		closesocket(MainSocket);
	}
	void RestartServer(ConfigServer ServerConfig) {
		Triggers.TrigerHandle = false;
		for (int i = 0; i < this->ServerConfig.MaxPlayer; i++) {
			closesocket(this->Clients[i].ClientSocket);
			memset(&this->Clients[i], 0, sizeof(Clients[i]));
			//memset(&this->Clients[i].CAddr, 0, sizeof(Clients[i].CAddr));
			//this->Clients[i].Buffer[512]=' ';			
			this->Clients[i].CAddrlen = sizeof(this->Clients[i].CAddr);
		}
		WSACleanup();
		ServerAddr = NULL;
		closesocket(this->MainSocket);
		InitServer(ServerConfig);
		StartServer();
	}
};
//std::string msg = "Hello. It`s my first network program!";
//int msg_size = msg.size();
//Packet msgtype = P_ChatMessage;
//send(newConnection, (char*)&msgtype, sizeof(Packet), NULL);
//send(newConnection, (char*)&msg_size, sizeof(int), NULL);
//send(newConnection, msg.c_str(), msg_size, NULL);

//CreateThread(NULL, NULL, (LPTHREAD_START_ROUTINE)ClientHandler, (LPVOID)(i), NULL, NULL);

//Packet testpacket = P_Test;
//send(newConnection, (char*)&testpacket, sizeof(Packet), NULL);

#pragma once
#include <chrono>
#include <mutex>
#include <thread>
#include<iostream>
#include <winsock2.h>
#include <iostream>
#include <windows.h>
#include <WS2tcpip.h>
#include<cstring>
#pragma comment(lib, "ws2_32.lib")
using std::cout;

struct ClientConfig {
	const char* Port;
	int Maxplayer;
};
class CClient {
	std::chrono::milliseconds ms;
	struct addrinfo* ClientAddr = NULL;
	ClientConfig ConfigClient;
	WSADATA WsaData;
	SOCKET ConnectSocket;
	std::string AllDataClients;
	char Buffer[512];
	std::string CanSendData = "OK";
	struct {
		bool TrigerHandle = true;
	}Triggers;
	nlohmann::json ConfigGame = {
		{"XP"		, 0.0f},
		{"YP"		, 0.0f},
		{"Dir"		, 0	 },
		{"CharacP"	, 0  }
	};


public:

	int InitClient(ClientConfig& ConfigClient) {

		int initResult = 0;
		this->ConfigClient = ConfigClient;

		//std::cout<< this->ServerConfig.MaxPlayer;
		addrinfo   ClientAddrH;


		ZeroMemory(&ClientAddrH, sizeof(ClientAddrH));
		ClientAddrH.ai_family = AF_INET;
		ClientAddrH.ai_socktype = SOCK_STREAM;
		ClientAddrH.ai_protocol = IPPROTO_TCP;


		if (WSAStartup(MAKEWORD(2, 2), &this->WsaData)) {
			std::cout << "Error" << std::endl;
			exit(1);
			WSACleanup();
		}

		initResult = getaddrinfo(NULL, "9889", &ClientAddrH, &this->ClientAddr);
		if (initResult != 0) {
			std::cout << "getaddrinfo failed: " << initResult << "\n";
			WSACleanup();
			return 1;
		}


		this->ConnectSocket = socket(ClientAddrH.ai_family, this->ClientAddr->ai_socktype, this->ClientAddr->ai_protocol);
		if (this->ConnectSocket == INVALID_SOCKET) {
			cout << "Error at socket: " << WSAGetLastError() << '\n';
			freeaddrinfo(this->ClientAddr);
			WSACleanup();
			return 1;
		}
		initResult = connect(this->ConnectSocket, this->ClientAddr->ai_addr, (int)this->ClientAddr->ai_addrlen);
		if (initResult == SOCKET_ERROR) {
			closesocket(this->ConnectSocket);
			cout << "Error at connect: " << WSAGetLastError() << '\n';
			this->ConnectSocket = INVALID_SOCKET;
		}
		freeaddrinfo(this->ClientAddr);
		if (this->ConnectSocket == INVALID_SOCKET) {
			cout << "Unable to connect to server!\n";
			WSACleanup();
			return 1;
		}
		else if (this->ConnectSocket != INVALID_SOCKET) cout << "Client connected to server!\n";

		return EXIT_SUCCESS;

	};
	void FillJson(float x, float y, int dir, int charcl) {
		this->ConfigGame["XP"] = x;
		this->ConfigGame["YP"] = y;
		this->ConfigGame["Dir"] = dir;
		this->ConfigGame["CharacP"] = charcl;

	}
	void Re() {
		int i;
		std::cin >> i;
		if (i == 1) { SendtoServer(); }
	
	}
	void StartClient() {
		while (this->Triggers.TrigerHandle) {
			SendtoServer();
			RecvData();
		}
	}
	void SendtoServer() {
		int initResult = 0;
		//this->ConfigGame.dump().length() (nlohmann::to_string(this->ConfigGame)).length()
		initResult = send(this->ConnectSocket, (this->ConfigGame.dump().c_str()), this->ConfigGame.dump().length(), 0);
		if (initResult == SOCKET_ERROR) {
			cout << "send failed: " << WSAGetLastError() << "\n";

			WSACleanup();
		}
		//else cout << "SEND: "<< this->ConfigGame.dump().c_str() << "\n";
	}
	// Receive data until the server closes the connection
	void RecvData() {
		int readBytes = 0;
		//ms = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch());	
		//std::cout << "time:" << ms.count() << "\n";
		//	cout << "RECVBEG\n"; 
		readBytes = recv(this->ConnectSocket, this->Buffer, sizeof(this->Buffer), NULL);
		cout << "ReadBytes" << readBytes << "\n";
		//cout<<"RECVEND\n";
		if ((readBytes == SOCKET_ERROR) || (readBytes == 0)) {
			cout << "recv Error or connect closed: " << WSAGetLastError() << "\n";
		}
		else {
			
			this->AllDataClients = this->Buffer;
			if (readBytes > this->AllDataClients.length())readBytes = this->AllDataClients.length();
			if (((this->AllDataClients[0] == '{')) && (this->AllDataClients[readBytes-1] == '}')) {
				this->AllDataClients = this->AllDataClients.substr(0, readBytes);
			}
			else this->AllDataClients = "Fail";
		//	cout << "RECV: "<<GetAllDataClients()<<"\n";
		}
		//strcpy_s(this->Buffer, "");
		//cout << "RECVEND\n";
	};
	std::string GetAllDataClients() {
		return AllDataClients;
	}
	void ShutdownClient() {
		int initResult = 0;
		initResult = shutdown(this->ConnectSocket, SD_SEND);
		if (initResult == SOCKET_ERROR) {
			printf("shutdown failed: %d\n", WSAGetLastError());
			closesocket(this->ConnectSocket);
			WSACleanup();

		}
	}

	void StopConnect() {
		Triggers.TrigerHandle = false;
		closesocket(this->ConnectSocket);
		WSACleanup();

	};
};








#pragma once
#include <chrono>
#include <mutex>
#include <thread>
#include<iostream>
#include <winsock2.h>
#include <iostream>
#include <windows.h>
#include <WS2tcpip.h>
#include<string.h>
#include "nlohmann/json.hpp"
#pragma comment(lib, "ws2_32.lib")
using std::cout;
struct ConfigServer {
	const char* Port;
	int MaxPlayer;
};
struct SClient {
public:
	char Buffer[256];
	sockaddr_in CAddr;
	socklen_t CAddrlen;
	SOCKET ClientSocket;
	std::string AllDataClients;
	std::string DataClients;
	SClient() {
		CAddrlen = sizeof(CAddr);
		DataClients = "Fail";
	}


};
class CServer {

	ConfigServer ServerConfig;
	addrinfo* ServerAddr = NULL;
	WSADATA WsaData;
	SOCKET MainSocket;
	SClient Clients[4];

	struct {
		bool TrigerHandle = true;
	}Triggers;

public:
	void FillDataClients(int i, int maxclients) {

		for (int counter = 0; counter < maxclients; counter++)
		{
			if (counter == i)continue;
			this->Clients[i].AllDataClients = this->Clients[i].AllDataClients + this->Clients[counter].DataClients;
		}
	}
	int InitServer(ConfigServer ServerConfig) {

		int initResult = 0;
		this->ServerConfig = ServerConfig;
		//std::cout<< this->ServerConfig.MaxPlayer;
		addrinfo  ServerAddrh;

		ZeroMemory(&ServerAddrh, sizeof(ServerAddrh));

		ServerAddrh.ai_family = AF_INET;
		ServerAddrh.ai_socktype = SOCK_STREAM;
		ServerAddrh.ai_protocol = IPPROTO_TCP;
		
		ServerAddrh.ai_flags = AI_PASSIVE;

		if (WSAStartup(MAKEWORD(2, 2), &this->WsaData)) {
			std::cout << "Error" << std::endl;
			exit(1);
			WSACleanup();
		}

		initResult = getaddrinfo(NULL, this->ServerConfig.Port, &ServerAddrh, &(this->ServerAddr));
		if (initResult != NULL) {
			cout << "getaddrinfo failed: %d: " << initResult << "\n";
			WSACleanup();
			return 1;
		}

		this->MainSocket = socket(this->ServerAddr->ai_family, this->ServerAddr->ai_socktype, this->ServerAddr->ai_protocol);
		if (this->MainSocket == INVALID_SOCKET) {
			cout << "Server socket failed with error: %ld" << WSAGetLastError() << '\n';
			freeaddrinfo(this->ServerAddr);
			WSACleanup();
			return 1;
		}

		initResult = bind(this->MainSocket, this->ServerAddr->ai_addr, this->ServerAddr->ai_addrlen);
		if (initResult == SOCKET_ERROR) {
			cout << "bind failed with error:" << WSAGetLastError() << "\n";
			freeaddrinfo(this->ServerAddr);
			closesocket(this->MainSocket);
			WSACleanup();
			return 1;
		}

		return EXIT_SUCCESS;

	}
	int StartServer() {

		int initResult = 0;
		initResult = listen(this->MainSocket, this->ServerConfig.MaxPlayer * 2);
		if (initResult == SOCKET_ERROR) {
			cout << "listen failed with error:" << WSAGetLastError() << "\n";
			freeaddrinfo(this->ServerAddr);
			closesocket(this->MainSocket);
			WSACleanup();
			return 1;

		}
		std::thread AcceptThread = std::thread(&CServer::AcceptConnections, this);
		AcceptThread.detach();
		return 0;

	}
	void HandleConnections() {

		while (this->Triggers.TrigerHandle) {
			for (int i = 0; i < this->ServerConfig.MaxPlayer; i++) {

				int byteslenrecv;
				
				byteslenrecv = recv(this->Clients[i].ClientSocket, this->Clients[i].Buffer, sizeof(this->Clients[i].Buffer), NULL);
		
				if ((byteslenrecv == SOCKET_ERROR) || (byteslenrecv == 0)) {
					closesocket(this->Clients[i].ClientSocket);

				}
				else {
					std::string temp = this->Clients[i].Buffer;
					if (byteslenrecv > temp.length())byteslenrecv = temp.length();
					if ((temp[0] == '{') && (temp[byteslenrecv - 1] == '}') && (byteslenrecv > 0) && (temp.length() > 0)) {
					this->Clients[i].DataClients = temp.substr(0, byteslenrecv);
					std::this_thread::sleep_for(std::chrono::microseconds(10));
						FillDataClients(i, this->ServerConfig.MaxPlayer);
						send(this->Clients[i].ClientSocket, this->Clients[i].AllDataClients.c_str(), this->Clients[i].AllDataClients.length(), NULL);

					}
					else send(this->Clients[i].ClientSocket, "Fail", 4, NULL);
				}
				strcpy_s(this->Clients[i].Buffer, "");
				this->Clients[i].AllDataClients = "";

				
			}

		}
	}
	void AcceptConnections() {

		for (int i = 0; i < this->ServerConfig.MaxPlayer; i++) {

			this->Clients[i].ClientSocket = accept(this->MainSocket, (sockaddr*)&this->Clients[i].CAddr, &this->Clients[i].CAddrlen);

			if (this->Clients[i].ClientSocket == INVALID_SOCKET) {
				cout << "Client socket failed with error: %ld\n" << WSAGetLastError() << "\n";
				closesocket(Clients[i].ClientSocket);
			}
			else {
				cout << "Client Connected!\n";
				std::thread HandleThread = std::thread(&CServer::HandleConnections, this);
				HandleThread.detach();

			}

		}

	}
	void StopServer() {
		Triggers.TrigerHandle = false;
		for (int i = 0; i < this->ServerConfig.MaxPlayer; i++) {

			closesocket(this->Clients[i].ClientSocket);
		}
		closesocket(MainSocket);
	}
	void RestartServer(ConfigServer ServerConfig) {
		Triggers.TrigerHandle = false;
		for (int i = 0; i < this->ServerConfig.MaxPlayer; i++) {
			closesocket(this->Clients[i].ClientSocket);
			memset(&this->Clients[i], 0, sizeof(Clients[i]));		
			this->Clients[i].CAddrlen = sizeof(this->Clients[i].CAddr);
		}
		WSACleanup();
		ServerAddr = NULL;
		closesocket(this->MainSocket);
		InitServer(ServerConfig);
		StartServer();
	}
};
//std::string msg = "Hello. It`s my first network program!";
//int msg_size = msg.size();
//Packet msgtype = P_ChatMessage;
//send(newConnection, (char*)&msgtype, sizeof(Packet), NULL);
//send(newConnection, (char*)&msg_size, sizeof(int), NULL);
//send(newConnection, msg.c_str(), msg_size, NULL);

//CreateThread(NULL, NULL, (LPTHREAD_START_ROUTINE)ClientHandler, (LPVOID)(i), NULL, NULL);

//Packet testpacket = P_Test;
//send(newConnection, (char*)&testpacket, sizeof(Packet), NULL);




#pragma once
#include <thread>
#include<iostream>
#include <winsock2.h>
#include <iostream>
#include <windows.h>
#include <WS2tcpip.h>
#include<cstring>
#pragma comment(lib, "ws2_32.lib")
using std::cout;

struct ClientConfig {
	const char* Port;
	int Maxplayer;
};
class CClient {
	std::chrono::milliseconds ms;
	struct addrinfo* ClientAddr = NULL;
	ClientConfig ConfigClient;
	WSADATA WsaData;
	SOCKET ConnectSocket;
	std::string AllDataClients;
	char Buffer[512];
	std::string CanSendData = "OK";
	struct {
		bool TrigerHandle = true;
	}Triggers;
	nlohmann::json ConfigGame = {
	{"XP"		, 0.0f},
	{"YP"		, 0.0f},
	{"Dir"		, 0	 },
	{"CharacP"	, 0  },
	{"Health"	, 0	 },
	{"Shoot"	,0   }
	};
public:

	int InitClient(ClientConfig& ConfigClient) {

		int initResult = 0;
		this->ConfigClient = ConfigClient;

		addrinfo   ClientAddrH;


		ZeroMemory(&ClientAddrH, sizeof(ClientAddrH));
		ClientAddrH.ai_family = AF_INET;
		ClientAddrH.ai_socktype = SOCK_STREAM;
		ClientAddrH.ai_protocol = IPPROTO_TCP;
	

		if (WSAStartup(MAKEWORD(2, 2), &this->WsaData)) {
			std::cout << "Error" << std::endl;
			exit(1);
			WSACleanup();
		}

		initResult = getaddrinfo(NULL, "9889", &ClientAddrH, &this->ClientAddr);
		if (initResult != 0) {
			std::cout << "getaddrinfo failed: " << initResult << "\n";
			WSACleanup();
			return 1;
		}


		this->ConnectSocket = socket(ClientAddrH.ai_family, this->ClientAddr->ai_socktype, this->ClientAddr->ai_protocol);
		if (this->ConnectSocket == INVALID_SOCKET) {
			cout << "Error at socket: " << WSAGetLastError() << '\n';
			freeaddrinfo(this->ClientAddr);
			WSACleanup();
			return 1;
		}
		initResult = connect(this->ConnectSocket, this->ClientAddr->ai_addr, (int)this->ClientAddr->ai_addrlen);
		if (initResult == SOCKET_ERROR) {
			closesocket(this->ConnectSocket);
			cout << "Error at connect: " << WSAGetLastError() << '\n';
			this->ConnectSocket = INVALID_SOCKET;
		}
		freeaddrinfo(this->ClientAddr);
		if (this->ConnectSocket == INVALID_SOCKET) {
			cout << "Unable to connect to server!\n";
			WSACleanup();
			return 1;
		}
		else if (this->ConnectSocket != INVALID_SOCKET) cout << "Client connected to server!\n";

		return EXIT_SUCCESS;

	};
	void FillJson(float x, float y, int dir, int charcl, int lives,bool IsAshoot) {
		this->ConfigGame["XP"] = x;
		this->ConfigGame["YP"] = y;
		this->ConfigGame["Dir"] = dir;
		this->ConfigGame["CharacP"] = charcl;
		this->ConfigGame["Health"] = lives;
		this->ConfigGame["Shoot"] = IsAshoot;
	}

	void StartClient() {
		while (this->Triggers.TrigerHandle) {
			SendtoServer();
			std::this_thread::sleep_for(std::chrono::microseconds(15));
			RecvData();
		}
	}
	void SendtoServer() {
		int initResult = 0;
		//this->ConfigGame.dump().length() (nlohmann::to_string(this->ConfigGame)).length()
		initResult = send(this->ConnectSocket, (this->ConfigGame.dump().c_str()), this->ConfigGame.dump().length(), 0);
		if (initResult == SOCKET_ERROR) {
			cout << "send failed: " << WSAGetLastError() << "\n";

			WSACleanup();
		}
		//else cout << "SEND: "<< this->ConfigGame.dump().c_str() << "\n";
	}
	void RecvData() {
		int readBytes = 0;
 
		readBytes = recv(this->ConnectSocket, this->Buffer, sizeof(this->Buffer), NULL);
		//cout << "ReadBytes" << readBytes << "\n";
		if ((readBytes == SOCKET_ERROR) || (readBytes == 0)) {
			cout << "recv Error or connect closed: " << WSAGetLastError() << "\n";
		}
		else {
			
			this->AllDataClients = this->Buffer;
			if (readBytes > this->AllDataClients.length())readBytes = this->AllDataClients.length();
			if (((this->AllDataClients[0] == '{')) && (this->AllDataClients[readBytes-1] == '}')) {
				this->AllDataClients = this->AllDataClients.substr(0, readBytes);
			}
			else this->AllDataClients = "Fail";
		}
	
	};
	std::string GetAllDataClients() {
		return AllDataClients;
	}
	void ShutdownClient() {
		int initResult = 0;
		initResult = shutdown(this->ConnectSocket, SD_SEND);
		if (initResult == SOCKET_ERROR) {
			printf("shutdown failed: %d\n", WSAGetLastError());
			closesocket(this->ConnectSocket);
			WSACleanup();

		}
	}

	void StopConnect() {
		Triggers.TrigerHandle = false;
		closesocket(this->ConnectSocket);
		WSACleanup();

	};
};


#pragma once
#include <thread>
#include<iostream>
#include <winsock2.h>
#include <iostream>
#include <windows.h>
#include <WS2tcpip.h>
#include<cstring>

#pragma comment(lib, "ws2_32.lib")
using std::cout;

struct ClientConfig {
	const char* Port;
	int Maxplayer;
	const char* IPaddr;
};
class CClient {
	std::chrono::milliseconds ms;
	struct addrinfo* ClientAddr = NULL;
	int CounterForReconnect ;
	ClientConfig ConfigClient;
	WSADATA WsaData;
	SOCKET ConnectSocket;
	std::string AllDataClients;
	char Buffer[512];
	u_long iMode = 0;
	std::string CanSendData = "OK";
	struct {
		bool TrigerHandle = true;
	}Triggers;
	nlohmann::json ConfigGame = {
	{"XP"		, 0.0f},
	{"YP"		, 0.0f},
	{"Dir"		, 0	 },
	{"CharacP"	, 0  },
	{"Health"	, 0	 },
	{"Shoot"	,0   }
	};
public:

	int InitClient(ClientConfig& ConfigClient) {

		CounterForReconnect = 0;
		int initResult = 0;
		this->ConfigClient = ConfigClient;

		addrinfo   ClientAddrH;
				
		ZeroMemory(&ClientAddrH, sizeof(ClientAddrH));
		ClientAddrH.ai_family = AF_INET;
		ClientAddrH.ai_socktype = SOCK_STREAM;
		ClientAddrH.ai_protocol = IPPROTO_TCP;
		ClientAddrH.ai_flags = AI_NUMERICHOST;

		if (WSAStartup(MAKEWORD(2, 2), &this->WsaData)) {
			std::cout << "Error" << std::endl;
			exit(1);
			WSACleanup();
		}

		initResult = getaddrinfo(this->ConfigClient.IPaddr, "9889", &ClientAddrH, &this->ClientAddr);
		if (initResult != 0) {
			std::cout << "getaddrinfo failed: " << initResult << "\n";
			WSACleanup();
			return 1;
		}


		this->ConnectSocket = socket(ClientAddrH.ai_family, this->ClientAddr->ai_socktype, this->ClientAddr->ai_protocol);
		if (this->ConnectSocket == INVALID_SOCKET) {
			cout << "Error at socket: " << WSAGetLastError() << '\n';
			freeaddrinfo(this->ClientAddr);
			WSACleanup();
			return 1;
		}
		initResult = ioctlsocket(this->ConnectSocket, FIONBIO, &iMode);
		if (initResult != NO_ERROR)
			printf("ioctlsocket failed with error: %ld\n", initResult);
		initResult = connect(this->ConnectSocket, this->ClientAddr->ai_addr, (int)this->ClientAddr->ai_addrlen);
		if (initResult == SOCKET_ERROR) {
			closesocket(this->ConnectSocket);
			cout << "Error at connect: " << WSAGetLastError() << '\n';
			this->ConnectSocket = INVALID_SOCKET;
		}
		freeaddrinfo(this->ClientAddr);
		if (this->ConnectSocket == INVALID_SOCKET) {
			cout << "Unable to connect to server!\n";
			WSACleanup();
			return 1;
		}
		else if (this->ConnectSocket != INVALID_SOCKET) cout << "Client connected to server!\n";

		return EXIT_SUCCESS;

	};
	void FillJson(float x, float y, int dir, int charcl, int lives,bool IsAshoot) {
		this->ConfigGame["XP"] = x;
		this->ConfigGame["YP"] = y;
		this->ConfigGame["Dir"] = dir;
		this->ConfigGame["CharacP"] = charcl;
		this->ConfigGame["Health"] = lives;
		this->ConfigGame["Shoot"] = IsAshoot;
	}

	void StartClient() {
		while (this->Triggers.TrigerHandle) {
			SendtoServer();
			std::this_thread::sleep_for(std::chrono::microseconds(50));
			RecvData();
		}
	}
	void SendtoServer() {
		int initResult = 0;
		//this->ConfigGame.dump().length() (nlohmann::to_string(this->ConfigGame)).length()
		initResult = send(this->ConnectSocket, (this->ConfigGame.dump().c_str()), this->ConfigGame.dump().length(), 0);
		if (initResult == SOCKET_ERROR) {
			cout << "send failed: " << WSAGetLastError() << "\n";
			CounterForReconnect++;
			if (CounterForReconnect < 5)
				InitClient(ConfigClient);
			else StopConnect();
			WSACleanup();
		}
		else CounterForReconnect = 0; //cout << "SEND: "<< this->ConfigGame.dump().c_str() << "\n";
	}
	void RecvData() {
		int readBytes = 0;
 
		readBytes = recv(this->ConnectSocket, this->Buffer, sizeof(this->Buffer), NULL);
		//cout << "ReadBytes" << readBytes << "\n";
		if ((readBytes == SOCKET_ERROR) || (readBytes == 0)) {
			cout << "recv Error or connect closed: " << WSAGetLastError() << "\n";
		}
		else {
			
			this->AllDataClients = this->Buffer;
			if (readBytes > this->AllDataClients.length())readBytes = this->AllDataClients.length();
			if (((this->AllDataClients[0] == '{')) && (this->AllDataClients[readBytes-1] == '}')) {
				this->AllDataClients = this->AllDataClients.substr(0, readBytes);
			}
			else this->AllDataClients = "Fail";
		}
	
	};
	std::string GetAllDataClients() {
		return AllDataClients;
	}
	void ShutdownClient() {
		int initResult = 0;
		initResult = shutdown(this->ConnectSocket, SD_SEND);
		if (initResult == SOCKET_ERROR) {
			printf("shutdown failed: %d\n", WSAGetLastError());
			closesocket(this->ConnectSocket);
			WSACleanup();

		}
	}

	void StopConnect() {
		Triggers.TrigerHandle = false;
		closesocket(this->ConnectSocket);
		std::cout << "Client socket closed\n";
		WSACleanup();

	};
};


