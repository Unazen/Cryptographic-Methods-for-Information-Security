#include "belt.hpp"

#include <algorithm>
#include <array>
#include <cstring>
#include <stdexcept>
#include <vector>

namespace
{
    using Byte = uint8_t;
    using Block = std::array<Byte, 16>;
    using Key = std::array<Byte, 32>;

    constexpr size_t BS = 16;

    constexpr Byte H[256] = {
        0xB1,0x94,0xBA,0xC8,0x0A,0x08,0xF5,0x3B,0x36,0x6D,0x00,0x8E,0x58,0x4A,0x5D,0xE4,
        0x85,0x04,0xFA,0x9D,0x1B,0xB6,0xC7,0xAC,0x25,0x2E,0x72,0xC2,0x02,0xFD,0xCE,0x0D,
        0x5B,0xE3,0xD6,0x12,0x17,0xB9,0x61,0x81,0xFE,0x67,0x86,0xAD,0x71,0x6B,0x89,0x0B,
        0x5C,0xB0,0xC0,0xFF,0x33,0xC3,0x56,0xB8,0x35,0xC4,0x05,0xAE,0xD8,0xE0,0x7F,0x99,
        0xE1,0x2B,0xDC,0x1A,0xE2,0x82,0x57,0xEC,0x70,0x3F,0xCC,0xF0,0x95,0xEE,0x8D,0xF1,
        0xC1,0xAB,0x76,0x38,0x9F,0xE6,0x78,0xCA,0xF7,0xC6,0xF8,0x60,0xD5,0xBB,0x9C,0x4F,
        0xF3,0x3C,0x65,0x7B,0x63,0x7C,0x30,0x6A,0xDD,0x4E,0xA7,0x79,0x9E,0xB2,0x3D,0x31,
        0x3E,0x98,0xB5,0x6E,0x27,0xD3,0xBC,0xCF,0x59,0x1E,0x18,0x1F,0x4C,0x5A,0xB7,0x93,
        0xE9,0xDE,0xE7,0x2C,0x8F,0x0C,0x0F,0xA6,0x2D,0xDB,0x49,0xF4,0x6F,0x73,0x96,0x47,
        0x06,0x07,0x53,0x16,0xED,0x24,0x7A,0x37,0x39,0xCB,0xA3,0x83,0x03,0xA9,0x8B,0xF6,
        0x92,0xBD,0x9B,0x1C,0xE5,0xD1,0x41,0x01,0x54,0x45,0xFB,0xC9,0x5E,0x4D,0x0E,0xF2,
        0x68,0x20,0x80,0xAA,0x22,0x7D,0x64,0x2F,0x26,0x87,0xF9,0x34,0x90,0x40,0x55,0x11,
        0xBE,0x32,0x97,0x13,0x43,0xFC,0x9A,0x48,0xA0,0x2A,0x88,0x5F,0x19,0x4B,0x09,0xA1,
        0x7E,0xCD,0xA4,0xD0,0x15,0x44,0xAF,0x8C,0xA5,0x84,0x50,0xBF,0x66,0xD2,0xE8,0x8A,
        0xA2,0xD7,0x46,0x52,0x42,0xA8,0xDF,0xB3,0x69,0x74,0xC5,0x51,0xEB,0x23,0x29,0x21,
        0xD4,0xEF,0xD9,0xB4,0x3A,0x62,0x28,0x75,0x91,0x14,0x10,0xEA,0x77,0x6C,0xDA,0x1D
    };

    constexpr size_t KI[8][7] = {
        {0,1,2,3,4,5,6},
        {7,0,1,2,3,4,5},
        {6,7,0,1,2,3,4},
        {5,6,7,0,1,2,3},
        {4,5,6,7,0,1,2},
        {3,4,5,6,7,0,1},
        {2,3,4,5,6,7,0},
        {1,2,3,4,5,6,7}
    };

