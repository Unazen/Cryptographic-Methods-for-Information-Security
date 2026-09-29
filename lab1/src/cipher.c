#include "../include/cipher.h"

#include <stddef.h>
#include <string.h>

#define LATIN_ALPHABET_SIZE 26
#define CYRILLIC_ALPHABET_SIZE 33

/*
 * ---------------------------------------------------------
 * ВСПОМОГАТЕЛЬНЫЕ ФУНКЦИИ
 * ---------------------------------------------------------
 */

/*
 * Нормализует сдвиг относительно размера алфавита.
 *
 * Например:
 *
 *  29 % 26 = 3
 *  -1 % 26 = -1
 *
 * Поэтому отрицательное значение дополнительно
 * приводится к положительному диапазону.
 */
static int normalize_shift(int shift, int alphabet_size)
{
	shift %= alphabet_size;

	if (shift < 0)
	{
		shift += alphabet_size;
	}

	return shift;
}

/*
 * Проверяет, является ли байт началом UTF-8 символа.
 *
 * Для нашей задачи используются:
 *
 * ASCII:
 * 0xxxxxxx
 *
 * Кириллица:
 * 110xxxxx
 *
 * Остальные варианты нам пока не нужны.
 */
static int utf8_char_length(const unsigned char *text)
{
	if ((text[0] & 0x80) == 0)
	{
		return 1;
	}

	if ((text[0] & 0xE0) == 0xC0)
	{
		return 2;
	}

	if ((text[0] & 0xF0) == 0xE0)
	{
		return 3;
	}

	if ((text[0] & 0xF8) == 0xF0)
	{
		return 4;
	}

	return 0;
}

/*
 * Декодирует UTF-8 символ в Unicode code point.
 *
 * Например:
 *
 * 'A' -> 65
 * 'А' -> 1040
 * 'Б' -> 1041
 *
 * Возвращает 0 при ошибке.
 */
static unsigned int utf8_decode(
	const unsigned char *text,
	int length)
{
	unsigned int code_point;

	if (length == 1)
	{
		return text[0];
	}

	if (length == 2)
	{
		code_point =
			((unsigned int)(text[0] & 0x1F) << 6) |
			(unsigned int)(text[1] & 0x3F);

		return code_point;
	}

	if (length == 3)
	{
		code_point =
			((unsigned int)(text[0] & 0x0F) << 12) |
			((unsigned int)(text[1] & 0x3F) << 6) |
			(unsigned int)(text[2] & 0x3F);

		return code_point;
	}

	if (length == 4)
	{
		code_point =
			((unsigned int)(text[0] & 0x07) << 18) |
			((unsigned int)(text[1] & 0x3F) << 12) |
			((unsigned int)(text[2] & 0x3F) << 6) |
			(unsigned int)(text[3] & 0x3F);

		return code_point;
	}

	return 0;
}

/*
 * Кодирует Unicode code point обратно в UTF-8.
 *
 * Возвращает количество записанных байт.
 */
static int utf8_encode(
	unsigned int code_point,
	char *output)
{
	if (code_point <= 0x7F)
	{
		output[0] = (char)code_point;
		return 1;
	}

	if (code_point <= 0x7FF)
	{
		output[0] = (char)(0xC0 | (code_point >> 6));
		output[1] = (char)(0x80 | (code_point & 0x3F));

		return 2;
	}

	if (code_point <= 0xFFFF)
	{
		output[0] = (char)(0xE0 | (code_point >> 12));
		output[1] = (char)(0x80 | ((code_point >> 6) & 0x3F));
		output[2] = (char)(0x80 | (code_point & 0x3F));

		return 3;
	}

	if (code_point <= 0x10FFFF)
	{
		output[0] = (char)(0xF0 | (code_point >> 18));
		output[1] = (char)(0x80 | ((code_point >> 12) & 0x3F));
		output[2] = (char)(0x80 | ((code_point >> 6) & 0x3F));
		output[3] = (char)(0x80 | (code_point & 0x3F));

		return 4;
	}

	return 0;
}

/*
 * Проверяет латинскую букву.
 */
static int is_latin(unsigned int c)
{
	return (c >= 'A' && c <= 'Z') ||
		   (c >= 'a' && c <= 'z');
}

/*
 * Проверяет русскую букву.
 *
 * А-Я:
 * U+0410 ... U+042F
 *
 * а-я:
 * U+0430 ... U+044F
 *
 * Ё:
 * U+0401
 *
 * ё:
 * U+0451
 */
static int is_cyrillic(unsigned int c)
{
	return (c >= 0x0410 && c <= 0x042F) ||
		   (c >= 0x0430 && c <= 0x044F) ||
		   c == 0x0401 ||
		   c == 0x0451;
}

/*
 * Проверяет, является ли символ буквой.
 */
static int is_letter(unsigned int c)
{
	return is_latin(c) || is_cyrillic(c);
}

