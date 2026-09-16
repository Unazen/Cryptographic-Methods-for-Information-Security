#ifndef BIGMATH_HPP
#define BIGMATH_HPP

#include <cstdint>
#include <cstddef>
#include <string>
#include <memory>

/**
 * Класс для работы с большими целыми числами
 */
class BigInt
{
public:
    BigInt(size_t words = 0);                       // Создать пустое число с capacity = words
    explicit BigInt(const std::string& hexstr);     // Создать из hex строки (throws on error)
    static BigInt fromDec(const std::string& decstr);// Создать из десятичной строки
    static BigInt fromU64(uint64_t v);             // Создать из uint64_t

    BigInt(const BigInt& other);                   // Копирование
    BigInt& operator=(const BigInt& other);

    ~BigInt();

    std::string toHex() const;                      // hex-строка
    uint64_t toU64OrDie() const;                   // преобразование в uint64_t (throws если не помещается)

    int cmp(const BigInt& other) const;            // -1,0,1 как strcmp
    bool operator==(const BigInt& other) const;
    bool operator!=(const BigInt& other) const;

    static BigInt mulNaive(const BigInt& a, const BigInt& b);
    static BigInt modNaive(const BigInt& a, const BigInt& m);
    static BigInt modExp(const BigInt& base, const BigInt& exp, const BigInt& mod);

private:
    uint32_t* digits_;     // Массив слов
    size_t length_;        // Длина массива
    int sign_;             // 0 или 1
};

/**
 * Класс контекста Монтгомери
 */
class MontgomeryCtx
{
public:
    explicit MontgomeryCtx(const BigInt& mod);    // Инициализация контекста
    ~MontgomeryCtx();

    BigInt toMont(const BigInt& a) const;         // a_bar = a * R mod N
    BigInt fromMont(const BigInt& a_bar) const;   // a = a_bar * R^{-1} mod N

    BigInt mul(const BigInt& a, const BigInt& b) const;  // r = a*b*R^{-1} mod N
    BigInt exp(const BigInt& base, const BigInt& exp) const; // r = base^exp mod N

private:
    std::unique_ptr<BigInt> mod_;      // N
    std::unique_ptr<BigInt> R_;        // R
    std::unique_ptr<BigInt> R_inv_;    // R^{-1} mod N
    uint32_t nprime_;                  // -N^{-1} mod base
    size_t n_words_;
};

#endif // BIGMATH_HPP
