#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "include/cipher.h"

#define BUFFER_SIZE 4096

/*
 * Тип выбранного шифра.
 */
typedef enum
{
	CIPHER_CAESAR,
	CIPHER_VIGENERE
} CipherType;

/*
 * Читает строку с клавиатуры.
 * Удаляет символ '\n' в конце.
 */
static void read_line(char *buffer, size_t size)
{
	if (fgets(buffer, size, stdin) != NULL)
	{
		buffer[strcspn(buffer, "\n")] = '\0';
	}
}

/*
 * Очищает оставшиеся символы во входном потоке.
 *
 * Нужно, например, после scanf("%d", ...).
 */
static void clear_input(void)
{
	int c;

	while ((c = getchar()) != '\n' && c != EOF)
	{
		/* Ничего не делаем */
	}
}

/*
 * Выводит текущее состояние программы.
 */
static void print_current_settings(
	CipherType cipher,
	int caesar_key,
	const char *vigenere_key)
{
	printf("\n=== Текущие настройки ===\n");

	if (cipher == CIPHER_CAESAR)
	{
		printf("Шифр: Цезарь\n");
		printf("Сдвиг: %d\n", caesar_key);
	}
	else
	{
		printf("Шифр: Виженер\n");
		printf("Ключ: %s\n", vigenere_key);
	}
}

/*
 * Шифрование текста.
 */
static void encrypt_text(
	CipherType cipher,
	int caesar_key,
	const char *vigenere_key)
{
	char plaintext[BUFFER_SIZE];
	char ciphertext[BUFFER_SIZE];

	printf("\nВведите текст для шифрования:\n> ");
	read_line(plaintext, BUFFER_SIZE);

	if (cipher == CIPHER_CAESAR)
	{
		if (caesar_encrypt(
				plaintext,
				ciphertext,
				caesar_key) != 0)
		{
			printf("Ошибка шифрования.\n");
			return;
		}
	}
	else
	{
		if (vigenere_encrypt(
				plaintext,
				ciphertext,
				vigenere_key) != 0)
		{
			printf("Ошибка шифрования.\n");
			return;
		}
	}

	printf("\nИсходный текст:\n%s\n", plaintext);
	printf("\nЗашифрованный текст:\n%s\n", ciphertext);
}

/*
 * Расшифрование текста.
 */
static void decrypt_text(
	CipherType cipher,
	int caesar_key,
	const char *vigenere_key)
{
	char ciphertext[BUFFER_SIZE];
	char plaintext[BUFFER_SIZE];

	printf("\nВведите текст для расшифрования:\n> ");
	read_line(ciphertext, BUFFER_SIZE);

	if (cipher == CIPHER_CAESAR)
	{
		if (caesar_decrypt(
				ciphertext,
				plaintext,
				caesar_key) != 0)
		{
			printf("Ошибка расшифрования.\n");
			return;
		}
	}
	else
	{
		if (vigenere_decrypt(
				ciphertext,
				plaintext,
				vigenere_key) != 0)
		{
			printf("Ошибка расшифрования.\n");
			return;
		}
	}

	printf("\nШифротекст:\n%s\n", ciphertext);
	printf("\nРасшифрованный текст:\n%s\n", plaintext);
}

/*
 * Читает весь текстовый файл в память.
 *
 * Возвращает:
 *   0 - успех
 *   1 - ошибка
 */
static int read_file(
	const char *filename,
	char *buffer,
	size_t buffer_size)
{
	FILE *file;
	size_t bytes_read;

	file = fopen(filename, "rb");

	if (file == NULL)
	{
		return 1;
	}

	bytes_read = fread(
		buffer,
		sizeof(char),
		buffer_size - 1,
		file);

	if (ferror(file))
	{
		fclose(file);
		return 1;
	}

	buffer[bytes_read] = '\0';

	fclose(file);

	return 0;
}

/*
 * Записывает текст в файл.
 *
 * Если файла не существует,
 * он будет создан.
 *
 * Если существует — его содержимое
 * будет перезаписано.
 */
