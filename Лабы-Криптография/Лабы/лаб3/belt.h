#ifndef BELT_H
#define BELT_H

#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>

/* Константы */
#define BELT_BLOCK_SIZE   16  /* 128 бит = 16 байт */
#define BELT_KEY_SIZE     32  /* 256 бит = 32 байта */
#define BELT_MAC_SIZE     16  /* типичный размер MAC */

/* --- Уровень блока (шифрование/дешифрование одного блока) ---
   in  — указатель на входной блок размером BELT_BLOCK_SIZE
   key — указатель на ключ размером BELT_KEY_SIZE
   out — указатель на выходной буфер размером BELT_BLOCK_SIZE
   Функции не изменяют входы; поведение при NULL — UB (должно документироваться). */
void belt_encrypt_block(const uint8_t in[BELT_BLOCK_SIZE],
                        const uint8_t key[BELT_KEY_SIZE],
                        uint8_t out[BELT_BLOCK_SIZE]);

void belt_decrypt_block(const uint8_t in[BELT_BLOCK_SIZE],
                        const uint8_t key[BELT_KEY_SIZE],
                        uint8_t out[BELT_BLOCK_SIZE]);

/* --- Режимы блочного шифрования: ECB, CBC ---
   Общие соглашения:
   - in_len — длина входных данных в байтах (любая).
   - Для шифрования: выполняется паддинг PKCS#7 (чтоб выход был кратен BELT_BLOCK_SIZE).
   - Для дешифрования: функция проверяет/убирает PKCS#7 паддинг и в out_len возвращает длину исходного сообщения.
   - out — Буфер, выделяемый вызывающим; размер должен быть >= in_len + BELT_BLOCK_SIZE (безопасный минимум).
   - out_len — указатель для записи фактической длины выходных данных.
   - Возвращаемое значение: 0 — успех, <0 — ошибка (например, неверный паддинг, NULL, недостаточный буфер).
*/

/* ECB (Electronic Codebook) */
int belt_encrypt_ecb(const uint8_t *in, size_t in_len,
                     const uint8_t key[BELT_KEY_SIZE],
                     uint8_t *out, size_t *out_len);

int belt_decrypt_ecb(const uint8_t *in, size_t in_len,
                     const uint8_t key[BELT_KEY_SIZE],
                     uint8_t *out, size_t *out_len);

/* CBC (Cipher Block Chaining)
   iv — вектор инициализации размером BELT_BLOCK_SIZE (для шифрования/дешифрования).
   Примечание: при шифровании iv изменять не нужно (если хотите, можно вернуть последний блок — но здесь не требуется). */
int belt_encrypt_cbc(const uint8_t *in, size_t in_len,
                     const uint8_t key[BELT_KEY_SIZE],
                     const uint8_t iv[BELT_BLOCK_SIZE],
                     uint8_t *out, size_t *out_len);

int belt_decrypt_cbc(const uint8_t *in, size_t in_len,
                     const uint8_t key[BELT_KEY_SIZE],
                     const uint8_t iv[BELT_BLOCK_SIZE],
                     uint8_t *out, size_t *out_len);

/* --- Удобные аллокирующие обёртки ---
   Эти функции сами выделяют буфер для выходных данных (malloc).
   Caller обязан free() возвращённого буфера.
   При ошибке возвращают NULL. out_len заполняется при успехе. */
uint8_t *belt_encrypt_ecb_alloc(const uint8_t *in, size_t in_len,
                                const uint8_t key[BELT_KEY_SIZE],
                                size_t *out_len);

uint8_t *belt_encrypt_cbc_alloc(const uint8_t *in, size_t in_len,
                                const uint8_t key[BELT_KEY_SIZE],
                                const uint8_t iv[BELT_BLOCK_SIZE],
                                size_t *out_len);

/* --- MAC (имитовставка) ---
   belt_mac вычисляет MAC по входному сообщению и ключу.
   - out_mac — буфер размером >= BELT_MAC_SIZE; будет заполнен результатом.
   - Реализация должна соответствовать спецификации СТБ 34.101.31 (в спецификации возможны разные варианты размера MAC).
   Возвращает 0 при успехе, <0 при ошибке. */
int belt_mac(const uint8_t *in, size_t in_len,
             const uint8_t key[BELT_KEY_SIZE],
             uint8_t out_mac[BELT_MAC_SIZE]);

/* --- Доп. утилиты ---
   Функция возвращает true, если библиотека встроена инициализирована (если требуется init).
   Можно сделать пустой функцией/always true, но в расширяемой реализации пригодится. */
bool belt_library_ready(void);

/* --- Советы и подсказки:
   - Все вспомогательные функции (S-box, линейные преобразования, round keys и т.д.) — static в .c.
   - Обеспечьте константное время для ключевых операций (по возможности) для уменьшения утечек по времени.
   - Поддержка чтения/записи блоков из файлов — в отдельном модуле приложения (демонстрация).
   - Для потоковой обработки (например, CTR/CFB) можно позже добавить API, совместимый с этим заголовком.
*/

#endif /* BELT_H */
