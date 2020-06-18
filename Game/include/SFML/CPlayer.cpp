class Player :public UnionAllObject {

public:
	enum {
		RIGHT,
		LEFT,
		JUMP,
		DOWN,
		STAY,
		SHOOT,
		BULLETHIT,
	} CharacterPersonage;
	bool OnGround, IsAshoot, Recharge;
	Text NickName1, NickName2, HealthForIcon, ClipForIcon;
	Texture textureArrow;
	float TimeRecharge, BulletHit, Falling;
	Sprite arrow, Icon;
	int  Health, Clip, Frame;//PlayerScore; если захочу реализовать смену ружь€ (не захочу так как не умею рисовать)

	Player(Image& image, String name, Level lvl, float X, float Y, int W, int H, Text playertext) :UnionAllObject(image, name, X, Y, W, H) {
		//ћј —»ћјЋ№Ќџ… Ќ»  8 —»ћ¬ќЋќ¬
		Health = 5;
		IsAshoot = false;
		Clip = 7;
		BulletHit = 0;
		arrow.setTexture(texture);
		arrow.setOrigin(0, 0);
		arrow.setTextureRect(IntRect(230, 0, 30, 29));

		Icon.setTexture(texture);
		Icon.setOrigin(0, 0);
		Icon.setTextureRect(IntRect(280, 0, 200, 75));
		Icon.setPosition(500, 500);

		sprite.setTextureRect(IntRect(32, 18, Width, Height));
		CharacterPersonage = STAY;
		lvl.LoadFromFile("map.tmx");
		obj = lvl.GetAllObjects();

		NickName1 = NickName2 = playertext;
		ClipForIcon = playertext;
		HealthForIcon = playertext;

		Falling = 0;

		NickName1.setString(Name);
		NickName2.setString(Name);
	}
	void update(float time)
	{
		//	std::cout <<"AFTeR:    " << getplayercoordinateY() << "\n";
		move(time);
		switch (CharacterPersonage)
		{
		case JUMP:
			Dx = 0;
			OnGround = false; break;
		case RIGHT:
			Dx = Speed + BulletHit; break;
		case LEFT:
			Dx = Speed + BulletHit; break;
		case STAY:
			if (Direction == RIGHT) {
				sprite.setTextureRect(sf::IntRect(18 + 48 * int(frame_change(time, 2, 0.002)), 18, 48, 64));
			}
			if (Direction == LEFT) {
				Width = 30;
				sprite.setTextureRect(sf::IntRect(167 + 49 * int(frame_change(time, 2, 0.002)), 112, 48, 64));
			}
			if (Speed > 0)Dx = Speed - 0.2 + BulletHit;
			else Dx = Speed + 0.2 + BulletHit;
			IsMove = false; break;
		case DOWN:
			if (y < 810) {
				Falling = 0;
			}
			else CharacterPersonage = STAY;
			break;
		case SHOOT:
			if (Direction == RIGHT) {
				sprite.setTextureRect(sf::IntRect(26 + 100 * int(frame_change(time, Frame, 0.005)), 494, 70, 64));
			}
			if (Direction == LEFT) {
				Width = -16; //if (OnGround==true)Dy = 0;
				sprite.setTextureRect(sf::IntRect(2 + 100 * int(frame_change(time, Frame, 0.005)), 398, 70, 64));
			}
			if (Speed > 0)Dx = Speed - 0.2 + BulletHit;
			else Dx = Speed + 0.2 + BulletHit;
			break;
		}
		if (BulletHit > 0)BulletHit += 0.5;
		if (BulletHit < 0)BulletHit -= 0.5;
		if (abs(BulletHit) > 4)BulletHit = 0;
		//std::cout << "time:" << time << "\n";
		//std::cout << "time    :" << time << '\n';
		x += Dx * time;
		y += Dy * time;
		//std::cout << "BEFORE:    "  << getplayercoordinateY() << "\n";
		check_Collision_With_Map(Dy);
		sprite.setPosition(x + Width / 2, y + Height / 2);
		arrow.setPosition(x, 0);

		HealthForIcon.setString(std::to_string(Health));
		if (Clip == 0) { ClipForIcon.setString("RG"); }
		else ClipForIcon.setString(std::to_string(Clip));
		NickName1.setPosition(x - 2 * Name.getSize(), y - 25);//задаем позицию текста, отступа€ от центра камеры

		setPlayerCoordinateForViewAndIcon(x, y, Icon, HealthForIcon, ClipForIcon, NickName2);
		if (!OnGround) { Dy = Dy + 0.001 * time; }

		//if (Health <= 0) { setPlayerCoordinateForView(x, y); }//если жизней меньше либо равно 0, то умираем

	}
	float getplayercoordinateX() {
		return x;
	}
	float getplayercoordinateY() {
		return y;
	}
	void check_Collision_With_Map(float& dy)//ф ци€ проверки столкновений с картой
	{
		OnGround = false;
		//if (Bullet.getRect() )
		for (int i = 0; i < obj.size(); i++)//проходимс€ по объектам
			if (getRect().intersects(obj[i].rect))//провер€ем пересечение игрока с объектом
			{

				//std::cout << "SSS\nx:" << x << "\ny:" << y << obj[i].name << "\n";
				if (obj[i].name == "Solid")
				{
					//если встретили преп€тствие
					//std::cout << "\nx:" << x << "\ny+h:" << (static_cast<int>(y)) + Height << "obj:" << obj[i].rect.top << "\n";
					//std::cout << "\nx:" << x << "\ny:" << y << obj[i].rect.top << "\n";
					//if ((y + Height - obj[i].rect.top < 10)  && (dy > 0)) { y = obj[i].rect.top - Height;  Dy = 0; OnGround = true; }
					//else if((CharacterPersonage == DOWN) && (int(y) != 864)) { y += 32; CharacterPersonage = JUMP; }
					//if ((y + Height - obj[i].rect.top > 10) && (y + Height - obj[i].rect.top < 32)  && (dy > 0)) { y = obj[i].rect.top - Height;  Dy = 0; OnGround = true; }
					//else if ((CharacterPersonage == DOWN) && (int(y) != 864)) { y += 32; CharacterPersonage = JUMP; }*/
					//if ((Y- obj[i].rect.top < 32) && (Y + Height - obj[i].rect.top > 10)) {
						//if ((CharacterPersonage != DOWN) && (dy > 0)) { y = obj[i].rect.top - Height;  Dy = 0; OnGround = true; }
						//else if ((CharacterPersonage == DOWN)) { y += 25; CharacterPersonage = JUMP; }
					//}
					//	else
					//		//if (y + Height - obj[i].rect.top < 10) {
					if ((Falling <= 1) && (Falling > 0)) { Falling = 0; OnGround = false; }//0.6 sec
					else if ((CharacterPersonage != DOWN) && (dy > 0)) { y = obj[i].rect.top - Height;  Dy = 0; OnGround = true; }
					else if ((CharacterPersonage == DOWN) && (y < obj[i].rect.top - Height)) { CharacterPersonage = JUMP; }
					
				}
				else if (obj[i].name == "Dead") {

					if ((y - obj[i].rect.top < 0) || (x - obj[i].rect.left < 0)) {
						x = 912; y = -612; Dy = 0; Health--; Clip = 7; if (Health == 0)std::cout << "YOU DEAD!";
					}

				}
			}
	}

	void move(float& time) {

		CharacterPersonage = STAY;

		Width = 44;
		if (Keyboard::isKeyPressed(Keyboard::Left)) {
			CharacterPersonage = LEFT;
			Speed = -0.2; Direction = LEFT; Width = 44;
			sprite.setTextureRect(sf::IntRect(22 + 96 * int(frame_change(time, 8, 0.005)), 208, 44, 64)); Height = 64;
		}
		if (Keyboard::isKeyPressed(Keyboard::Right)) {
			CharacterPersonage = RIGHT;
			Speed = 0.2; Direction = RIGHT;
			sprite.setTextureRect(sf::IntRect(30 + 96 * int(frame_change(time, 8, 0.005)), 304, 44, 64)); Height = 64;
		}
		if ((Keyboard::isKeyPressed(Keyboard::Up)) && (OnGround)) {
			CharacterPersonage = JUMP;
			Dy = -0.55; OnGround = false;
		}
		if ((Keyboard::isKeyPressed(Keyboard::Down)) && (OnGround)) {
			CharacterPersonage = DOWN;
		}
		if (Keyboard::isKeyPressed(Keyboard::Z)) {
			if ((Recharge == true) && (Clip > 0)) {
				IsAshoot = true;
				CharacterPersonage = SHOOT;
				Recharge = false;
				Clip--;
				CharacterPersonage = SHOOT;
				Frame = 4;
			}
			else if ((Recharge == false) && (IsAshoot == true)) { IsAshoot = false; CharacterPersonage = SHOOT; Frame = 1; }
			else if (Clip <= 0) CharacterPersonage = STAY;
			else { Frame = 4; CharacterPersonage = SHOOT; }
		}

	};
};