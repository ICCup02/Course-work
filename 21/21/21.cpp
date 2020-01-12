#include <iostream>
#include <time.h>
using namespace std;
int main() {
	setlocale(LC_ALL, "RUS");
	srand(time(NULL));
	int arr[10][8] = { {} };
	int k = rand() % 10 + 1;
	int n = rand() % 8 + 1;
	/*cout << k << " " << n << endl;*/
	arr[k - 1][n - 1] = 1;
	/*cout << arr[k - 1][n - 1] << endl;*/
	/*for (int i = 0, p = 0, l = 0; i < 80; l++, i++)
	{
		cout << arr[p][l] << " ";
		if (l == 7) { l = -1; p++; cout << endl; };
	}*/
	int s, d;
	for (int i = 0; i < 5; i++)
	{
		cin >> s >> d;
		if ((s >9) || (d > 7) || (s < 0) || (d < 0)) {
			cout << " Выход за границу массива\n"; i--;
		}
		else if (arr[s][d] == arr[k - 1][n - 1]) { cout << "Вы угадали\n"; i = 5; break; }
		else if (arr[s][d] == 2) { cout << " Эту уже проверяли\n"; i--; }
		else if (i == 4) { cout << "Вы проиграли\n"; }
		else { cout << "NO\n"; arr[s][d] = 2; }
	}

}