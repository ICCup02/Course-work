#include <iostream>
#include<limits>
using namespace std;
int main()
{

	int arr[6] = { INT_MAX,	INT_MAX,INT_MAX,INT_MAX,INT_MAX }, a;
	cin >> a;
	for (int i = 4, k = 0; k < a; k++)
	{
		cin >> arr[5];
		if (arr[5] <= arr[i]) {
			for (int u = i; u >= 0; u--)
			{
				swap(arr[5], arr[u]);
			}
		}
		else {
			for (int u = i; u > 0; u--)
			{
				if ((arr[5] < arr[u - 1]) && (arr[5] >= arr[u]))
				{
					swap(arr[5], arr[u - 1]);
				}
			}
		}
		

		for (int y = ( 4 - k < 0)?0:4-k; y < 5; y++)
		{
			cout << arr[y] << " ";
		}
		cout << endl;
	}
}