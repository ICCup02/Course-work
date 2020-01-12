#include <iostream>
using namespace std;
void print(int* arr, int n) {
	for (int i = 0; i < n; i++)
	{
		cout << arr[i] << " ";
	}
}
void buble(int *arr, int n) {
	bool p = true;
	
	while (p) 
	{
		p = false;
		for (int i = 1, min = 0; i<n; i++, min++)
		{
			if (arr[min] > arr[i]) { swap(arr[min], arr[i]); p = true; }
		}
		n--;
	}
}
int main()
{
	int n;
	cin >> n;
	int* arr = new int[n];
	for (int i = 0; i < n; i++)
	{
		cin >> arr[i];
	}
	buble(arr, n);
	print(arr, n);
	delete[] arr;
	return 0;
}

