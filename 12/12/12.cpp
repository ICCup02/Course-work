
#include <iostream>

using namespace std;
int main()
{
	int i, a, y;
	setlocale(LC_ALL, "RUS");
	cin >> a;
	double c, z=0;
	y = a / 2;
	for (int i = 0, n = 0; y > 1; y--)
	{
		z = a % y;
		if (z == 0)
		{
			cout << "Число состaвное"; y = 0;
		}
	}

	if (y == 1)
			{
			cout << "Число простое";
			}
}