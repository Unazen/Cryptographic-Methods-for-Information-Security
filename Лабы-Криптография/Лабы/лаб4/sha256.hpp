#ifndef SHA256_HPP
#define SHA256_HPP

#include <cstddef>
#include <cstdint>
#include <vector>
#include <string>

class SHA256
{
public:
    static constexpr size_t BLOCK_SIZE = 64;  // 512 бит
    static constexpr size_t HASH_SIZE  = 32;  // 256 бит

    SHA256();
    ~SHA256();

    void init();  // инициализация
    void update(const uint8_t* data, size_t len);  // добавление данных
    void final(uint8_t out[HASH_SIZE]);           // завершение, получение хеша

    // Однократное вычисление хеша (комбинированный метод)
    static void compute(const uint8_t* data, size_t len, uint8_t out[HASH_SIZE]);

    static int hmac(const uint8_t* key, size_t key_len,
                    const uint8_t* data, size_t data_len,
                    uint8_t out[HASH_SIZE]);

    static int hashToHex(const uint8_t hash[HASH_SIZE],
                         char* out_hex, size_t out_hex_len);

    static bool libraryReady();  // всегда true, если библиотека готова

private:
    uint32_t state_[8]{};
    uint64_t bitlen_{0};
    uint8_t buffer_[BLOCK_SIZE]{};
    size_t buflen_{0};

    void compress(const uint8_t block[BLOCK_SIZE]);
    void padAndFinalize(uint8_t out[HASH_SIZE]);
};

#endif // SHA256_HPP
