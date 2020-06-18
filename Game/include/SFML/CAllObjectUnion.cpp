

class UnionAllObject {
public:
	std::vector<Object> obj;
	float  Speed, Dx, Dy, x, y;//, MoveTimer;
	int Width, Height, Direction;//направление пули
	bool IsAlife, IsMove;
	Texture texture;
	Sprite sprite;
	String Name;
	UnionAllObject(Image& image, String name, float X, float Y, int W, int H) {
		x = X; y = Y; Width = W; Height = H;  //MoveTimer = 0;
		Speed = 0.2;  Dx = 0; Dy = 0; Name = name; Direction = 1;
		IsAlife = true;  IsMove = false;
		this->texture.loadFromImage(image);
		this->sprite.setTexture(texture);
		this->sprite.setOrigin(W / 2, H / 2);
	}
	virtual FloatRect getRect() {//ф-ция получения прямоугольника. его коорд,размеры (шир,высот).
		return FloatRect(x, y, Width, Height);//эта ф-ция нужна для проверки столкновений 
	}
	virtual void update(float time) = 0;
	float frame_change(float& time, int quantityFrame, double speedFrameChange) {
		static float Current_Frame = 0;
		Current_Frame += speedFrameChange * time;
		if (Current_Frame > quantityFrame) { Current_Frame = 0; }
		return Current_Frame;
	};
};
