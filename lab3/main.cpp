#include "belt.hpp"

#include <algorithm>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <iterator>
#include <string>
#include <vector>

using namespace std;

static vector<uint8_t> readFile(const string &filename)
{
	ifstream file(filename, ios::binary);

	if (!file)
		throw runtime_error("Cannot open input file");

	return vector<uint8_t>(
		istreambuf_iterator<char>(file),
		istreambuf_iterator<char>());
}

static bool writeFile(
	const string &filename,
	const vector<uint8_t> &data)
{
	ofstream file(filename, ios::binary);

	if (!file)
		return false;

	file.write(
		reinterpret_cast<const char *>(data.data()),
		static_cast<streamsize>(data.size()));

	return file.good();
}

static void printHex(
	const string &label,
	const vector<uint8_t> &data)
{
	cout << label;

	for (uint8_t byte : data)
	{
		cout << hex << uppercase
			 << setw(2) << setfill('0')
			 << static_cast<unsigned>(byte);
	}

	cout << dec << setfill(' ') << '\n';
}

static vector<uint8_t> toBytes(const string &text)
{
	return vector<uint8_t>(text.begin(), text.end());
}

static vector<uint8_t> readHex(const string &prompt, size_t size)
{
	string hexText;

	cout << prompt;
	cin >> hexText;

	if (hexText.size() != size * 2)
		throw runtime_error("Incorrect hexadecimal input length");

	vector<uint8_t> result(size);

	auto digit = [](char c) -> int
	{
		if (c >= '0' && c <= '9')
			return c - '0';
		if (c >= 'a' && c <= 'f')
			return c - 'a' + 10;
		if (c >= 'A' && c <= 'F')
			return c - 'A' + 10;
		return -1;
	};

	for (size_t i = 0; i < size; ++i)
	{
		int high = digit(hexText[2 * i]);
		int low = digit(hexText[2 * i + 1]);

		if (high < 0 || low < 0)
			throw runtime_error("Invalid hexadecimal character");

		result[i] = static_cast<uint8_t>((high << 4) | low);
	}

	return result;
}

int main()
{
	try
	{
		cout << "BELT - STB 34.101.31\n";
		cout << "1. Enter message from console\n";
		cout << "2. Read message from file\n";
		cout << "Choose: ";

		int choice;
		cin >> choice;
		// cin.ignore(numeric_limits<streamsize>::max(), '\n');

		vector<uint8_t> message;

		if (choice == 1)
		{
			string text;

			cout << "Enter message: ";
			getline(cin, text);

			message = toBytes(text);
		}
		else if (choice == 2)
		{
			string filename;

			cout << "Input filename: ";
			getline(cin, filename);

			message = readFile(filename);
		}
		else
		{
			cout << "Invalid choice\n";
			return 1;
		}

		auto key = readHex("Enter 256-bit key (64 hex chars): ", 32);
		auto iv = readHex("Enter IV (32 hex chars): ", 16);

		BeltCipher cipher(key.data());

		if (!BeltCipher::libraryReady())
		{
			cout << "BELT library is not initialized\n";
			return 1;
		}

		vector<uint8_t> encryptedECB;
		vector<uint8_t> decryptedECB;
		vector<uint8_t> encryptedCBC;
		vector<uint8_t> decryptedCBC;

		if (cipher.encryptECB(
				message.data(), message.size(), encryptedECB) != 0)
		{
			cout << "ECB encryption failed\n";
			return 1;
		}

		if (cipher.decryptECB(
				encryptedECB.data(), encryptedECB.size(), decryptedECB) != 0)
		{
			cout << "ECB decryption failed\n";
			return 1;
		}

		if (cipher.encryptCBC(
				message.data(), message.size(), iv.data(), encryptedCBC) != 0)
		{
			cout << "CBC encryption failed\n";
			return 1;
		}

		if (cipher.decryptCBC(
				encryptedCBC.data(), encryptedCBC.size(),
				iv.data(), decryptedCBC) != 0)
		{
			cout << "CBC decryption failed\n";
			return 1;
		}

		printHex("ECB ciphertext: ", encryptedECB);
		printHex("CBC ciphertext: ", encryptedCBC);

		cout << "ECB decryption correct: "
			 << boolalpha << (message == decryptedECB) << '\n';

		cout << "CBC decryption correct: "
			 << boolalpha << (message == decryptedCBC) << '\n';

		vector<uint8_t> mac(BeltCipher::MAC_SIZE);

		if (cipher.computeMAC(
				message.data(), message.size(), mac.data()) != 0)
		{
			cout << "MAC calculation failed\n";
			return 1;
		}

		printHex("MAC: ", mac);

		if (!writeFile("encrypted_ecb.bin", encryptedECB) ||
			!writeFile("encrypted_cbc.bin", encryptedCBC))
		{
			cout << "Could not write encrypted files\n";
			return 1;
		}

		cout << "Encrypted files saved successfully\n";
	}
	catch (const exception &e)
	{
		cerr << "Error: " << e.what() << '\n';
		return 1;
	}

	return 0;
}