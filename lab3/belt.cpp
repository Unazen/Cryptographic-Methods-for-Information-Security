
#include "BELT_HPP"
#include <belt/belt.hpp>

#include <algorithm>
#include <cstring>
#include <stdexcept>

namespace
{
    belt::Bytes makeBytes(const uint8_t* data, size_t size)
    {
        if (size > 0 && data == nullptr)
        {
            throw std::invalid_argument("Null data pointer");
        }

        if (size == 0)
        {
            return {};
        }

        return belt::Bytes(data, data + size);
    }

    belt::Bytes makeKey(const uint8_t* key, size_t size)
    {
        if (key == nullptr || size != BeltCipher::KEY_SIZE)
        {
            throw std::invalid_argument("Key must be 32 bytes");
        }

        return belt::Bytes(key, key + size);
    }

    belt::Bytes makeIV(const uint8_t* iv)
    {
        if (iv == nullptr)
        {
            throw std::invalid_argument("IV must not be null");
        }

        return belt::Bytes(
            iv,
            iv + BeltCipher::BLOCK_SIZE
        );
    }

    int copyResult(
        const belt::Bytes& result,
        std::vector<uint8_t>& out)
    {
        out = result;
        return 0;
    }
}

BeltCipher::BeltCipher(const uint8_t key[KEY_SIZE])
{
    if (key == nullptr)
    {
        throw std::invalid_argument("Key must not be null");
    }

    std::copy(key, key + KEY_SIZE, key_);
}

void BeltCipher::encryptBlock(
    const uint8_t in[BLOCK_SIZE],
    uint8_t out[BLOCK_SIZE]) const
{
    if (in == nullptr || out == nullptr)
    {
        throw std::invalid_argument("Block pointer must not be null");
    }

    belt::Block cipher(makeKey(key_, KEY_SIZE));
    cipher.encrypt(in, out);
}

void BeltCipher::decryptBlock(
    const uint8_t in[BLOCK_SIZE],
    uint8_t out[BLOCK_SIZE]) const
{
    if (in == nullptr || out == nullptr)
    {
        throw std::invalid_argument("Block pointer must not be null");
    }

    belt::Block cipher(makeKey(key_, KEY_SIZE));
    cipher.decrypt(in, out);
}

int BeltCipher::encryptECB(
    const uint8_t* in,
    size_t in_len,
    std::vector<uint8_t>& out) const
{
    try
    {
        const auto key = makeKey(key_, KEY_SIZE);
        const auto data = makeBytes(in, in_len);

        return copyResult(
            belt::ecb_encrypt(key, data),
            out
        );
    }
    catch (...)
    {
        out.clear();
        return -1;
    }
}

int BeltCipher::decryptECB(
    const uint8_t* in,
    size_t in_len,
    std::vector<uint8_t>& out) const
{
    try
    {
        const auto key = makeKey(key_, KEY_SIZE);
        const auto data = makeBytes(in, in_len);

        return copyResult(
            belt::ecb_decrypt(key, data),
            out
        );
    }
    catch (...)
    {
        out.clear();
        return -1;
    }
}

int BeltCipher::encryptCBC(
    const uint8_t* in,
    size_t in_len,
    const uint8_t iv[BLOCK_SIZE],
    std::vector<uint8_t>& out) const
{
    try
    {
        const auto key = makeKey(key_, KEY_SIZE);
        const auto data = makeBytes(in, in_len);
        const auto initializationVector = makeIV(iv);

        return copyResult(
            belt::cbc_encrypt(key, initializationVector, data),
            out
        );
    }
    catch (...)
    {
        out.clear();
        return -1;
    }
}

int BeltCipher::decryptCBC(
    const uint8_t* in,
    size_t in_len,
    const uint8_t iv[BLOCK_SIZE],
    std::vector<uint8_t>& out) const
{
    try
    {
        const auto key = makeKey(key_, KEY_SIZE);
        const auto data = makeBytes(in, in_len);
        const auto initializationVector = makeIV(iv);

        return copyResult(
            belt::cbc_decrypt(key, initializationVector, data),
            out
        );
    }
    catch (...)
    {
        out.clear();
        return -1;
    }
}

int BeltCipher::computeMAC(
    const uint8_t* in,
    size_t in_len,
    uint8_t out_mac[MAC_SIZE]) const
{
    try
    {
        if (out_mac == nullptr)
        {
            return -1;
        }

        const auto key = makeKey(key_, KEY_SIZE);
        const auto data = makeBytes(in, in_len);
        const auto mac = belt::mac(key, data);

        if (mac.size() != MAC_SIZE)
        {
            return -1;
        }

        std::copy(mac.begin(), mac.end(), out_mac);
        return 0;
    }
    catch (...)
    {
        return -1;
    }
}

bool BeltCipher::libraryReady()
{
    return true;
}
