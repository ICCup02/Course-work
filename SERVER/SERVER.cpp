#include <winsock2.h>
#include <iostream>
#include <windows.h>
#include <WS2tcpip.h>
#include "CServer.cpp"
#pragma comment(lib, "ws2_32.lib")
using std::cout;

int main() {

	CServer Server;

	int RestartOrEndGame=0;

	ConfigServer ServerConfig{"9889",2};
	
	Server.InitServer(ServerConfig);
	
	Server.StartServer();

	std::cout << "Write 1 for restart game, or write 2 for completion game\n";

	while (RestartOrEndGame == 0) {

		std::cin >> RestartOrEndGame;
		if (RestartOrEndGame == 1) {
			std::cout << "Has the number of players changed?(if yes write number players or no write number less than 2 or more than 4)\n";
			std::cin >> RestartOrEndGame;
			if ((RestartOrEndGame < 4) && (RestartOrEndGame > 2)) {
				ServerConfig.MaxPlayer = RestartOrEndGame;
			}
			RestartOrEndGame = 0;
			Server.RestartServer(ServerConfig);
		}
		else if (RestartOrEndGame == 2) { Server.StopServer(); }
	}
	return EXIT_SUCCESS;
}
