#include <iostream>
#include<ctime>

int main()
{
	int i = 0;
	setlocale(LC_ALL, "RUS");
	srand(time(NULL));
	int a, n = rand() % 101;
	std::cout << "Приветствую. Давай сыграем в игру,попробуй угадать мое число от 0 до 100" << std::endl;

	for (int i = 0; i < 5; i++)
	{
		std::cin >> a;
		if ((a < n - 20) && (i < 4))
		{
			std::cout << "Загаданное число больше" << std::endl;
		}
		else
			if ((a > n + 20) && (i < 4))
			{
				std::cout << "Загаданное число меньше" << std::endl;
			}
			else
				if ((a == n) && (i < 4))
				{
					std::cout << "Поздравляю вы угадали" << std::endl << "Хотите сыграть еще(1-Да)" << std::endl;
					int g;
					std::cin >> g;
					if (g == 1)
					{
						std::cout << "Вводите число ";
						i = -1;
						n = rand() % 101;
					}
				}
				else
					if (i < 4)
					{
						std::cout << "Вы уже близко" << std::endl;
					}
		if (i == 4)
		{
			std::cout << "Вы проиграли. Было загадано " << n << std::endl << "Хотите сыграть еще(1-Да)" << std::endl;
			int g; std::cin >> g;
			if (g == 1)
			{
				n = rand() % 101;
				std::cout << "Вводите число "; i = -1;
			}

		}
	}
	std::cout <<i;

	return 0;

}