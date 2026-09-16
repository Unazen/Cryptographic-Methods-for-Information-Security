#include <iostream>

using namespace std;
// temp
int main()
{
	string text;
	int shift;

	cout << "Enter text to encrypt: ";
	getline(cin, text);
	cout << "Enter shift value: ";
	cin >> shift;

	for (char &c : text)
	{
		if (isalpha(c))
		{
			char base = islower(c) ? 'a' : 'A';
			c = (c - base + shift) % 26 + base;
		}
	}

	cout << "Encrypted text: " << text << endl;

	return 0;
}