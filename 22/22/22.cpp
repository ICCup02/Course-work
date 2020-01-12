#include <iostream>
using namespace std;
int o = 1;
void SWAP(int*a, int*b) {

	int k;
	k = *a;
	*a = *b;
	*b = k;

}
void info(const int*p) {
	cout << "Адрес:" << p << "; Значение:" << *p << endl;

}
int msort(int*a, int*b, int*c)
{		
	if ((*a >= *b)&&(*a >= *c)){ 
		if (*b >= *c) { SWAP(b, c); } SWAP(a, c);
	}else 
	if ((*c >= *b) && (*c >= *a)) {
		if (*a >=*b ) { SWAP(a, b); }
	}else
	{
		if (*a >= *c) { SWAP(c, a); } SWAP(b, c);
	}
			return *c;
}
int* add(int*a, const int*p)
{
	*a = *a + *p;
	return a;
}

int main()
{
	setlocale(LC_ALL, "RUS");

	int a = 1, b = 2, c = 3;
	
	info(&a);
	info(&b);
	info(&c);
	cout << "Максимум:" << msort(&a, &b, &c);
	cout << endl;
	add(&a, &b);

	info(add(&a, &c));
	
}
