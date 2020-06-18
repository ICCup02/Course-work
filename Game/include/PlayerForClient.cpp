
class PlayerForClient{

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

	float x, y;
	int Width, Height, Direction, Health, Clip, Frame;;
	Texture texture,textureArrow;;
	Sprite sprite;
	String Name;
	Text NickName1, NickName2, HealthForIcon, ClipForIcon;
	Sprite arrow, Icon;

	

	PlayerForClient(Image& image, String name, Level lvl, float X, float Y, int W, int H, Text playertext) {
		//МАКСИМАЛЬНЫЙ НИК 8 СИМВОЛОВ
		x = X; y = Y; Width = W; Height = H;  //MoveTimer = 0;
		Name = name; Direction = 1;
		Health = 10;
		this->texture.loadFromImage(image);
		this->sprite.setTexture(texture);
		this->sprite.setOrigin(W / 2, H / 2);
	

		arrow.setTexture(texture);
		arrow.setOrigin(0, 0);
		arrow.setTextureRect(IntRect(230, 0, 30, 29));

		Icon.setTexture(texture);
		Icon.setOrigin(0, 0);
		Icon.setTextureRect(IntRect(280, 0, 175, 75));
		Icon.setPosition(500, 500);

		sprite.setTextureRect(IntRect(32, 18, Width, Height));
		CharacterPersonage = STAY;


		NickName1 = NickName2 = playertext;
		ClipForIcon = playertext;
		HealthForIcon = playertext;

		NickName1.setString(Name);
		NickName2.setString(Name);
		sprite.setPosition(x + Width / 2, y + Height / 2);
		arrow.setPosition(x, 0);
	
		HealthForIcon.setString(std::to_string(Health));
		ClipForIcon.setString("∞");
		NickName1.setPosition(x - 2 * Name.getSize(), y - 25);//задаем позицию текста, отступая от центра камеры
	}
	void Update(float time, float& X, float& Y,int &CharacterPers, int& dir,int &health) {
		x = X; y=Y;
		Direction = dir;
		Health = health;
		switch (CharacterPers) {
		case JUMP:
			break;
		case RIGHT:
			Direction = RIGHT;
			sprite.setTextureRect(sf::IntRect(30 + 96 * int(frame_change(time, 8, 0.005)), 304, 44, 64)); Height = 64;
			break;

		case LEFT:
			Direction = LEFT;
			sprite.setTextureRect(sf::IntRect(22 + 96 * int(frame_change(time, 8, 0.005)), 208, 44, 64)); Height = 64;
			break;

		case STAY:
			if (Direction == RIGHT) {
				sprite.setTextureRect(sf::IntRect(18 + 48 * int(frame_change(time, 2, 0.002)), 18, 48, 64));
			}
			if (Direction == LEFT) {
				Width = 30;
				sprite.setTextureRect(sf::IntRect(167 + 49 * int(frame_change(time, 2, 0.002)), 112, 48, 64));
			}
			break;
		case DOWN:
			break;
		case SHOOT:
			if (Direction == RIGHT) {
				sprite.setTextureRect(sf::IntRect(26 + 100 * int(frame_change(time, 4, 0.006)), 494, 70, 64));
			}
			if (Direction == LEFT) {
				Width = -16; //if (OnGround==true)Dy = 0;
				sprite.setTextureRect(sf::IntRect(2 + 100 * int(frame_change(time, 4, 0.006)), 398, 70, 64));
			}
			break;

		}
	

	};
	float frame_change(float& time, int quantityFrame, double speedFrameChange) {
		static float Current_Frame = 0;
		Current_Frame += speedFrameChange * time;
		if (Current_Frame > quantityFrame) { Current_Frame = 0; }
		return Current_Frame;
	};
	void SetPlayerGrafic(float XMainPlayer, float YMainPlayer) {
		/*XfordrawIcon = XMainPlayer;
		YfordrawIcon = YMainPlayer;*/
		sprite.setPosition(x + Width / 2, y + Height / 2);
		
		if (Health <= 0)HealthForIcon.setString("D");
		else HealthForIcon.setString(std::to_string(Health));
		//if (Clip == 0) { ClipForIcon.setString("RG"); }
		//else ClipForIcon.setString(std::to_string(Clip));
		ClipForIcon.setString("∞");
		arrow.setPosition(x, 0); 
		NickName1.setPosition(x - 2 * Name.getSize(), y - 25);//задаем позицию текста, отступая от центра камеры
		setPlayerCoordinateForViewAndIcon(XMainPlayer, YMainPlayer, Icon, HealthForIcon, ClipForIcon, NickName2, 1);
		//if (Health <= 0) { setPlayerCoordinateForView(x, y); }//если жизней меньше либо равно 0, то умираем
	
	
	}
};