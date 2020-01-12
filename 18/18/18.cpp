#include <iostream>
#include <ctime>
using namespace std;

int main()
{
	srand(time(0));
	int arr[20];
	for (int i = 0; i < 20; i++)
	{
		arr[i] = rand() % 201 - 100;
		cout << arr[i] << " ";

	}
	int min=arr[0], max=arr[0];
	for (int i = 1; i < 20; i++)
	{
		if (min > arr[i]) { min = arr[i]; }
		if (max < arr[i]) { max = arr[i]; }
	}
	cout << endl << "min=" << min << endl << "max=" << max;
	//
	//cout << "RAND_MAX = " << RAND_MAX << endl; // константа, хранящая максимальный предел из интервала случайных чисел
	//cout << "random number = " << rand() % 200 - 100 << endl; // запуск генератора случайных чисел
	return 0;
}