#include <SFML/Graphics.hpp>
#include "view.h"
#include <iostream>
#include <sstream>
#include "tinyxml.h"
#include "level.h"
#include <vector>
#include <list>
#include <time.h>
#include <thread>
#include <chrono>
#include <nlohmann/json.hpp>

////////////////////////////////////////////////////CLASS SERVER////////////////////////
#include"CClient.cpp"
////////////////////////////////////////////////////CLASS UNIONALLOBJECT////////////////////////
#include "CAllObjectUnion.cpp"
//////////////////////////////////CLASS PlayerForClient//////////////////////////////
#include "CPlayer.cpp"
//////////////////////////////////CLASS PLAYER//////////////////////////////
#include "PlayerForClient.cpp"
//////////////////////////////////CLASS BULLET//////////////////////////////
#include "CBullet.cpp"

using namespace sf;
using std::cout;
int Winner(int& Maxplayer, int& Mainplayer, int *Hp) {
	int counter = 0;
	if (Mainplayer > 0)
		counter++;

	for (int i = 0; i < Maxplayer-1; i++)
		if (Hp[i] > 0) counter++;


	if (counter == 1) {
		for (int i = 0; i < Maxplayer-1; i++)
			if (Hp[i] > 0) { return i+2; }

	else if (Mainplayer > 0) { return 1; }
	}
	else return 10;
}
nlohmann::json GameConfig = {
	{"XP"		, 0.0f},
	{"YP"		, 0.0f},
	{"Dir"		, 0	 },
	{"CharacP"	, 0  },
	{"Health"	, 0	 },
	{"Shoot"	,0   }
};
int main(int argc, char* argv[])
{
	setlocale(LC_ALL,
		"RUS");
	std::string NickName = "";
	
	while ((NickName.length() > 7)||(NickName.length()==0)) {
		cout << "Write your Nick,no more than 7: ";
		std::cin >> NickName;
		cout << "\n";
	}

	float xClietns = 0, 
		yClietns = 0,
		timeRecharge = 0.3;
	bool IsAshoot = false;
	int dirClietns = 0, 
		CharacterPersonClietns = 0, 
		counter = 0, 
		health[3] = { 10,10,10 };

	char *IpAddr = new char[15];
	std::cout << "Write Ip adress server: ";
	std::cin >> IpAddr;
	std::cout << "\n";
	ClientConfig ConfigC = { "9889",2,IpAddr };
	CClient Client;
	Client.InitClient(ConfigC);

	RenderWindow window(VideoMode(1920, 1080), "ClashShootRun",sf::Style::Fullscreen);
	view.reset(FloatRect(0, 0, 1600, 800));

	Font font;
	font.loadFromFile("CyrilicOld.TTF");
	Text text("", font, 20);
	text.setStyle(Text::Bold);

	Level lvl;//создали экземпляр класса уровень
	lvl.LoadFromFile("map.tmx");//загрузили в него карту, внутри класса с помощью методов он ее обработывает.

	Image HeroImage;
	HeroImage.loadFromFile("images/Player.png");
	HeroImage.createMaskFromColor(Color(255, 255, 255));

	Text textWins("", font, 22);
	Image Wins;
	Wins.loadFromFile("images/Background.png");
	Wins.createMaskFromColor(Color(255, 255, 255));

	Image BulletImage;
	BulletImage.loadFromFile("images/Bullet.png");
	BulletImage.createMaskFromColor(Color(255, 255, 255));

	Texture textureWins;
	Sprite spriteWin;
	textureWins.loadFromImage(Wins);
	spriteWin.setTexture(textureWins);
	spriteWin.setOrigin(0, 0);

	

	Object player = lvl.Level::GetObjectLevel("Player");

	std::list<UnionAllObject*> AllObject;
	std::list<UnionAllObject*>::iterator itforBullet;
	std::list<PlayerForClient*> players;
	std::list<PlayerForClient*>::iterator itforPlayers;

	Player MainPlayer(HeroImage, NickName, lvl, player.rect.left, player.rect.top, 42, 64, text);

	for (int i = 0; i < ConfigC.Maxplayer - 1; i++)
		players.push_back(new PlayerForClient(HeroImage, ("Player" + std::to_string(i + 2)), lvl, player.rect.left, player.rect.top, 42, 64, text));

	Clock clockForGame, clockForRecharge;

	std::thread StartCl = std::thread(&CClient::StartClient, &Client);
	StartCl.detach();


	while (window.isOpen()) {
		Event event;
		if (Winner(ConfigC.Maxplayer, MainPlayer.Health, health) == 10) {

			float time = clockForGame.getElapsedTime().asMicroseconds();
			float ShootTime = clockForRecharge.getElapsedTime().asSeconds();
			clockForGame.restart();
			time = time / 800;
			
			MainPlayer.update(time);
			Client.FillJson(MainPlayer.getplayercoordinateX(), MainPlayer.getplayercoordinateY(), MainPlayer.Direction, MainPlayer.CharacterPersonage, MainPlayer.Health, MainPlayer.IsAshoot);
			if (ShootTime > timeRecharge) {
				clockForRecharge.restart(); MainPlayer.Recharge = true;
			}

			if ((MainPlayer.Recharge == true) && (timeRecharge >= 2)) {
				timeRecharge = 0.3;
				MainPlayer.Clip = 7;
				clockForRecharge.restart();
			}
			if (MainPlayer.Clip <= 0) {
				timeRecharge = 2;
				MainPlayer.Recharge = false;
			}
			if (MainPlayer.IsAshoot == true) {
				AllObject.push_back(new Bullet(BulletImage, "Bullet", lvl, MainPlayer.getplayercoordinateX(), MainPlayer.getplayercoordinateY(), 18, 5, MainPlayer.Direction));
				Client.FillJson(MainPlayer.getplayercoordinateX(), MainPlayer.getplayercoordinateY(), MainPlayer.Direction, MainPlayer.CharacterPersonage, MainPlayer.Health, MainPlayer.IsAshoot);
				clockForRecharge.restart();
			}
			while (window.pollEvent(event))
			{
				if (event.type == sf::Event::Closed)
					window.close();
			}				

			for (itforPlayers = players.begin(), counter = 0; itforPlayers != players.end(); counter++, itforPlayers++) {

				if ((Client.GetAllDataClients() != "Fail") && (Client.GetAllDataClients() != ""))
				{

					try {
						GameConfig = nlohmann::json::parse(Client.GetAllDataClients());

					}
					catch (nlohmann::detail::type_error ex) {
						try {
							GameConfig = nlohmann::json::parse(Client.GetAllDataClients());
						}
						catch (nlohmann::detail::type_error ex) {
							cout << ex.what();
							cout << "RECV: " << Client.GetAllDataClients() << "d\n";
						}
						
					}
					catch (std::exception ex) {
						try {
							GameConfig = nlohmann::json::parse(Client.GetAllDataClients());
						}
						catch (nlohmann::detail::exception ex) {
							cout << ex.what();
							cout << "RECV: " << Client.GetAllDataClients() << "d\n";
						}
						
					}

					xClietns = GameConfig["XP"].get<float>();
					yClietns = GameConfig["YP"].get<float>();
					dirClietns = GameConfig["Dir"].get<int>();
					CharacterPersonClietns = GameConfig["CharacP"].get<int>();
					health[counter] = GameConfig["Health"].get<int>();
					IsAshoot = GameConfig["Shoot"].get<bool>();

					if (IsAshoot == true)
						AllObject.push_back(new Bullet(BulletImage, "Bullet", lvl, xClietns, yClietns, 18, 5, dirClietns));

					(*itforPlayers)->Update(time, xClietns, yClietns, CharacterPersonClietns, dirClietns, health[counter]);

					for (itforBullet = AllObject.begin(); itforBullet != AllObject.end();)//говорим что проходимся от начала до конца
					{
						UnionAllObject* b = *itforBullet;//для удобства, чтобы не писать (*itforBullet)->
						b->update(time);//вызываем ф-цию update для всех объектов (по сути для тех, кто жив)
						if (((b->getRect().left) <= xClietns + 40) && ((b->getRect().left) >= xClietns) &&
							(abs(yClietns - b->getRect().top) < 64) && (b->getRect().top - yClietns >= 0)) {
							b->IsAlife = false;
						}

						if (b->IsAlife == false) { itforBullet = AllObject.erase(itforBullet); delete b; }// если этот объект мертв, то удаляем его
						else itforBullet++;//и идем курсором (итератором) к след объекту. так делаем со всеми объектами списка
					}

				}

			}


			for (itforBullet = AllObject.begin(); itforBullet != AllObject.end();)//говорим что проходимся от начала до конца
			{
				UnionAllObject* b = *itforBullet;//для удобства, чтобы не писать (*itforBullet)->
				b->update(time);//вызываем ф-цию update для всех объектов (по сути для тех, кто жив)
				
				if (b->Direction == 1) {
					if (((b->getRect().left) <= MainPlayer.getRect().left + 40) && ((b->getRect().left) >= MainPlayer.getRect().left) &&
						(abs(MainPlayer.getRect().top - b->getRect().top) < 64) && (b->getRect().top - MainPlayer.getRect().top >= 0)) {
						MainPlayer.BulletHit = -6; b->IsAlife = false;
					}
				}
				else if (b->Direction == 0) {
					if (((b->getRect().left) <= MainPlayer.getRect().left + 40) && ((b->getRect().left) >= MainPlayer.getRect().left) &&
						(abs(MainPlayer.getRect().top - b->getRect().top) < 64) && (b->getRect().top - MainPlayer.getRect().top >= 0)) {
						MainPlayer.BulletHit = 6; b->IsAlife = false;
					}
				}


				if (b->IsAlife == false) { itforBullet = AllObject.erase(itforBullet); delete b; }// если этот объект мертв, то удаляем его
				else itforBullet++;//и идем курсором (итератором) к след объекту. так делаем со всеми объектами списка
			}
			window.setView(view);
			window.clear(Color(255, 255, 255));
			lvl.Draw(window);

			window.draw(MainPlayer.sprite);
			window.draw(MainPlayer.NickName1);//рисую текст
			window.draw(MainPlayer.Icon);
			window.draw(MainPlayer.ClipForIcon);
			window.draw(MainPlayer.HealthForIcon);
			window.draw(MainPlayer.NickName2);
			if (MainPlayer.getplayercoordinateY() < 0)window.draw(MainPlayer.arrow);

			for (itforPlayers = players.begin(); itforPlayers != players.end(); itforPlayers++) {
				(*itforPlayers)->SetPlayerGrafic(MainPlayer.getplayercoordinateX(), MainPlayer.getplayercoordinateY());
				window.draw((*itforPlayers)->NickName1);//рисую текст
				window.draw((*itforPlayers)->Icon);
				window.draw((*itforPlayers)->ClipForIcon);
				window.draw((*itforPlayers)->HealthForIcon);
				window.draw((*itforPlayers)->NickName2);
				window.draw((*itforPlayers)->sprite);
				if (yClietns < 0) window.draw((*itforPlayers)->arrow);
			}
			for (itforBullet = AllObject.begin(); itforBullet != AllObject.end(); itforBullet++) {
				window.draw((*itforBullet)->sprite);
			}
			window.display();
		}
		else {

			while (window.pollEvent(event))
			{
				if (event.type == sf::Event::Closed)
					window.close();
			}
			window.draw(spriteWin);
			if (Winner(ConfigC.Maxplayer, MainPlayer.Health, health)==1) textWins.setString("You Win!");
			else textWins.setString(std::to_string(Winner(ConfigC.Maxplayer, MainPlayer.Health, health)) + " !!!!");

			textWins.setFillColor(Color(255, 255, 0));
			textWins.setPosition(985, 228);
			window.draw(textWins);
			window.display();
		}


	}

	return EXIT_SUCCESS;
}

