#ifndef CIPHERS_H
#define CIPHERS_H

#include <stddef.h>
#include <stdint.h>

/**
 * caesar_encrypt
 * Шифрует входной текст с помощью шифра Цезаря.
 * @param plaintext - указатель на исходный текст (строка с нуль-терминатором)
 * @param ciphertext - указатель на буфер для зашифрованного текста
 * @param key - числовой сдвиг (ключ)
 * @return 0 при успешном шифровании, 1 при ошибке
 */
int caesar_encrypt(const char *plaintext, char *ciphertext, int key)
;

/**
 * caesar_decrypt
 * Дешифрует текст, зашифрованный шифром Цезаря.
 * @param ciphertext - указатель на зашифрованный текст
 * @param plaintext - указатель на буфер для расшифрованного текста
 * @param key - числовой сдвиг (ключ)
 * @return 0 при успешном дешифровании, 1 при ошибке
 */
int caesar_decrypt(const char *ciphertext, char *plaintext, int key);

/**
 * vigenere_encrypt
 * Шифрует входной текст с помощью шифра Виженера.
 * @param plaintext - указатель на исходный текст
 * @param ciphertext - указатель на буфер для зашифрованного текста
 * @param key - строковый ключ (последовательность букв)
 * @return 0 при успешном шифровании, 1 при ошибке
 */
int vigenere_encrypt(const char *plaintext, char *ciphertext, const char *key);

/**
 * vigenere_decrypt
 * Дешифрует текст, зашифрованный шифром Виженера.
 * @param ciphertext - указатель на зашифрованный текст
 * @param plaintext - указатель на буфер для расшифрованного текста
 * @param key - строковый ключ
 * @return 0 при успешном дешифровании, 1 при ошибке
 */
int vigenere_decrypt(const char *ciphertext, char *plaintext, const char *key);

#endif /* CIPHERS_H */