/*
 * Возвращает числовую позицию буквы
 * внутри соответствующего алфавита.
 *
 * Например:
 *
 * A -> 0
 * B -> 1
 * Z -> 25
 *
 * А -> 0
 * Б -> 1
 * Я -> 32
 */
static int letter_to_index(unsigned int c)
{
	/* Латиница A-Z */
	if (c >= 'A' && c <= 'Z')
		return (int)(c - 'A');

	/* Латиница a-z */
	if (c >= 'a' && c <= 'z')
		return (int)(c - 'a');

	/* Кириллица А-Е */
	if (c >= 0x0410 && c <= 0x0415)
		return (int)(c - 0x0410);

	/* Ё */
	if (c == 0x0401)
		return 6;

	/* Кириллица Ж-Я */
	if (c >= 0x0416 && c <= 0x042F)
		return (int)(c - 0x0416) + 7;

	/* Кириллица а-е */
	if (c >= 0x0430 && c <= 0x0435)
		return (int)(c - 0x0430);

	/* ё */
	if (c == 0x0451)
		return 6;

	/* Кириллица ж-я */
	if (c >= 0x0436 && c <= 0x044F)
		return (int)(c - 0x0436) + 7;

	return -1;
}

/*
 * Возвращает размер алфавита,
 * которому принадлежит буква.
 */
static int get_alphabet_size(unsigned int c)
{
	if (is_latin(c))
	{
		return LATIN_ALPHABET_SIZE;
	}

	if (is_cyrillic(c))
	{
		return CYRILLIC_ALPHABET_SIZE;
	}

	return 0;
}

/*
 * Преобразует индекс обратно в Unicode-символ.
 *
 * latin:
 *   uppercase = 1 -> A-Z
 *   uppercase = 0 -> a-z
 *
 * cyrillic:
 *   uppercase = 1 -> А-Я
 *   uppercase = 0 -> а-я
 */
static unsigned int index_to_letter(
	int index,
	int alphabet_size,
	int uppercase)
{
	/* Латиница */
	if (alphabet_size == LATIN_ALPHABET_SIZE)
	{
		if (uppercase)
			return (unsigned int)('A' + index);

		return (unsigned int)('a' + index);
	}

	/* Кириллица А-Е */
	if (index <= 5)
	{
		if (uppercase)
			return 0x0410 + index;

		return 0x0430 + index;
	}

	/* Кириллица Ё */
	if (index == 6)
	{
		if (uppercase)
			return 0x0401;

		return 0x0451;
	}

	/* Кириллица Ж-Я */
	if (uppercase)
		return 0x0416 + (index - 7);

	return 0x0436 + (index - 7);
}

/*
 * Сдвигает одну букву.
 */
static int shift_code_point(
	unsigned int c,
	int shift,
	unsigned int *result)
{
	int index;
	int alphabet_size;
	int uppercase;
	int new_index;

	if (!is_letter(c))
	{
		*result = c;
		return 0;
	}

	index = letter_to_index(c);
	alphabet_size = get_alphabet_size(c);

	uppercase =
		(c >= 'A' && c <= 'Z') ||
		(c >= 0x0410 && c <= 0x042F) ||
		c == 0x0401;

	new_index =
		(index + normalize_shift(shift, alphabet_size)) % alphabet_size;

	*result = index_to_letter(
		new_index,
		alphabet_size,
		uppercase);

	return 0;
}

/*
 * Получает сдвиг из символа ключа Виженера.
 *
 * A/a -> 0
 * B/b -> 1
 * ...
 * Z/z -> 25
 *
 * А/а -> 0
 * Б/б -> 1
 * ...
 */
static int key_char_to_shift(
	unsigned int c,
	int *shift)
{
	int index;

	if (!is_letter(c))
	{
		return 1;
	}

	index = letter_to_index(c);

	if (index < 0)
	{
		return 1;
	}

	*shift = index;

	return 0;
}

/*
 * ---------------------------------------------------------
 * ШИФР ЦЕЗАРЯ
 * ---------------------------------------------------------
 */

/*
 * Шифрование Цезаря.
 */
int caesar_encrypt(
	const char *plaintext,
	char *ciphertext,
	int key)
{
	size_t i = 0;
	size_t output_index = 0;

	if (plaintext == NULL || ciphertext == NULL)
	{
		return 1;
	}

	while (plaintext[i] != '\0')
	{
		const unsigned char *current =
			(const unsigned char *)&plaintext[i];

		int length = utf8_char_length(current);

		unsigned int code_point;
		unsigned int shifted;

		if (length == 0)
		{
			return 1;
		}

		code_point = utf8_decode(current, length);

		if (is_letter(code_point))
		{
			shift_code_point(
				code_point,
				key,
				&shifted);

			output_index += utf8_encode(
				shifted,
				&ciphertext[output_index]);
		}
		else
		{
			memcpy(
				&ciphertext[output_index],
				&plaintext[i],
				(size_t)length);

			output_index += (size_t)length;
		}

		i += (size_t)length;
	}

	ciphertext[output_index] = '\0';

	return 0;
}

