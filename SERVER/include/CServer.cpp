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
	u_long iMode = 0;
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
		initResult = ioctlsocket(this->MainSocket, FIONBIO, &iMode);
		if (initResult != NO_ERROR)
			printf("ioctlsocket failed with error: %ld\n", initResult);

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
		else std::cout << "Server in wiretap mode\n";
		std::thread AcceptThread = std::thread(&CServer::AcceptConnections, this);
		AcceptThread.detach();
		return 0;

	}
	void HandleConnections() {

		while (this->Triggers.TrigerHandle) {
			for (int i = 0; i < this->ServerConfig.MaxPlayer; i++) {

				int byteslenrecv;
				std::this_thread::sleep_for(std::chrono::microseconds(75));
				byteslenrecv = recv(this->Clients[i].ClientSocket, 
					this->Clients[i].Buffer, 
					sizeof(this->Clients[i].Buffer), 
					NULL);

				if ((byteslenrecv == SOCKET_ERROR) || (byteslenrecv == 0)) {
					closesocket(this->Clients[i].ClientSocket);

				}
				else {
					std::string temp = this->Clients[i].Buffer;
					if (byteslenrecv > temp.length())byteslenrecv = temp.length();
					if ((temp[0] == '{') && (temp[byteslenrecv - 1] == '}') && (byteslenrecv > 0) && (temp.length() > 0)) {
						this->Clients[i].DataClients = temp.substr(0, byteslenrecv);

						FillDataClients(i, this->ServerConfig.MaxPlayer);						
						send(this->Clients[i].ClientSocket, this->Clients[i].AllDataClients.c_str(), 
							this->Clients[i].AllDataClients.length(), NULL);

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
				cout << i + 1 << " client Connected!\n";
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
		std::cout << "Server closed";
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



