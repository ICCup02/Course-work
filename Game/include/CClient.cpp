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

	
	void FillJson(float x, float y, int dir, int charcl, int lives, bool IsAshoot) {
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
		initResult = send(this->ConnectSocket, (this->ConfigGame.dump().c_str()),this->ConfigGame.dump().length(), 0);
		if (initResult == SOCKET_ERROR) {
			cout << "send failed: " << WSAGetLastError() << "\n";
			CounterForReconnect++;
			if (CounterForReconnect < 5)
				InitClient(ConfigClient);
			else StopConnect();
			WSACleanup();
		}
		else CounterForReconnect = 0;
	}
	void RecvData() {
		int readBytes = 0; 
		readBytes = recv(this->ConnectSocket, this->Buffer, sizeof(this->Buffer), NULL);
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


