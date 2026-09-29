#include "../include/cipher.h"

#include <stddef.h>
#include <string.h>
#include <ctype.h>

#define ALPHABET_SIZE 26

/*
 * Нормализация сдвига.
 *
 * Например:
 *   3   -> 3
 *   26  -> 0
 *   29  -> 3
 *   -1  -> 25
 */
static int normalize_shift(int shift)
{
	shift %= ALPHABET_SIZE;

	if (shift < 0)
	{
		shift += ALPHABET_SIZE;
	}

	return shift;
}

/*
 * Сдвигает один символ латинского алфавита.
 *
 * shift > 0  -> шифрование
 * shift < 0  -> дешифрование
 *
 * Неалфавитные символы остаются без изменений.
 */
static char shift_char(char c, int shift)
{
	shift = normalize_shift(shift);

	if (c >= 'A' && c <= 'Z')
	{
		return (char)('A' + (c - 'A' + shift) % ALPHABET_SIZE);
	}

	if (c >= 'a' && c <= 'z')
	{
		return (char)('a' + (c - 'a' + shift) % ALPHABET_SIZE);
	}

	return c;
}

/*
 * Шифр Цезаря.
 */
int caesar_encrypt(const char *plaintext, char *ciphertext, int key)
{
	size_t i;

	if (plaintext == NULL || ciphertext == NULL)
	{
		return 1;
	}

	for (i = 0; plaintext[i] != '\0'; i++)
	{
		ciphertext[i] = shift_char(plaintext[i], key);
	}

	ciphertext[i] = '\0';

	return 0;
}

/*
 * Дешифрирование Цезаря.
 */
int caesar_decrypt(const char *ciphertext, char *plaintext, int key)
{
	size_t i;

	if (ciphertext == NULL || plaintext == NULL)
	{
		return 1;
	}

	for (i = 0; ciphertext[i] != '\0'; i++)
	{
		plaintext[i] = shift_char(ciphertext[i], -key);
	}

	plaintext[i] = '\0';

	return 0;
}

/*
 * Преобразует букву ключа Виженера
 * в числовой сдвиг.
 *
 * A/a -> 0
 * B/b -> 1
 * C/c -> 2
 * ...
 * Z/z -> 25
 */
static int key_char_to_shift(char c)
{
	if (c >= 'A' && c <= 'Z')
	{
		return c - 'A';
	}

	if (c >= 'a' && c <= 'z')
	{
		return c - 'a';
	}

	return -1;
}

/*
 * Шифр Виженера.
 */
int vigenere_encrypt(const char *plaintext, char *ciphertext, const char *key)
{
	size_t i;
	size_t key_index = 0;
	size_t key_length;

	if (plaintext == NULL || ciphertext == NULL || key == NULL)
	{
		return 1;
	}

	key_length = strlen(key);

	if (key_length == 0)
	{
		return 1;
	}

	for (i = 0; plaintext[i] != '\0'; i++)
	{
		char key_char = key[key_index];
		int shift = key_char_to_shift(key_char);

		if (shift < 0)
		{
			return 1;
		}

		/*
		 * Сдвигаем только буквы.
		 * Пробелы, цифры и знаки препинания
		 * остаются без изменений.
		 */
		if ((plaintext[i] >= 'A' && plaintext[i] <= 'Z') ||
			(plaintext[i] >= 'a' && plaintext[i] <= 'z'))
		{
			ciphertext[i] = shift_char(plaintext[i], shift);

			key_index++;

			if (key_index == key_length)
			{
				key_index = 0;
			}
		}
		else
		{
			ciphertext[i] = plaintext[i];
		}
	}

	ciphertext[i] = '\0';

	return 0;
}

/*
 * Расшифровка Виженера.
 */
int vigenere_decrypt(
	const char *ciphertext,
	char *plaintext,
	const char *key)
{
	size_t i;
	size_t key_index = 0;
	size_t key_length;

	if (ciphertext == NULL || plaintext == NULL || key == NULL)
	{
		return 1;
	}

	key_length = strlen(key);

	if (key_length == 0)
	{
		return 1;
	}

	for (i = 0; ciphertext[i] != '\0'; i++)
	{
		char key_char = key[key_index];
		int shift = key_char_to_shift(key_char);

		if (shift < 0)
		{
			return 1;
		}

		if ((ciphertext[i] >= 'A' && ciphertext[i] <= 'Z') ||
			(ciphertext[i] >= 'a' && ciphertext[i] <= 'z'))
		{
			plaintext[i] = shift_char(ciphertext[i], -shift);

			key_index++;

			if (key_index == key_length)
			{
				key_index = 0;
			}
		}
		else
		{
			plaintext[i] = ciphertext[i];
		}
	}

	plaintext[i] = '\0';

	return 0;
}