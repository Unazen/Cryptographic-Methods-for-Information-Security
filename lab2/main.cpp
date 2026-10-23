
#include "bigmath.hpp"

#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include <chrono>
#include <random>
#include <iomanip>
#include <stdexcept>
#include <cstdint>

using namespace std;
using Clock = chrono::steady_clock;

namespace
{
	BigInt readDecimal(const string &prompt)
	{
		string input;

		cout << prompt;
		cin >> input;

		return BigInt::fromDec(input);
	}

	/* BigInt readHex(const string &value)
	{
		return BigInt(value);
	} */

	void printNumber(const string &label, const BigInt &value)
	{
		cout << label << value.toHex() << " (hex)" << '\n';
	}

	double measureMilliseconds(const BigInt &base,
							   const BigInt &exp,
							   const BigInt &mod,
							   bool useMontgomery)
	{
		auto start = Clock::now();

		if (useMontgomery)
		{
			MontgomeryCtx ctx(mod);
			volatile uint64_t sink =
				ctx.exp(base, exp).toU64OrDie();
			(void)sink;
		}
		else
		{
			volatile uint64_t sink =
				BigInt::modExp(base, exp, mod).toU64OrDie();
			(void)sink;
		}

		auto finish = Clock::now();

		return chrono::duration<double, milli>(
				   finish - start)
			.count();
	}

	string randomHex(size_t digits, mt19937 &generator)
	{
		static const char symbols[] = "0123456789ABCDEF";

		string result;
		result.reserve(digits);

		for (size_t i = 0; i < digits; ++i)
			result += symbols[generator() % 16];

		if (!result.empty() && result[0] == '0')
			result[0] = '8';

		return result;
	}

	BigInt makeRandomModulus(size_t bits, mt19937 &generator)
	{
		size_t digits = (bits + 3) / 4;
		string hex = randomHex(digits, generator);

		hex[0] = '8';
		hex.back() = 'B';

		return BigInt(hex);
	}

	void modularExponentiation()
	{
		BigInt base = readDecimal("Введите основание: ");
		BigInt exp = readDecimal("Введите показатель степени: ");
		BigInt mod = readDecimal("Введите модуль: ");

		if (mod == BigInt::fromU64(0))
			throw invalid_argument("Модуль не может быть равен нулю.");

		BigInt ordinary = BigInt::modExp(base, exp, mod);

		printNumber("Результат mod_exp: ", ordinary);

		if (mod == BigInt::fromU64(0))
			throw invalid_argument("Модуль не должен быть равен нулю.");

		try
		{
			MontgomeryCtx ctx(mod);
			BigInt montgomery = ctx.exp(base, exp);

			printNumber("Результат Монтгомери: ", montgomery);

			cout << "Проверка: "
				 << (ordinary == montgomery
						 ? "результаты совпадают"
						 : "ОШИБКА: результаты различаются")
				 << '\n';
		}
		catch (const invalid_argument &)
		{
			cout << "Монтгомери требует нечётный модуль больше единицы.\n";
		}
	}

	void modularMultiplication()
	{
		BigInt a = readDecimal("Введите первое число: ");
		BigInt b = readDecimal("Введите второе число: ");
		BigInt mod = readDecimal("Введите модуль: ");

		if (mod == BigInt::fromU64(0))
			throw invalid_argument("Модуль не может быть равен нулю.");

		BigInt ordinary = BigInt::modNaive(
			BigInt::mulNaive(a, b), mod);

		printNumber("Обычное произведение по модулю: ", ordinary);

		try
		{
			MontgomeryCtx ctx(mod);

			BigInt aMont = ctx.toMont(a);
			BigInt bMont = ctx.toMont(b);

			BigInt productMont = ctx.mul(aMont, bMont);
			BigInt result = ctx.fromMont(productMont);

			printNumber("Произведение Монтгомери: ", result);

			cout << "Проверка: "
				 << (ordinary == result
						 ? "результаты совпадают"
						 : "ОШИБКА: результаты различаются")
				 << '\n';
		}
		catch (const invalid_argument &)
		{
			cout << "Монтгомери требует нечётный модуль больше единицы.\n";
		}
	}

