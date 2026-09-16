#ifndef DH_HPP
#define DH_HPP

#include <cstddef>
#include <cstdint>
#include <memory>
#include <vector>
#include "bigmath.hpp"  // ваш BigInt

class DiffieHellman
{
public:
    class Params
    {
    public:
        std::shared_ptr<BigInt> p; // большой простой модуль
        std::shared_ptr<BigInt> g; // генератор
        size_t bitlen = 0;

        Params() = default;
        Params(std::shared_ptr<BigInt> prime, std::shared_ptr<BigInt> generator);

        static std::shared_ptr<Params> generate(size_t bits, bool safe_prime = true);
    };

    class PrivateKey
    {
    public:
        std::shared_ptr<BigInt> x;
        size_t bitlen = 0;

        static std::shared_ptr<PrivateKey> generate(const Params& params);
    };

    class PublicKey
    {
    public:
        std::shared_ptr<BigInt> Y;

        PublicKey() = default;
        explicit PublicKey(std::shared_ptr<BigInt> val);
        static std::shared_ptr<PublicKey> compute(const Params& params, const PrivateKey& priv);
    };

    static std::shared_ptr<BigInt> computeSharedSecret(const Params& params,
                                                       const PrivateKey& priv,
                                                       const PublicKey& other_pub);

    static bool deriveKey(const BigInt& shared_secret, uint8_t* out, size_t out_len);
    static std::vector<uint8_t> sharedSecretToBytes(const BigInt& shared_secret);

    static void demoConsole(size_t bits,
                            bool safe_prime,
                            const std::string& alice_msg,
                            const std::string& bob_msg);

private:
    static std::shared_ptr<BigInt> randomInRange(const BigInt& min, const BigInt& max);
};

#endif // DH_HPP