static int write_file(
	const char *filename,
	const char *text)
{
	FILE *file;

	file = fopen(filename, "wb");

	if (file == NULL)
	{
		return 1;
	}

	if (fputs(text, file) == EOF)
	{
		fclose(file);
		return 1;
	}

	fclose(file);

	return 0;
}

/*
 * Шифрование файла.
 */
static void encrypt_file(
	CipherType cipher,
	int caesar_key,
	const char *vigenere_key)
{
	char input_filename[BUFFER_SIZE];
	char output_filename[BUFFER_SIZE];

	char plaintext[BUFFER_SIZE];
	char ciphertext[BUFFER_SIZE];

	printf("\nВведите имя входного .txt файла:\n> ");
	read_line(input_filename, BUFFER_SIZE);

	printf("Введите имя выходного .txt файла:\n> ");
	read_line(output_filename, BUFFER_SIZE);

	/*
	 * Читаем исходный файл.
	 */
	if (read_file(
			input_filename,
			plaintext,
			BUFFER_SIZE) != 0)
	{
		printf("\nОшибка: не удалось прочитать файл.\n");
		return;
	}

	/*
	 * Шифруем содержимое.
	 */
	if (cipher == CIPHER_CAESAR)
	{
		if (caesar_encrypt(
				plaintext,
				ciphertext,
				caesar_key) != 0)
		{
			printf("\nОшибка шифрования файла.\n");
			return;
		}
	}
	else
	{
		if (vigenere_encrypt(
				plaintext,
				ciphertext,
				vigenere_key) != 0)
		{
			printf("\nОшибка шифрования файла.\n");
			return;
		}
	}

	/*
	 * Создаём выходной файл
	 * и записываем туда шифротекст.
	 */
	if (write_file(
			output_filename,
			ciphertext) != 0)
	{
		printf("\nОшибка: не удалось создать выходной файл.\n");
		return;
	}

	printf("\nФайл успешно зашифрован.\n");
	printf("Результат записан в: %s\n", output_filename);
}

/*
 * Расшифрование файла.
 */
static void decrypt_file(
	CipherType cipher,
	int caesar_key,
	const char *vigenere_key)
{
	char input_filename[BUFFER_SIZE];
	char output_filename[BUFFER_SIZE];

	char ciphertext[BUFFER_SIZE];
	char plaintext[BUFFER_SIZE];

	printf("\nВведите имя входного .txt файла:\n> ");
	read_line(input_filename, BUFFER_SIZE);

	printf("Введите имя выходного .txt файла:\n> ");
	read_line(output_filename, BUFFER_SIZE);

	/*
	 * Читаем шифротекст из файла.
	 */
	if (read_file(
			input_filename,
			ciphertext,
			BUFFER_SIZE) != 0)
	{
		printf("\nОшибка: не удалось прочитать файл.\n");
		return;
	}

	/*
	 * Расшифровываем содержимое.
	 */
	if (cipher == CIPHER_CAESAR)
	{
		if (caesar_decrypt(
				ciphertext,
				plaintext,
				caesar_key) != 0)
		{
			printf("\nОшибка расшифрования файла.\n");
			return;
		}
	}
	else
	{
		if (vigenere_decrypt(
				ciphertext,
				plaintext,
				vigenere_key) != 0)
		{
			printf("\nОшибка расшифрования файла.\n");
			return;
		}
	}

	/*
	 * Создаём выходной файл
	 * и записываем туда результат.
	 */
	if (write_file(
			output_filename,
			plaintext) != 0)
	{
		printf("\nОшибка: не удалось создать выходной файл.\n");
		return;
	}

	printf("\nФайл успешно расшифрован.\n");
	printf("Результат записан в: %s\n", output_filename);
}

/*
 * Выбор шифра.
 */
