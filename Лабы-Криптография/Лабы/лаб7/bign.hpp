#ifndef BIGN_HPP
#define BIGN_HPP

#include <cstddef>
#include <cstdint>
#include <vector>
#include <memory>
#include "bigmath.hpp"  // ваш BigInt

class ECPoint {
public:
    BigInt x;
    BigInt y;
    bool infinite = false; // true если точка на бесконечности

    ECPoint() = default;
    ECPoint(const BigInt& x_, const BigInt& y_, bool inf = false);
};

class CurveParams {
public:
    BigInt p;
    BigInt a;
    BigInt b;
    ECPoint G;
    BigInt q;

    CurveParams() = default;

    void init(); // Инициализация кривой
};

class PrivateKey {
public:
    BigInt x;

    PrivateKey() = default;

    std::vector<uint8_t> to_bytes() const;
    static PrivateKey from_bytes(const uint8_t* in, size_t in_len);
};

class PublicKey {
public:
    ECPoint Y;

    PublicKey() = default;

    std::vector<uint8_t> to_bytes() const;
    static PublicKey from_bytes(const uint8_t* in, size_t in_len);
};

class Signature {
public:
    BigInt r;
    BigInt s;

    Signature() = default;

    std::vector<uint8_t> to_bytes() const;
    static Signature from_bytes(const uint8_t* in, size_t in_len);
};

class Bign {
public:
    // Генерация ключевой пары
    static bool generate_keypair(const CurveParams& curve,
                                 PrivateKey& sk,
                                 PublicKey& pk);

    // Подписание сообщения
    static bool sign(const CurveParams& curve,
                     const PrivateKey& sk,
                     const uint8_t* message,
                     size_t message_len,
                     Signature& sig);

    // Проверка подписи
    static bool verify(const CurveParams& curve,
                       const PublicKey& pk,
                       const uint8_t* message,
                       size_t message_len,
                       const Signature& sig);
};

#endif /* BIGN_HPP */
