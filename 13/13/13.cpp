#include <iostream>
using namespace std;
int main()
{setlocale(LC_ALL, "rus");
int a,y=0;
	cin >> a;
	if (a == 0)
	{
		cout << "0";
	}
	
	else	{
				while (pow(2, y) <= a)
				{
				y = y + 1;
				}
				cout << y;
			}
}