    uint32_t load32(const Byte* p)
    {
        return uint32_t(p[0])
            | (uint32_t(p[1]) << 8)
            | (uint32_t(p[2]) << 16)
            | (uint32_t(p[3]) << 24);
    }

    void store32(Byte* p, uint32_t x)
    {
        p[0] = Byte(x);
        p[1] = Byte(x >> 8);
        p[2] = Byte(x >> 16);
        p[3] = Byte(x >> 24);
    }

    uint32_t rotl(uint32_t x, unsigned n)
    {
        return (x << n) | (x >> (32 - n));
    }

    uint32_t G(uint32_t x, unsigned r)
    {
        uint32_t y =
            (uint32_t(H[(x >> 24) & 255]) << 24)
            | (uint32_t(H[(x >> 16) & 255]) << 16)
            | (uint32_t(H[(x >> 8) & 255]) << 8)
            | uint32_t(H[x & 255]);

        return rotl(y, r);
    }

    void encryptRaw(const Byte* input, Byte* output, const Key& key)
    {
        uint32_t k[8];
        for (size_t i = 0; i < 8; ++i)
            k[i] = load32(key.data() + i * 4);

        uint32_t a = load32(input);
        uint32_t b = load32(input + 4);
        uint32_t c = load32(input + 8);
        uint32_t d = load32(input + 12);

        for (size_t i = 0; i < 8; ++i)
        {
            b ^= G(a + k[KI[i][0]], 5);
            c ^= G(d + k[KI[i][1]], 21);
            a -= G(b + k[KI[i][2]], 13);

            uint32_t e =
                G(b + c + k[KI[i][3]], 21) ^ uint32_t(i + 1);

            b += e;
            c -= e;
            d += G(c + k[KI[i][4]], 13);
            b ^= G(a + k[KI[i][5]], 21);
            c ^= G(d + k[KI[i][6]], 5);

            std::swap(a, b);
            std::swap(c, d);
            std::swap(b, c);
        }

        store32(output, b);
        store32(output + 4, d);
        store32(output + 8, a);
        store32(output + 12, c);
    }

    void decryptRaw(const Byte* input, Byte* output, const Key& key)
    {
        uint32_t k[8];
        for (size_t i = 0; i < 8; ++i)
            k[i] = load32(key.data() + i * 4);

        uint32_t a = load32(input);
        uint32_t b = load32(input + 4);
        uint32_t c = load32(input + 8);
        uint32_t d = load32(input + 12);

        for (size_t i = 0; i < 8; ++i)
        {
            const size_t j = 7 - i;

            b ^= G(a + k[KI[j][6]], 5);
            c ^= G(d + k[KI[j][5]], 21);
            a -= G(b + k[KI[j][4]], 13);

            uint32_t e =
                G(b + c + k[KI[j][3]], 21) ^ uint32_t(j + 1);

            b += e;
            c -= e;
            d += G(c + k[KI[j][2]], 13);
            b ^= G(a + k[KI[j][1]], 21);
            c ^= G(d + k[KI[j][0]], 5);

            std::swap(a, b);
            std::swap(c, d);
            std::swap(a, d);
        }

        store32(output, c);
        store32(output + 4, a);
        store32(output + 8, d);
        store32(output + 12, b);
    }

    Key getKey(const uint8_t* key)
    {
        if (!key)
            throw std::invalid_argument("Null key");

        Key result{};
        std::copy(key, key + 32, result.begin());
        return result;
    }

    void xorBlock(Byte* out, const Byte* a, const Byte* b)
    {
        for (size_t i = 0; i < BS; ++i)
            out[i] = a[i] ^ b[i];
    }

    void phi1(const Block& in, Block& out)
    {
        for (size_t j = 0; j < 4; ++j)
        {
            out[j] = in[4 + j];
            out[4 + j] = in[8 + j];
            out[8 + j] = in[12 + j];
            out[12 + j] = in[j] ^ in[4 + j];
        }
    }

