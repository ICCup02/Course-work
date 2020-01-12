#include <iostream>
#include <string>
using namespace std;


bool is_prime(long long int N) {
	for (int i = 2; i < N; i++) {
		if (N % i == 0) {
			return false;
		}
	}
	return true;
}

bool factorization(long long int N, string& result_str) {
	int k = 1;
	for (int i = 2; i <= N; i++) {
		if (N % i == 0) {
			k = 1;
			N /= i;
			while (N % i == 0) {
				k++;
				N /= i;
			}
			if (k != 1) result_str += to_string(i) + "^" + to_string(k) + "*";
			else result_str += to_string(i) + "*";
		}
	}
	result_str[result_str.length() - 1] = '\0';
	return true;
}

int main()
{
	setlocale(LC_ALL, "Russian");
	long long int num;
	string result;
	int k = 1;
	cout << "Введите N для факторизации: ";
	cin >> num;

	factorization(num, result);
	cout << result;
	return 0;
}