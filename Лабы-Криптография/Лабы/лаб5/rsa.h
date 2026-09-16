#ifndef RSA_H
#define RSA_H

#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>

/* Подключаем заголовок большого целого из ЛР2*/
#include "bigmath.h"

/* Размер открытого экспонентаa по умолчанию (рекомендуется 65537) */
#define RSA_DEFAULT_E 65537u

/* Структуры ключей */
typedef struct {
    BigInt *n;   /* модуль n = p * q */
    BigInt *e;   /* открытая экспонента e */
    /* можно добавить поле bitlen для удобства */
    size_t bitlen;
} RSA_PublicKey;

typedef struct {
    BigInt *n;   /* модуль (копия) */
    BigInt *d;   /* закрытая экспонента d */
    /* дополнительные поля для ускорения декодирования (CRT) */
    BigInt *p;   /* первичный простой множитель */
    BigInt *q;   /* вторичный простой множитель */
    BigInt *dp;  /* d mod (p-1) */
    BigInt *dq;  /* d mod (q-1) */
    BigInt *qinv;/* q^{-1} mod p */
    size_t bitlen;
} RSA_PrivateKey;


/* --- Функции управления ключами --- */

/*
 * rsa_generate_keys:
 *   Генерирует пару ключей RSA.
 *   - pub_out, priv_out — выходные параметры; при успехе функции они будут указывать
 *     на выделенные структуры (caller должен вызвать rsa_free_public/rsa_free_private).
 *   - bits — желаемая длина модуля n в битах (например, 1024, 2048, 4096).
 *   - e_value — открытая экспонента (если 0 — используется RSA_DEFAULT_E).
 *   Возвращает 0 при успехе, <0 при ошибке.
 *
 * Примечания:
 *   - В реализации надо выбирать случайные большие простые p,q (по bits/2 каждый) с помощью
 *     теста простоты Миллера–Рабина и крипто-безопасного генератора случайных чисел.
 *   - Функция должна вычислить также CRT-параметры dp, dq, qinv для ускорения дешифрования.
 */
int rsa_generate_keys(RSA_PublicKey **pub_out, RSA_PrivateKey **priv_out,
                      size_t bits, uint32_t e_value);


/* Освобождение ключей (освобождают все BigInt внутри и саму структуру) */
void rsa_free_public(RSA_PublicKey *pub);
void rsa_free_private(RSA_PrivateKey *priv);


/* --- Шифрование / дешифрование (числовой интерфейс) ---
 *
 * rsa_encrypt:
 *   Шифрование числового сообщения m (0 <= m < n).
 *   - m: BigInt (сообщение)
 *   - pub: указатель на RSA_PublicKey
 *   Возвращает новый BigInt c = m^e mod n (caller обязан вызвать bigint_free).
 *   При ошибке возвращает NULL.
 *
 * rsa_decrypt:
 *   Дешифрование числового шифртекста c.
 *   - c: BigInt
 *   - priv: указатель на RSA_PrivateKey
 *   Возвращает новый BigInt m = c^d mod n (caller обязан вызвать bigint_free).
 *   В реализации рекомендуется использовать CRT-ускорение (dp,dq,qinv).
 */
BigInt *rsa_encrypt(const BigInt *m, const RSA_PublicKey *pub);
BigInt *rsa_decrypt(const BigInt *c, const RSA_PrivateKey *priv);


/* --- Преобразование между строкой (байтовым массивом) и числом ---
 *
 * rsa_str_to_number:
 *   Преобразует массив байт (big-endian или little-endian — выбрать соглашение; здесь
 *   принято: байтовая строка интерпретируется как big-endian; т.е. str[0] -- старший байт)
 *   в BigInt; возвращаемое BigInt нужно освободить через bigint_free.
 *   - str: указатель на байты
 *   - len: длина в байтах
 *   Возвращает BigInt* или NULL при ошибке.
 *
 * Примечание: перед шифрованием необходимо гарантировать m < n. Если строка даёт m >= n,
 * нужно либо отказать, либо использовать разбивку/паддинг (PKCS#1 v1.5 / OAEP) — это задание
 * повышенной сложности и выходит за рамки базового лабораторного задания.
 *
 * rsa_number_to_str:
 *   Обратная функция: преобразует BigInt m в байтовый массив (big-endian).
 *   - m: входной BigInt
 *   - out_len: указатель на size_t для записи длины результирующего буфера
 *   Возвращает выделенный буфер (uint8_t *) длиной *out_len; освобождение — через free().
 *   Возвращает NULL при ошибке.
 */
BigInt *rsa_str_to_number(const uint8_t *str, size_t len);
uint8_t *rsa_number_to_str(const BigInt *m, size_t *out_len);


/* --- Утилиты / проверки --- */

/* Проверка корректности пары ключей:
 *   Возвращает true, если (m^e mod n)^d mod n == m для тестового m (внутри функции можно
 *   взять случайное или фиксированное значение); иначе false.
 *   Эта функция — вспомогательная для тестов. */
bool rsa_check_keypair(const RSA_PublicKey *pub, const RSA_PrivateKey *priv);


/* --- Вспомогательные (скрываемые в реализации) ---
 * В .c реализуйте: miller_rabin, ext_gcd (расширенный евклид), mod_inverse,
 * random_prime_generate, padding (если будете поддерживать PKCS/OAEP), и т.д.
 * Эти функции не экспортировать в заголовке (объявлять static).
 */


/* --- Замечания по безопасности и использованию ---
 * - Все функции, выделяющие память, должны документировать, кто освобождает память.
 * - Шифрование: m должно удовлетворять 0 <= m < n; если входная строка даёт m >= n,
 *   используйте схемы паддинга и/или разбивки сообщения на блоки, либо выдавайте ошибку.
 */

#endif /* RSA_H */