    void phi2(const Block& in, Block& out)
    {
        for (size_t j = 0; j < 4; ++j)
        {
            out[j] = in[j] ^ in[12 + j];
            out[4 + j] = in[j];
            out[8 + j] = in[4 + j];
            out[12 + j] = in[8 + j];
        }
    }

    std::vector<Byte> processECB(
        const Key& key, const Byte* in, size_t len, bool encrypt)
    {
        if (len > 0 && !in)
            throw std::invalid_argument("Null input");

        if (len < BS)
            throw std::invalid_argument("BELT ECB requires at least 16 bytes");

        std::vector<Byte> out(len);
        const size_t full = len / BS;
        const size_t rem = len % BS;

        auto run = [&](const Byte* src, Byte* dst)
        {
            if (encrypt)
                encryptRaw(src, dst, key);
            else
                decryptRaw(src, dst, key);
        };

        if (rem == 0)
        {
            for (size_t off = 0; off < len; off += BS)
                run(in + off, out.data() + off);

            return out;
        }

        size_t off = 0;
        for (size_t i = 0; i + 1 < full; ++i, off += BS)
            run(in + off, out.data() + off);

        Block t{};
        run(in + off, t.data());

        Block last{};
        for (size_t j = 0; j < rem; ++j)
            last[j] = in[off + BS + j];

        for (size_t j = rem; j < BS; ++j)
            last[j] = t[j];

        run(last.data(), out.data() + off);

        for (size_t j = 0; j < rem; ++j)
            out[off + BS + j] = t[j];

        return out;
    }

    std::vector<Byte> processCBC(
        const Key& key,
        const Byte* in,
        size_t len,
        const Byte* iv,
        bool encrypt)
    {
        if (!iv)
            throw std::invalid_argument("Null IV");

        if (len > 0 && !in)
            throw std::invalid_argument("Null input");

        if (len < BS)
            throw std::invalid_argument("BELT CBC requires at least 16 bytes");

        std::vector<Byte> out(len);
        const size_t full = len / BS;
        const size_t rem = len % BS;

        Block prev{};
        std::copy(iv, iv + BS, prev.begin());

        const size_t direct = rem == 0 ? full : full - 1;
        size_t off = 0;

        for (size_t i = 0; i < direct; ++i, off += BS)
        {
            Block tmp{};

            if (encrypt)
            {
                xorBlock(tmp.data(), in + off, prev.data());
                encryptRaw(tmp.data(), out.data() + off, key);
                std::copy(out.data() + off,
                          out.data() + off + BS, prev.begin());
            }
            else
            {
                decryptRaw(in + off, tmp.data(), key);
                xorBlock(out.data() + off, tmp.data(), prev.data());
                std::copy(in + off, in + off + BS, prev.begin());
            }
        }

        if (rem == 0)
            return out;

        Block t{};
        Block last{};

        if (encrypt)
        {
            xorBlock(last.data(), in + off, prev.data());
            encryptRaw(last.data(), t.data(), key);

            for (size_t j = 0; j < rem; ++j)
                last[j] = in[off + BS + j] ^ t[j];

            for (size_t j = rem; j < BS; ++j)
                last[j] = t[j];

            encryptRaw(last.data(), out.data() + off, key);

            for (size_t j = 0; j < rem; ++j)
                out[off + BS + j] = t[j];
        }
        else
        {
            decryptRaw(in + off, t.data(), key);

            for (size_t j = 0; j < rem; ++j)
                last[j] = in[off + BS + j];

            for (size_t j = rem; j < BS; ++j)
                last[j] = t[j];

            Block dec{};
            decryptRaw(last.data(), dec.data(), key);
            xorBlock(out.data() + off, dec.data(), prev.data());

            for (size_t j = 0; j < rem; ++j)
                out[off + BS + j] = t[j] ^ in[off + BS + j];
        }

        return out;
    }

