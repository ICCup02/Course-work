#include <iostream>
using namespace std;
int main()
{
	setlocale(LC_ALL, "RUS");
		double a, b;
	
	char c;
	cin >> a >> c >> b;
	
	switch (c)
	{
	case'+': cout << a + b;
		break;
	case'*': cout << a * b;
		break;
	case'/': if (b == 0) { cout << "Ошибка"; } else cout << a / b;
		 break;
	case'-': cout << a - b;
		break;
	default:cout << "Не правильно введены значения";

	}
}
