#ifndef BELT_HPP
#define BELT_HPP

#include <cstddef>
#include <cstdint>
#include <vector>
#include <memory>

class BeltCipher
{
public:
    static constexpr size_t BLOCK_SIZE = 16;  // 128 бит
    static constexpr size_t KEY_SIZE   = 32;  // 256 бит
    static constexpr size_t MAC_SIZE   = 16;  // размер MAC

    BeltCipher() = default;
    explicit BeltCipher(const uint8_t key[KEY_SIZE]);

    void encryptBlock(const uint8_t in[BLOCK_SIZE],
                      uint8_t out[BLOCK_SIZE]) const;

    void decryptBlock(const uint8_t in[BLOCK_SIZE],
                      uint8_t out[BLOCK_SIZE]) const;

    // ECB
    int encryptECB(const uint8_t* in, size_t in_len,
                   std::vector<uint8_t>& out) const;

    int decryptECB(const uint8_t* in, size_t in_len,
                   std::vector<uint8_t>& out) const;

    // CBC
    int encryptCBC(const uint8_t* in, size_t in_len,
                   const uint8_t iv[BLOCK_SIZE],
                   std::vector<uint8_t>& out) const;

    int decryptCBC(const uint8_t* in, size_t in_len,
                   const uint8_t iv[BLOCK_SIZE],
                   std::vector<uint8_t>& out) const;

    int computeMAC(const uint8_t* in, size_t in_len,
                   uint8_t out_mac[MAC_SIZE]) const;

    static bool libraryReady();  // true если библиотека инициализирована

private:
    uint8_t key_[KEY_SIZE]{}; // Секретный ключ
};

#endif // BELT_HPP
