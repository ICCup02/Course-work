#include<iostream>
#include <climits>
using namespace std;
int main() {
	setlocale(LC_ALL, "RUS");
	long long int a, b = 1;
	cin >> a;
	if (a > 20)
	{
		cout << "Не могу посчитать факториал числа больше чем 20" << endl;
	}
	else
		if (a == 0)
		{
			cout << "0";
		}
		else 
		{
			for (int i = 0, c = a; c > 1; i++, c = c - 1)
			{
				b = b * c;
				
			}
			cout << b;
		}

}