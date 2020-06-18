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
				else cout << "Hey it's client buffer: " << this->Clients[i].Buffer << "\n";
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
			this->Clients[i].CAddrlen=sizeof(this->Clients[i].CAddr);
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




