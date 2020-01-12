#include <iostream>
using namespace std;
int main()
{
	setlocale(LC_ALL, "RUS");
	int s, r1, p = 0, r2, l1, l2;
	cin >> s >> l1 >> r1 >> l2 >> r2;
	int c = l1, x = 0, l = abs(r1 - l1) + 1, k = abs(r2 - l2) + 1;
	for (int i = 1, v = l * k; p < v; i++, p++)
	{
		if (s == l1 + l2) { c = l1; x = 1; break; }
		l2++;
		if (i == k)
		{
			l2 = l2 - i ; i = 0; l1++;
		}
	}
	if (x == 1)
	{
		cout << "x1=" << c << endl << "x2=" << s - c;
	}
	else cout << "-1";
	return 0;
}
