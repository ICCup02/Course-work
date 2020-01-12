#include <iostream>
using namespace std;
void printBMI(double BMI)
{
	if (BMI < 18.5) {
		cout << "Underweight";
	}
	else
		if (BMI < 25) {
			cout << "Normal weight ";
		}
		else
			if (BMI < 30) {
				cout << "Overweight";
			}
			else
				if (BMI >= 30) {
					cout << "Obesity";
				}

}
double BMI(double weight, double height)
{

	double BMI = weight / (height * height);
	return BMI;
}


int main()
{
	double a, b;
	cout << "Your weight ";
	cin >> a;
	cout << "\nYour height ";
	cin >> b;
	b = b / 100;
	cout << endl;
	printBMI(BMI(a, b));
}