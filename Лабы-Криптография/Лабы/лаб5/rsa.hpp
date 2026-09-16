#ifndef RSA_HPP
#define RSA_HPP

#include <cstddef>
#include <cstdint>
#include <memory>
#include <vector>
#include "bigmath.hpp"  // ваш BigInt

class RSA
{
public:
    static constexpr uint32_t DEFAULT_E = 65537;

    struct PublicKey
    {
        std::shared_ptr<BigInt> n; // модуль
        std::shared_ptr<BigInt> e; // открытая экспонента
        size_t bitlen = 0;
    };

    struct PrivateKey
    {
        std::shared_ptr<BigInt> n;   // модуль
        std::shared_ptr<BigInt> d;   // закрытая экспонента
        std::shared_ptr<BigInt> p;   // простые множители
        std::shared_ptr<BigInt> q;
        std::shared_ptr<BigInt> dp;  // d mod (p-1)
        std::shared_ptr<BigInt> dq;  // d mod (q-1)
        std::shared_ptr<BigInt> qinv;// q^-1 mod p
        size_t bitlen = 0;
    };

    static bool generateKeys(PublicKey& pub,
                             PrivateKey& priv,
                             size_t bits,
                             uint32_t e_value = DEFAULT_E);

    static std::shared_ptr<BigInt> encrypt(const BigInt& m, const PublicKey& pub);
    static std::shared_ptr<BigInt> decrypt(const BigInt& c, const PrivateKey& priv);

    static std::shared_ptr<BigInt> strToNumber(const uint8_t* str, size_t len);
    static std::vector<uint8_t> numberToStr(const BigInt& m);

    static bool checkKeypair(const PublicKey& pub, const PrivateKey& priv);

private:
    static std::shared_ptr<BigInt> modExp(const BigInt& base,
                                         const BigInt& exp,
                                         const BigInt& mod);

    static std::shared_ptr<BigInt> modInverse(const BigInt& a, const BigInt& m);
    static bool millerRabinTest(const BigInt& candidate, int rounds = 40);
    static std::shared_ptr<BigInt> randomPrime(size_t bits);
};

#endif // RSA_HPP