/*
 * Дешифрование Цезаря.
 */
int caesar_decrypt(
	const char *ciphertext,
	char *plaintext,
	int key)
{
	return caesar_encrypt(
		ciphertext,
		plaintext,
		-key);
}

/*
 * ---------------------------------------------------------
 * ШИФР ВИЖЕНЕРА
 * ---------------------------------------------------------
 */

/*
 * Получает следующий символ ключа UTF-8.
 */
static int get_key_character(
	const char *key,
	size_t *key_index,
	unsigned int *key_character)
{
	const unsigned char *current;
	int length;

	current =
		(const unsigned char *)&key[*key_index];

	length = utf8_char_length(current);

	if (length == 0)
	{
		return 1;
	}

	*key_character =
		utf8_decode(current, length);

	*key_index += (size_t)length;

	return 0;
}

/*
 * Переход к началу ключа.
 */
static void reset_key_index(
	const char *key,
	size_t *key_index)
{
	(void)key;

	*key_index = 0;
}

/*
 * Шифрование Виженера.
 */
int vigenere_encrypt(
	const char *plaintext,
	char *ciphertext,
	const char *key)
{
	size_t i = 0;
	size_t output_index = 0;
	size_t key_index = 0;

	if (plaintext == NULL ||
		ciphertext == NULL ||
		key == NULL)
	{
		return 1;
	}

	if (key[0] == '\0')
	{
		return 1;
	}

	while (plaintext[i] != '\0')
	{
		const unsigned char *current =
			(const unsigned char *)&plaintext[i];

		int length = utf8_char_length(current);

		unsigned int code_point;
		unsigned int shifted;

		if (length == 0)
		{
			return 1;
		}

		code_point = utf8_decode(current, length);

		/*
		 * Если это не буква,
		 * просто копируем её.
		 *
		 * Ключ при этом НЕ продвигаем.
		 */
		if (!is_letter(code_point))
		{
			memcpy(
				&ciphertext[output_index],
				&plaintext[i],
				(size_t)length);

			output_index += (size_t)length;
			i += (size_t)length;

			continue;
		}

		/*
		 * Получаем следующий символ ключа.
		 */
		while (1)
		{
			unsigned int key_character;

			if (key[key_index] == '\0')
			{
				reset_key_index(
					key,
					&key_index);
			}

			if (get_key_character(
					key,
					&key_index,
					&key_character) != 0)
			{
				return 1;
			}

			/*
			 * Символ ключа должен быть буквой.
			 */
			if (is_letter(key_character))
			{
				int shift;

				if (key_char_to_shift(
						key_character,
						&shift) != 0)
				{
					return 1;
				}

				shift_code_point(
					code_point,
					shift,
					&shifted);

				output_index += utf8_encode(
					shifted,
					&ciphertext[output_index]);

				break;
			}
		}

		i += (size_t)length;
	}

	ciphertext[output_index] = '\0';

	return 0;
}

/*
 * Дешифрование Виженера.
 */
int vigenere_decrypt(
	const char *ciphertext,
	char *plaintext,
	const char *key)
{
	size_t i = 0;
	size_t output_index = 0;
	size_t key_index = 0;

	if (ciphertext == NULL ||
		plaintext == NULL ||
		key == NULL)
	{
		return 1;
	}

	if (key[0] == '\0')
	{
		return 1;
	}

	while (ciphertext[i] != '\0')
	{
		const unsigned char *current =
			(const unsigned char *)&ciphertext[i];

		int length = utf8_char_length(current);

		unsigned int code_point;
		unsigned int shifted;

		if (length == 0)
		{
			return 1;
		}

		code_point = utf8_decode(current, length);

		/*
		 * Не-буквы просто копируем.
		 */
		if (!is_letter(code_point))
		{
			memcpy(
				&plaintext[output_index],
				&ciphertext[i],
				(size_t)length);

			output_index += (size_t)length;
			i += (size_t)length;

			continue;
		}

		/*
		 * Получаем следующий символ ключа.
		 */
		while (1)
		{
			unsigned int key_character;

			if (key[key_index] == '\0')
			{
				reset_key_index(
					key,
					&key_index);
			}

			if (get_key_character(
					key,
					&key_index,
					&key_character) != 0)
			{
				return 1;
			}

			if (is_letter(key_character))
			{
				int shift;

				if (key_char_to_shift(
						key_character,
						&shift) != 0)
				{
					return 1;
				}

				shift_code_point(
					code_point,
					-shift,
					&shifted);

				output_index += utf8_encode(
					shifted,
					&plaintext[output_index]);

				break;
			}
		}

		i += (size_t)length;
	}

	plaintext[output_index] = '\0';

	return 0;
}