#include <iostream>
#include<string>
size_t len(const char* string)
{
	int i = 0;
	while (string[i] != '\0')
	{
		i++;
	}
	return i;
}
int cmp(const char* string1, const char* string2)
{
	int i = 0;
	while ((string1[i] == string2[i]) && (string1[i] != '\0') && (string2[i] != '\0'))
	{
		i++;
	}
	if ((string1[i] == string2[i]) && (string1[i] == '\0') && (string2[i] == '\0')) { return 0; }
	if (((string1[i] == '\0') && (string2[i] != '\0')) || ((string1[i + 1] == '\0') && (string2[i + 1] == '\0') && ((int(string2[i])) > int(string1[i]))))
	{
		return -1;
	}
	else return 1;
}
int str(char* string1, const char* string2)
{
	int i = 0;
	int count = 0, p;
	while (len(string1) > i) 
	{
		if (string1[i] == string2[count]) { count++; }
		else if ((count != len(string2)) && (string1[i] != string2[count])) { count = 0; }
	
		if (count == len(string2)) { i++; break; }
		i++;
	}
	if ((count != len(string2)) && (string1[i] != string2[count])) { count = i + 1; }
	return i-count;
	
}
char* cat(char* combination, const char* srcptr)
{
	int max = len(combination) + len(srcptr) + 1;
	int one_longitude = len(combination);
	combination[one_longitude] = ' ';
	for (int i = one_longitude + 1, count_srcptr = 0; i <= max; i++, count_srcptr++)
	{
		combination[i] = srcptr[count_srcptr];
	}
	combination[max] = '\0';
	return combination;
}
int main()
{
	setlocale(LC_ALL, "RUS");
	char* one = new char[50];
	std::cin.getline(one, len(one));

	char* two = new char[50];
	std::cin.getline(two, len(two));
	len(one);
	std::cout << cmp(one, two);
	std::cout << std::endl;
	std::cout << str(one, two);
	std::cout << std::endl;
	//std::cout << strstr(one, two) << std::endl;
	std::cout << cat(one, two);
}