static void choose_cipher(CipherType *cipher)
{
	int choice;

	printf("\n=== Выбор шифра ===\n");
	printf("1. Цезарь\n");
	printf("2. Виженер\n");
	printf("Выберите шифр: ");

	if (scanf("%d", &choice) != 1)
	{
		clear_input();
		printf("Ошибка: необходимо ввести число.\n");
		return;
	}

	clear_input();

	if (choice == 1)
	{
		*cipher = CIPHER_CAESAR;
		printf("Выбран шифр Цезаря.\n");
	}
	else if (choice == 2)
	{
		*cipher = CIPHER_VIGENERE;
		printf("Выбран шифр Виженера.\n");
	}
	else
	{
		printf("Ошибка: неизвестный вариант.\n");
	}
}

/*
 * Установка ключа.
 */
static void set_key(
	CipherType cipher,
	int *caesar_key,
	char *vigenere_key)
{
	if (cipher == CIPHER_CAESAR)
	{
		printf("\nВведите числовой сдвиг: ");

		if (scanf("%d", caesar_key) != 1)
		{
			clear_input();
			printf("Ошибка: ключ должен быть числом.\n");
			return;
		}

		clear_input();

		printf("Сдвиг установлен: %d\n", *caesar_key);
	}
	else
	{
		printf("\nВведите строковый ключ: ");
		read_line(vigenere_key, BUFFER_SIZE);

		if (strlen(vigenere_key) == 0)
		{
			printf("Ошибка: ключ не может быть пустым.\n");
			return;
		}

		printf("Ключ установлен: %s\n", vigenere_key);
	}
}

/*
 * Вывод меню.
 */
static void print_menu(void)
{
	printf("\n");
	printf("====================================\n");
	printf("       ШИФРЫ ЦЕЗАРЯ И ВИЖЕНЕРА     \n");
	printf("====================================\n");

	printf("1. Выбрать шифр\n");
	printf("2. Задать ключ\n");
	printf("3. Получить текущий ключ\n");
	printf("4. Зашифровать текст\n");
	printf("5. Расшифровать текст\n");
	printf("6. Зашифровать файл\n");
	printf("7. Расшифровать файл\n");
	printf("8. Выход\n");

	printf("====================================\n");
	printf("Ваш выбор: ");
}

/*
 * Главная функция программы.
 */
int main(void)
{
	CipherType cipher = CIPHER_CAESAR;

	int caesar_key = 3;

	char vigenere_key[BUFFER_SIZE] = "KEY";

	int choice;

	printf("Добро пожаловать в программу шифрования!\n");

	printf("\nНачальные настройки:\n");
	printf("Шифр: Цезарь\n");
	printf("Сдвиг: 3\n");

	while (1)
	{
		print_menu();

		if (scanf("%d", &choice) != 1)
		{
			clear_input();

			printf("\nОшибка: необходимо ввести номер пункта меню.\n");

			continue;
		}

		clear_input();

		switch (choice)
		{
		/*
		 * 1. Выбрать шифр
		 */
		case 1:
			choose_cipher(&cipher);
			break;

		/*
		 * 2. Задать ключ
		 */
		case 2:
			set_key(
				cipher,
				&caesar_key,
				vigenere_key);
			break;

		/*
		 * 3. Получить текущий ключ
		 */
		case 3:
			print_current_settings(
				cipher,
				caesar_key,
				vigenere_key);
			break;

		/*
		 * 4. Зашифровать текст
		 */
		case 4:
			encrypt_text(
				cipher,
				caesar_key,
				vigenere_key);
			break;

		/*
		 * 5. Расшифровать текст
		 */
		case 5:
			decrypt_text(
				cipher,
				caesar_key,
				vigenere_key);
			break;

		/*
		 * 6. Зашифровать файл
		 */
		case 6:
			encrypt_file(
				cipher,
				caesar_key,
				vigenere_key);
			break;

		/*
		 * 7. Расшифровать файл
		 */
		case 7:
			decrypt_file(
				cipher,
				caesar_key,
				vigenere_key);
			break;

		/*
		 * 8. Выход
		 */
		case 8:
			printf("\nВыход из программы.\n");
			return 0;

		default:
			printf("\nОшибка: такого пункта меню нет.\n");
			break;
		}
	}
}