	void benchmark()
	{
		const vector<size_t> sizes = {32, 64, 128, 256, 512};

		mt19937 generator(123456);

		ofstream csv("statistics.csv");

		if (!csv)
			throw runtime_error("Не удалось создать statistics.csv");

		csv << "Размер_бит,Обычный_мс,Монтгомери_мс,Результаты_совпадают\n";

		cout << "\nСтатистика производительности\n";
		cout << "Размер бит | Обычный, мс | Монтгомери, мс | Проверка\n";

		for (size_t bits : sizes)
		{
			BigInt mod = makeRandomModulus(bits, generator);
			BigInt base(randomHex((bits + 3) / 4, generator));

			// Показатель фиксированного размера,
			// чтобы сравнивать скорость при разных размерах модуля.
			BigInt exp("FFFFFFFF");

			BigInt ordinary = BigInt::modExp(base, exp, mod);

			MontgomeryCtx ctx(mod);
			BigInt montgomery = ctx.exp(base, exp);

			if (ordinary != montgomery)
				throw runtime_error("Ошибка проверки результатов.");

			double ordinaryTime = measureMilliseconds(
				base, exp, mod, false);

			double montgomeryTime = measureMilliseconds(
				base, exp, mod, true);

			cout << setw(10) << bits
				 << " | " << setw(11) << fixed << setprecision(3)
				 << ordinaryTime
				 << " | " << setw(14) << montgomeryTime
				 << " | совпадают\n";

			csv << bits << ','
				<< ordinaryTime << ','
				<< montgomeryTime << ",да\n";
		}

		cout << "\nСтатистика сохранена в statistics.csv\n";
	}

	void processFile()
	{
		string filename;

		cout << "Введите имя входного файла: ";
		cin >> filename;

		ifstream file(filename);

		if (!file)
			throw runtime_error("Не удалось открыть файл.");

		string line;
		size_t lineNumber = 0;

		while (getline(file, line))
		{
			++lineNumber;

			if (line.empty() || line[0] == '#')
				continue;

			istringstream input(line);
			string operation;
			string first;
			string second;
			string third;

			input >> operation >> first >> second >> third;

			try
			{
				if (operation == "EXP" && !first.empty() && !second.empty() && !third.empty())
				{
					BigInt base = BigInt::fromDec(first);
					BigInt exp = BigInt::fromDec(second);
					BigInt mod = BigInt::fromDec(third);

					BigInt result = BigInt::modExp(base, exp, mod);

					cout << "Строка " << lineNumber
						 << ": результат возведения в степень = "
						 << result.toHex() << " (hex)\n";

					MontgomeryCtx ctx(mod);
					BigInt montResult = ctx.exp(base, exp);

					cout << "Результат Монтгомери = "
						 << montResult.toHex() << " (hex)\n";

					if (result != montResult)
						cout << "ОШИБКА: результаты различаются!\n";
				}
				else if (operation == "MUL" && !first.empty() && !second.empty() && !third.empty())
				{
					BigInt a = BigInt::fromDec(first);
					BigInt b = BigInt::fromDec(second);
					BigInt mod = BigInt::fromDec(third);

					BigInt result = BigInt::modNaive(
						BigInt::mulNaive(a, b), mod);

					cout << "Строка " << lineNumber
						 << ": результат умножения = "
						 << result.toHex() << " (hex)\n";

					MontgomeryCtx ctx(mod);

					BigInt montResult = ctx.fromMont(
						ctx.mul(ctx.toMont(a), ctx.toMont(b)));

					cout << "Результат Монтгомери = "
						 << montResult.toHex() << " (hex)\n";

					if (result != montResult)
						cout << "ОШИБКА: результаты различаются!\n";
				}
				else
				{
					cout << "Строка " << lineNumber
						 << ": неверный формат данных.\n";
				}
			}
			catch (const exception &e)
			{
				cout << "Ошибка в строке " << lineNumber
					 << ": " << e.what() << '\n';
			}
		}
	}

	void showMenu()
	{
		cout << "\n====================================\n";
		cout << " Лабораторная работа №2\n";
		cout << " Быстрые операции в модульной арифметике\n";
		cout << "====================================\n";
		cout << "1. Возведение в степень по модулю\n";
		cout << "2. Умножение по модулю\n";
		cout << "3. Сравнение производительности\n";
		cout << "4. Чтение операций из файла\n";
		cout << "0. Выход\n";
		cout << "Выберите действие: ";
	}
}

int main()
{
	while (true)
	{
		showMenu();

		int choice;

		if (!(cin >> choice))
		{
			cin.clear();
			cin.ignore(numeric_limits<streamsize>::max(), '\n');

			cout << "Ошибка: введите номер пункта меню.\n";
			continue;
		}

		if (choice == 0)
		{
			cout << "Завершение программы.\n";
			break;
		}

		try
		{
			switch (choice)
			{
			case 1:
				modularExponentiation();
				break;

			case 2:
				modularMultiplication();
				break;

			case 3:
				benchmark();
				break;

			case 4:
				processFile();
				break;

			default:
				cout << "Неизвестный пункт меню.\n";
			}
		}
		catch (const exception &e)
		{
			cout << "Ошибка: " << e.what() << '\n';
		}
	}

	return 0;
}
