#include <iostream>
using namespace std;
int main()
{	
	int n;
	cin >> n;
	int* arr = new int[n];
	for (int i = 0; i < n; i++)
	{
		cin >> arr[i];
	}
	for (int i = 0,l=0,c=0, k = 0; i < 9*n; l++,i++)
	{
		if (arr[l] == k) {
			c++;
		}
		if (l == n-1) {
			if (c > 0) { cout << k << ":" << c<<endl; }
			c = 0;
			k++;
			l = -1;
		}
	}
	delete[] arr;
}
