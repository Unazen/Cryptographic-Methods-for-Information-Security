#ifndef SHA256_H
#define SHA256_H

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

/* Константы */
#define SHA256_BLOCK_SIZE  64   /* блок компрессии в байтах (512 бит) */
#define SHA256_HASH_SIZE   32   /* размер выходного хеша в байтах (256 бит) */

/* Контекст (внутреннее состояние) SHA-256.
   - state  : восемь 32-битных регистра (h0..h7)
   - bitlen : длина обработанных данных в битах (64-битное значение)
   - buffer : накопительный буфер для частичных блоков (до SHA256_BLOCK_SIZE байт)
   - buflen : текущее количество байт в buffer
   В реализации все приватные вспомогательные функции делают static внутри .c,
   а структура используется пользователем как opaque-хандл (передаётся по указателю).
*/
typedef struct {
    uint32_t state[8];
    uint64_t bitlen;
    uint8_t  buffer[SHA256_BLOCK_SIZE];
    size_t   buflen;
} SHA256_CTX;


/* Инициализация: устанавливает начальные константы состояния.
   ctx — указатель на контекст (должен быть действительным). */
void sha256_init(SHA256_CTX *ctx);

/* Обновление состояния блоком данных.
   Может вызываться многократно для потоковой обработки.
   data — указатель на байты, len — длина в байтах. */
void sha256_update(SHA256_CTX *ctx, const uint8_t *data, size_t len);

/* Завершение: дополняет сообщение, выполняет финальную компрессию и
   записывает 32-байтный хеш в out (out должно быть буфером длины >= SHA256_HASH_SIZE).
   После вызова ctx можно переиспользовать (снова вызвать sha256_init) или очистить. */
void sha256_final(SHA256_CTX *ctx, uint8_t out[SHA256_HASH_SIZE]);

/* Удобная обёртка: однократное вычисление хеша (init + update + final).
   data может быть NULL при len == 0. out — буфер длины >= SHA256_HASH_SIZE. */
void sha256_compute(const uint8_t *data, size_t len, uint8_t out[SHA256_HASH_SIZE]);


/* --- HMAC-SHA256 ---
   Вычисляет HMAC по стандартной схеме:
     HMAC(K, M) = H((K' xor opad) || H((K' xor ipad) || M))
   где K' — ключ, приведённый до блока SHA256_BLOCK_SIZE (padded or hashed).
   - key, key_len — ключ и его длина в байтах
   - data, data_len — сообщение
   - out — буфер длины >= SHA256_HASH_SIZE, заполнится результатом
   Возвращаемое значение:
     0  — успех
    -1  — неверные аргументы (NULL)
    -2  — иная ошибка (в реализации можно расширить коды ошибок).
*/
int sha256_hmac(const uint8_t *key, size_t key_len,
                const uint8_t *data, size_t data_len,
                uint8_t out[SHA256_HASH_SIZE]);


/* --- Вспомогательные утилиты (опционально экспортировать) ---
   hex string helper: преобразует бинарный хеш в NUL-terminated hex string.
   out_hex_len должно быть >= (SHA256_HASH_SIZE * 2 + 1).
   Возвращает 0 при успехе, <0 при ошибке. (Реализовать можно в utils.c) */
int sha256_hash_to_hex(const uint8_t hash[SHA256_HASH_SIZE],
                       char *out_hex, size_t out_hex_len);

/* Рекомендуемая безопасная очистка контекста (например, перед free): */
void sha256_ctx_clear(SHA256_CTX *ctx);

/* Функция указывает готовность/поддержку библиотеки (может быть всегда true).
   Полезна, если реализация требует динамической инициализации. */
bool sha256_library_ready(void);

#endif /* SHA256_H */
