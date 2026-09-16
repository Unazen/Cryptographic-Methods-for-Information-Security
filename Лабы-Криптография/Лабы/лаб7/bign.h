#ifndef BIGN_H
#define BIGN_H

#include <stdint.h>
#include <stddef.h>
#include "bigmath.h"  // ваш BigInt

/* ----------------- Константы ----------------- */
#define BIGN_SUCCESS 0
#define BIGN_ERROR   1

/* Точка на кривой (аффинные координаты) */
typedef struct
{
    BigInt x;
    BigInt y;
    int infinite;     // 1 — точка на бесконечности, 0 — обычная точка
} ECPoint;

/* Параметры кривой */
typedef struct
{
    BigInt p;         // модуль поля
    BigInt a;         // коэффициент a
    BigInt b;         // коэффициент b
    ECPoint G;        // базовая точка
    BigInt q;         // порядок подгруппы
} CurveParams;

/* Ключи */
typedef struct
{
    BigInt x;         // закрытый ключ
} PrivateKey;

typedef struct
{
    ECPoint Y;        // открытый ключ
} PublicKey;

/* Подпись */
typedef struct
{
    BigInt r;
    BigInt s;
} Signature;

/* Инициализация кривой */
void bign_curve_init(CurveParams* curve);

/* Генерация ключевой пары */
int bign_generate_keypair(const CurveParams* curve, PrivateKey* sk, PublicKey* pk);

/* Подписание сообщения */
int bign_sign(const CurveParams* curve,
              const PrivateKey* sk,
              const uint8_t* message, size_t message_len,
              Signature* sig);

/* Проверка подписи */
int bign_verify(const CurveParams* curve,
                const PublicKey* pk,
                const uint8_t* message, size_t message_len,
                const Signature* sig);

/* Сериализация и десериализация ключей/подписи */
size_t bign_privkey_to_bytes(const PrivateKey* sk, uint8_t* out, size_t out_len);
size_t bign_pubkey_to_bytes(const PublicKey* pk, uint8_t* out, size_t out_len);
size_t bign_signature_to_bytes(const Signature* sig, uint8_t* out, size_t out_len);

int bign_privkey_from_bytes(const uint8_t* in, size_t in_len, PrivateKey* sk);
int bign_pubkey_from_bytes(const uint8_t* in, size_t in_len, PublicKey* pk);
int bign_signature_from_bytes(const uint8_t* in, size_t in_len, Signature* sig);

/* Освобождение памяти BigInt */
void bign_bigint_free(BigInt* a);
void bign_ecpoint_free(ECPoint* P);

#endif /* BIGN_H */
