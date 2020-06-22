class Bullet:public UnionAllObject {
public:
	Bullet(Image& image, String name, Level& lvl, float X, float Y, int W, int H, int dir) :UnionAllObject(image, name, X, Y, W, H) {

		Direction = dir;
		Speed = 0.35;
		sprite.setTextureRect(IntRect(0, 0, Width, Height));
		if (Direction == 0)x = X + 82;
		else x = X - 42;
		y = Y + 22;

	}

	void update(float time)
	{
		switch (Direction)
		{

		case 0: Dx = Speed; Dy = 0; 
			sprite.setPosition(x, y);
			break;
		case 1: Dx = -Speed; Dy = 0; 
			sprite.setPosition(x, y);
			break;
		}
		x += Dx * time;
		y += Dy * time;

		if ((x <= 0) || (y <= 0)) IsAlife = false;
		if ((x >= 1920) || (y >= 1080))  IsAlife = false;
		
	}
	FloatRect getRect() {

		return FloatRect(x, y, Width, Height);
	}
};