#ifndef CIPHERS_HPP
#define CIPHERS_HPP

#include <string>

/**
 * Класс для работы с шифром Цезаря
 */
class CaesarCipher
{
public:
    /**
     * Конструктор с указанием ключа
     * @param key - числовой сдвиг
     */
    explicit CaesarCipher(int key);

    /**
     * Шифрование текста
     * @param plaintext - исходный текст
     * @return зашифрованный текст
     */
    std::string encrypt(const std::string& plaintext) const;

    /**
     * Дешифрование текста
     * @param ciphertext - зашифрованный текст
     * @return расшифрованный текст
     */
    std::string decrypt(const std::string& ciphertext) const;

    /**
     * Получить текущий ключ
     */
    int getKey() const;

    /**
     * Установить ключ
     */
    void setKey(int key);

private:
    int key_;
    char shiftChar(char c, int shift) const;
};

/**
 * Класс для работы с шифром Виженера
 */
class VigenereCipher
{
public:
    /**
     * Конструктор с указанием ключа (строка)
     * @param key - буквенный ключ
     */
    explicit VigenereCipher(const std::string& key);

    /**
     * Шифрование текста
     * @param plaintext - исходный текст
     * @return зашифрованный текст
     */
    std::string encrypt(const std::string& plaintext) const;

    /**
     * Дешифрование текста
     * @param ciphertext - зашифрованный текст
     * @return расшифрованный текст
     */
    std::string decrypt(const std::string& ciphertext) const;

    /**
     * Получить текущий ключ
     */
    std::string getKey() const;

    /**
     * Установить ключ
     */
    void setKey(const std::string& key);

private:
    std::string key_;
    char shiftChar(char c, int shift) const;
};

#endif // CIPHERS_HPP
