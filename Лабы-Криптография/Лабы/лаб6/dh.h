#ifndef DH_H
#define DH_H

#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>

/* Зависит от реализации длинной арифметики (ЛР2) */
#include "bigmath.h"

/* Общие соглашения:
   - Все возвращаемые BigInt* создаются функциями и должны быть освобождены через bigint_free().
   - Для генерации случайных чисел/простых требуется криптографически стойкий RNG (реализация внешняя).
   - В реализации операции возведения в степень должны использовать мод_exp / montgomery_exp из ЛР2.
*/

/* --- Типы ключей / параметров DH --- */

/* Пара параметров группы (p, g) */
typedef struct {
    BigInt *p;    /* большой простой модуль p */
    BigInt *g;    /* генератор g (2 <= g <= p-2) */
    size_t bitlen;/* длина p в битах (удобство) */
} DH_Params;

/* Приватный ключ (секретный экспонент) */
typedef struct {
    BigInt *x;    /* секретный экспонент (1 <= x <= p-2) */
    size_t bitlen;
} DH_PrivateKey;

/* Публичный ключ (отправляемая сторонами величина A=g^x mod p) */
typedef struct {
    BigInt *Y;    /* публичное значение (A или B) */
} DH_PublicKey;


/* --- Основные функции (интерфейс задания) --- */

/*
 * dh_generate_prime:
 *   Генерирует большое простое p и (опционально) подбирает удобный генератор g.
 *   - bits: желаемая длина p в битах (рекомендуется >= 1024; для стойкости — 2048+).
 *   - safe_prime: если true — попытаться сгенерировать "безопасное" простое p (p и (p-1)/2 просты).
 *   - params_out: при успехе возвращает указатель на выделенную структуру DH_Params (caller free через dh_params_free).
 *   Возвращает 0 при успехе, <0 при ошибке.
 *
 * Примечания: генерация больших простых использует Miller–Rabin (реализовать/взять из ЛР5).
 * Для безопасной генерации требуется крипто-стойкое случайное число.
 */
int dh_generate_prime(DH_Params **params_out, size_t bits, bool safe_prime);

/*
 * dh_compute_public_key:
 *   Вычисляет публичный ключ Y = g^x mod p.
 *   - params: указатель на DH_Params (p, g)
 *   - priv:   указатель на DH_PrivateKey (секрет x)
 *   Возвращает выделенный DH_PublicKey* (caller должен вызвать dh_public_free) или NULL при ошибке.
 */
DH_PublicKey *dh_compute_public_key(const DH_Params *params, const DH_PrivateKey *priv);

/*
 * dh_compute_shared_secret:
 *   Вычисляет общий секрет s = Y_other^x mod p (shared = other_pub^priv mod p).
 *   - params: сведения о группе (p, g)
 *   - priv:   закрытый ключ стороны (x)
 *   - other_pub: публичный ключ другой стороны (Y)
 *   Возвращает BigInt* = s (caller освобождает через bigint_free), либо NULL при ошибке.
 *
 * Замечание: возвращаемое значение — число 0..p-1. Для последующего использования в симметричном шифровании
 * рекомендуется привести его к симметрическому ключу через KDF (см. dh_derive_key ниже).
 */
BigInt *dh_compute_shared_secret(const DH_Params *params,
                                 const DH_PrivateKey *priv,
                                 const DH_PublicKey *other_pub);


/* --- Вспомогательные/полезные функции --- */

/* Создать приватный ключ с заданной длиной (случайно): x случайно в диапазоне [2, p-2].
   Пополнение/генерация приватного ключа должна использовать крипто-RNG.
   Возвращает DH_PrivateKey* или NULL. */
DH_PrivateKey *dh_generate_private_key(const DH_Params *params);

/* Освободить структуры */
void dh_params_free(DH_Params *params);
void dh_private_free(DH_PrivateKey *priv);
void dh_public_free(DH_PublicKey *pub);

/* KDF: производит вывод симметрического ключа (byte array) из общего секрета s (BigInt).
   - s: общий секрет (BigInt)
   - out: буфер для ключа
   - out_len: число байт требуемого симметричного ключа (например 16/24/32)
   Возвращает 0 при успехе, <0 при ошибке.
   Примечание: реализация может использовать SHA-256 или HKDF (рекомендуется HKDF-SHA256). */
int dh_derive_key(const BigInt *s, uint8_t *out, size_t out_len);

/* Утилита: конвертация общего секрета в фиксированное big-endian представление байтов (для KDF).
   Возвращает буфер (malloc) и его длину через out_len. Caller free(). */
uint8_t *dh_shared_secret_to_bytes(const BigInt *s, size_t *out_len);


/* --- Демонстрационные / вспомогательные сценарии --- */

/* Простая демонстрация одного процесса (Alice/Bob) внутри одной программы:
   - Генерация params (или чтение)
   - Генерация приватных ключей для Alice и Bob
   - Вычисление публичных ключей A и B
   - Вычисление секретов sA и sB и проверка их равенства
   - Производство симм. ключа через dh_derive_key и демонстрация обмена сообщениями через BELT
   Функция выполняет демонстрацию и печатает результаты в stdout.
   Возвращает 0 при успехе. */
int dh_demo_console(size_t bits, bool safe_prime, const char *alice_msg, const char *bob_msg);


/* --- Доп мыли ---
   1) Для демонстрации защищённого канала — используйте реализацию BELT (ЛР3) с ключом, полученным через dh_derive_key.
*/


#endif /* DH_H */
