#include<iostream >
using namespace std;
int main() {
	double a, b, x = 1;
	cin >> a >> b;
	if (b >= 1) {
		for (int i = 0; i < b; i++) {
			x *= a;
		}
	}
	if (b <= 1) {
		for (int i = 0; i > b; i--) {
			x = (x * a);
		}
		if (x == 0) { cout << "sdsd";  return 0; }
		x = 1 / x;
	}
	cout << x;
	return 0;
}