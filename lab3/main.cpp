#include "belt.hpp"

#include <algorithm>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <iterator>
#include <string>
#include <vector>

#include <limits>
#include <stdexcept>

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
		cout << "====================================\n";
		cout << " Лабораторная работа №3\n";
		cout << " Криптостандарт BELT (СТБ 34.101.31)\n";
		cout << "====================================\n";
		cout << "1. Ввести сообщение с клавиатуры\n";
		cout << "2. Прочитать сообщение из файла\n";
		cout << "Ваш выбор: ";

		int choice;

		if (!(cin >> choice))
		{
			cerr << "Ошибка: необходимо ввести номер пункта меню.\n";
			return 1;
		}

		cin.ignore(numeric_limits<streamsize>::max(), '\n');

		vector<uint8_t> message;

		if (choice == 1)
		{
			string text;

			cout << "Введите текст сообщения: ";
			if (!getline(cin, text))
			{
				cerr << "Ошибка: не удалось прочитать сообщение.\n";
				return 1;
			}

			message = toBytes(text);
		}
		else if (choice == 2)
		{
			string filename;

			cout << "Введите имя входного файла: ";
			if (!getline(cin, filename) || filename.empty())
			{
				cerr << "Ошибка: имя файла не указано.\n";
				return 1;
			}

			message = readFile(filename);
		}
		else
		{
			cerr << "Ошибка: такого пункта меню нет.\n";
			return 1;
		}

		if (message.empty())
		{
			cerr << "Ошибка: сообщение пустое.\n";
			return 1;
		}

		auto key = readHex(
			"Введите ключ длиной 256 бит (64 шестнадцатеричных символа): ",
			32);

		auto iv = readHex(
			"Введите IV длиной 128 бит (32 шестнадцатеричных символа): ",
			16);

		BeltCipher cipher(key.data());

		if (!BeltCipher::libraryReady())
		{
			cerr << "Ошибка: библиотека BELT не инициализирована.\n";
			return 1;
		}

		vector<uint8_t> encryptedECB;
		vector<uint8_t> decryptedECB;
		vector<uint8_t> encryptedCBC;
		vector<uint8_t> decryptedCBC;

		cout << "\n--- Режим ECB ---\n";

		if (cipher.encryptECB(
				message.data(), message.size(), encryptedECB) != 0)
		{
			cerr << "Ошибка: не удалось зашифровать сообщение в режиме ECB.\n";
			cerr << "Проверьте длину сообщения: требуется не менее 16 байт.\n";
			return 1;
		}

		if (cipher.decryptECB(
				encryptedECB.data(), encryptedECB.size(), decryptedECB) != 0)
		{
			cerr << "Ошибка: не удалось расшифровать сообщение в режиме ECB.\n";
			return 1;
		}

		printHex("Зашифрованное сообщение ECB: ", encryptedECB);

		cout << "Результат расшифрования ECB: "
			 << (message == decryptedECB
					 ? "совпадает с исходным сообщением"
					 : "НЕ совпадает с исходным сообщением")
			 << '\n';

		cout << "\n--- Режим CBC ---\n";

		if (cipher.encryptCBC(
				message.data(), message.size(),
				iv.data(), encryptedCBC) != 0)
		{
			cerr << "Ошибка: не удалось зашифровать сообщение в режиме CBC.\n";
			cerr << "Проверьте сообщение и вектор инициализации IV.\n";
			return 1;
		}

		if (cipher.decryptCBC(
				encryptedCBC.data(), encryptedCBC.size(),
				iv.data(), decryptedCBC) != 0)
		{
			cerr << "Ошибка: не удалось расшифровать сообщение в режиме CBC.\n";
			return 1;
		}

		printHex("Зашифрованное сообщение CBC: ", encryptedCBC);

		cout << "Результат расшифрования CBC: "
			 << (message == decryptedCBC
					 ? "совпадает с исходным сообщением"
					 : "НЕ совпадает с исходным сообщением")
			 << '\n';

		cout << "\n--- Вычисление имитовставки MAC ---\n";

		vector<uint8_t> mac(BeltCipher::MAC_SIZE);

		if (cipher.computeMAC(
				message.data(), message.size(), mac.data()) != 0)
		{
			cerr << "Ошибка: не удалось вычислить имитовставку MAC.\n";
			return 1;
		}

		printHex("Имитовставка MAC: ", mac);

		cout << "\n--- Сохранение результатов ---\n";

		if (!writeFile("encrypted_ecb.bin", encryptedECB))
		{
			cerr << "Ошибка: не удалось сохранить файл encrypted_ecb.bin.\n";
			return 1;
		}

		if (!writeFile("encrypted_cbc.bin", encryptedCBC))
		{
			cerr << "Ошибка: не удалось сохранить файл encrypted_cbc.bin.\n";
			return 1;
		}

		cout << "Результат ECB сохранён в файл encrypted_ecb.bin.\n";
		cout << "Результат CBC сохранён в файл encrypted_cbc.bin.\n";
		cout << "\nРабота программы завершена.\n";
	}
	catch (const exception &e)
	{
		cerr << "Ошибка: " << e.what() << '\n';
		return 1;
	}
	catch (...)
	{
		cerr << "Произошла неизвестная ошибка.\n";
		return 1;
	}

	return 0;
}
