class Bullet :public UnionAllObject {//класс пули
public:
	Bullet(Image& image, String name, Level& lvl, float X, float Y, int W, int H, int dir) :UnionAllObject(image, name, X, Y, W, H) {//всё так же, только взяли в конце состояние игрока (int dir)
	//инициализируем .получаем нужные объекты для взаимодействия пули с картой
		Direction = dir;
		Speed = 0.45;
		sprite.setTextureRect(IntRect(0, 0, Width, Height));
		if (Direction == 0)x = X + 82;
		else x = X - 42;
		y = Y + 22;
		//выше инициализация в конструкторе
	}

	void update(float time)
	{
		switch (Direction)
		{

		case 0: Dx = Speed; Dy = 0; //интовое значение state = right  
			sprite.setPosition(x, y);//72 32
			break;
		case 1: Dx = -Speed; Dy = 0; //интовое значение state = left
			sprite.setPosition(x, y);
			break;
		}
		x += Dx * time;//само движение пули по х
		y += Dy * time;//по у

		if ((x <= 0) || (y <= 0)) IsAlife = false;// задержка пули в левой стене, чтобы при проседании кадров она случайно не вылетела за предел карты и не было ошибки
		if ((x >= 1920) || (y >= 1080))  IsAlife = false;
		
	}
	FloatRect getRect() {//ф-ция получения прямоугольника. его коорд,размеры (шир,высот).

		return FloatRect(x, y, Width, Height);//эта ф-ция нужна для проверки столкновений 
	}
};