    std::array<Byte, 8> calculateMAC(
        const Key& key, const Byte* in, size_t len)
    {
        if (len > 0 && !in)
            throw std::invalid_argument("Null input");

        Block zero{};
        Block r{};
        encryptRaw(zero.data(), r.data(), key);

        Block s{};
        Block phi{};

        const size_t lastLen =
            len == 0 ? 0 : (len % BS == 0 ? BS : len % BS);

        const size_t lastOff = len - lastLen;

        for (size_t off = 0; off < lastOff; off += BS)
        {
            for (size_t j = 0; j < BS; ++j)
                s[j] ^= in[off + j];

            encryptRaw(s.data(), s.data(), key);
        }

        if (lastLen == BS)
        {
            phi1(r, phi);

            for (size_t j = 0; j < BS; ++j)
                s[j] ^= in[lastOff + j] ^ phi[j];
        }
        else
        {
            Block psi{};

            for (size_t j = 0; j < lastLen; ++j)
                psi[j] = in[lastOff + j];

            psi[lastLen] = 0x80;
            phi2(r, phi);

            for (size_t j = 0; j < BS; ++j)
                s[j] ^= psi[j] ^ phi[j];
        }

        Block fin{};
        encryptRaw(s.data(), fin.data(), key);

        std::array<Byte, 8> result{};
        std::copy(fin.begin(), fin.begin() + result.size(), result.begin());
        return result;
    }
}

BeltCipher::BeltCipher(const uint8_t key[KEY_SIZE])
{
    if (!key)
        throw std::invalid_argument("Null key");

    std::copy(key, key + KEY_SIZE, key_);
}

void BeltCipher::encryptBlock(
    const uint8_t in[BLOCK_SIZE], uint8_t out[BLOCK_SIZE]) const
{
    if (!in || !out)
        throw std::invalid_argument("Null block");

    const Key key = getKey(key_);
    encryptRaw(in, out, key);
}

void BeltCipher::decryptBlock(
    const uint8_t in[BLOCK_SIZE], uint8_t out[BLOCK_SIZE]) const
{
    if (!in || !out)
        throw std::invalid_argument("Null block");

    const Key key = getKey(key_);
    decryptRaw(in, out, key);
}

int BeltCipher::encryptECB(
    const uint8_t* in, size_t in_len,
    std::vector<uint8_t>& out) const
{
    try
    {
        out = processECB(getKey(key_), in, in_len, true);
        return 0;
    }
    catch (...)
    {
        out.clear();
        return -1;
    }
}

int BeltCipher::decryptECB(
    const uint8_t* in, size_t in_len,
    std::vector<uint8_t>& out) const
{
    try
    {
        out = processECB(getKey(key_), in, in_len, false);
        return 0;
    }
    catch (...)
    {
        out.clear();
        return -1;
    }
}

int BeltCipher::encryptCBC(
    const uint8_t* in, size_t in_len,
    const uint8_t iv[BLOCK_SIZE],
    std::vector<uint8_t>& out) const
{
    try
    {
        out = processCBC(getKey(key_), in, in_len, iv, true);
        return 0;
    }
    catch (...)
    {
        out.clear();
        return -1;
    }
}

int BeltCipher::decryptCBC(
    const uint8_t* in, size_t in_len,
    const uint8_t iv[BLOCK_SIZE],
    std::vector<uint8_t>& out) const
{
    try
    {
        out = processCBC(getKey(key_), in, in_len, iv, false);
        return 0;
    }
    catch (...)
    {
        out.clear();
        return -1;
    }
}

int BeltCipher::computeMAC(
    const uint8_t* in, size_t in_len,
    uint8_t out_mac[MAC_SIZE]) const
{
    try
    {
        if (!out_mac)
            return -1;

        const auto mac = calculateMAC(getKey(key_), in, in_len);

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
