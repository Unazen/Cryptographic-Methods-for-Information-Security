
#include "belt.hpp"

#include <cstdint>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <iterator>
#include <limits>
#include <stdexcept>
#include <string>
#include <vector>

using namespace std;

// Чтение данных из файла
static vector<uint8_t> readFile(const string& filename)
{
    ifstream file(filename, ios::binary);

    if (!file)
        throw runtime_error("Не удалось открыть входной файл.");

    return vector<uint8_t>(
        istreambuf_iterator<char>(file),
        istreambuf_iterator<char>());
}

// Запись данных в файл
static bool writeFile(
    const string& filename,
    const vector<uint8_t>& data)
{
    ofstream file(filename, ios::binary);

    if (!file)
        return false;

    if (!data.empty())
    {
        file.write(
            reinterpret_cast<const char*>(data.data()),
            static_cast<streamsize>(data.size()));
    }

    return file.good();
}

// Вывод данных в шестнадцатеричном формате
static void printHex(
    const string& label,
    const vector<uint8_t>& data)
{
    cout << label;

    for (uint8_t byte : data)
    {
        cout << hex << uppercase
             << setw(2) << setfill('0')
             << static_cast<unsigned>(byte);
    }

    cout << dec << setfill(' ') << '\n';
}

// Преобразование текста в байты
static vector<uint8_t> toBytes(const string& text)
{
    return vector<uint8_t>(text.begin(), text.end());
}

// Преобразование шестнадцатеричной строки в байты
static vector<uint8_t> readHex(
    const string& prompt,
    size_t size)
{
    string hexText;

    cout << prompt;

    if (!(cin >> hexText))
        throw runtime_error("Не удалось прочитать шестнадцатеричные данные.");

    if (hexText.size() != size * 2)
        throw runtime_error(
            "Неверная длина данных. Ожидается символов: "
            + to_string(size * 2) + ".");

    auto digit = [](char c) -> int
    {
        if (c >= '0' && c <= '9')
            return c - '0';

        if (c >= 'a' && c <= 'f')
            return c - 'a' + 10;

        if (c >= 'A' && c <= 'F')
            return c - 'A' + 10;

        return -1;
    };

    vector<uint8_t> result(size);

    for (size_t i = 0; i < size; ++i)
    {
        int high = digit(hexText[2 * i]);
        int low = digit(hexText[2 * i + 1]);

        if (high < 0 || low < 0)
            throw runtime_error(
                "Допускаются только шестнадцатеричные символы 0-9 и A-F.");

        result[i] = static_cast<uint8_t>((high << 4) | low);
    }

    return result;
}

// Ввод целого числа с проверкой
static int readChoice(const string& prompt, int min, int max)
{
    int value;

    while (true)
    {
        cout << prompt;

        if (cin >> value && value >= min && value <= max)
        {
            cin.ignore(
                numeric_limits<streamsize>::max(), '\n');
            return value;
        }

        cout << "Ошибка: введите число от "
             << min << " до " << max << ".\n";

        cin.clear();
        cin.ignore(
            numeric_limits<streamsize>::max(), '\n');
    }
}

// Выбор режима шифрования
static int chooseMode()
{
    cout << "\nВыберите режим:\n";
    cout << "1. ECB\n";
    cout << "2. CBC\n";

    return readChoice("Ваш выбор: ", 1, 2);
}

// Вывод основного меню
static void printMenu()
{
    cout << "\n========================================\n";
    cout << " Лабораторная работа №3 — BELT\n";
    cout << "========================================\n";
    cout << "1. Ввести сообщение с клавиатуры\n";
    cout << "2. Прочитать сообщение из файла\n";
    cout << "3. Зашифровать сообщение\n";
    cout << "4. Расшифровать сообщение\n";
    cout << "5. Вычислить MAC\n";
    cout << "6. Выполнить автоматические проверки\n";
    cout << "7. Сохранить результат в файл\n";
    cout << "0. Выход\n";
}

// Ввод ключа
static vector<uint8_t> readKey()
{
    return readHex(
        "Введите ключ (64 шестнадцатеричных символа): ",
        32);
}

// Ввод IV
static vector<uint8_t> readIV()
{
    return readHex(
        "Введите IV (32 шестнадцатеричных символа): ",
        16);
}

int main()
{
    vector<uint8_t> message;
    vector<uint8_t> encrypted;
    vector<uint8_t> decrypted;
    vector<uint8_t> mac;

    vector<uint8_t> key;
    vector<uint8_t> iv;

    bool hasMessage = false;
    bool hasKey = false;
    bool hasEncrypted = false;
    bool hasDecrypted = false;
    bool hasMAC = false;

    // Выбранный режим последнего шифрования
    int lastMode = 0;

    try
    {
        if (!BeltCipher::libraryReady())
        {
            cerr << "Ошибка: библиотека BELT не инициализирована.\n";
            return 1;
        }

        bool running = true;

        while (running)
        {
            printMenu();

            int choice = readChoice("Ваш выбор: ", 0, 7);

            try
            {
                switch (choice)
                {
                    // Ввод сообщения с клавиатуры
                    case 1:
                    {
                        string text;

                        cout << "\nВведите сообщение: ";

                        if (!getline(cin, text))
                            throw runtime_error(
                                "Не удалось прочитать сообщение.");

                        message = toBytes(text);

                        if (message.empty())
                            throw runtime_error(
                                "Сообщение не должно быть пустым.");

                        hasMessage = true;
                        hasEncrypted = false;
                        hasDecrypted = false;
                        hasMAC = false;

                        cout << "Сообщение успешно введено.\n";
                        break;
                    }

                    // Чтение сообщения из файла
                    case 2:
                    {
                        string filename;

                        cout << "\nВведите имя входного TXT-файла: ";

                        if (!getline(cin, filename) ||
                            filename.empty())
                        {
                            throw runtime_error(
                                "Имя файла не указано.");
                        }

                        message = readFile(filename);

                        if (message.empty())
                            throw runtime_error(
                                "Входной файл пуст.");

                        hasMessage = true;
                        hasEncrypted = false;
                        hasDecrypted = false;
                        hasMAC = false;

                        cout << "Сообщение успешно прочитано из файла.\n";
                        cout << "Размер сообщения: "
                             << message.size() << " байт.\n";
                        break;
                    }

                    // Шифрование сообщения
                    case 3:
                    {
                        if (!hasMessage)
                            throw runtime_error(
                                "Сначала введите сообщение "
                                "или прочитайте его из файла.");

                        key = readKey();
                        hasKey = true;

                        int mode = chooseMode();

                        iv.clear();

                        if (mode == 2)
                            iv = readIV();

                        BeltCipher cipher(key.data());

                        vector<uint8_t> result;

                        int status;

                        if (mode == 1)
                        {
                            status = cipher.encryptECB(
                                message.data(),
                                message.size(),
                                result);
                        }
                        else
                        {
                            status = cipher.encryptCBC(
                                message.data(),
                                message.size(),
                                iv.data(),
                                result);
                        }

                        if (status != 0)
                            throw runtime_error(
                                "Шифрование не выполнено. "
                                "Проверьте длину сообщения "
                                "и требования реализации BELT.");

                        encrypted = result;
                        hasEncrypted = true;
                        hasDecrypted = false;
                        hasMAC = false;
                        lastMode = mode;

                        cout << "\nШифрование "
                             << (mode == 1 ? "ECB" : "CBC")
                             << " выполнено.\n";

                        printHex("Шифртекст: ", encrypted);
                        break;
                    }

                    // Дешифрование сообщения
                    case 4:
                    {
                        if (!hasEncrypted)
                            throw runtime_error(
                                "Сначала зашифруйте сообщение "
                                "в пункте 3.");

                        if (!hasKey)
                            throw runtime_error(
                                "Ключ не задан.");

                        if (lastMode == 0)
                            throw runtime_error(
                                "Не удалось определить режим шифрования.");

                        BeltCipher cipher(key.data());

                        vector<uint8_t> result;
                        int status;

                        if (lastMode == 1)
                        {
                            status = cipher.decryptECB(
                                encrypted.data(),
                                encrypted.size(),
                                result);
                        }
                        else
                        {
                            status = cipher.decryptCBC(
                                encrypted.data(),
                                encrypted.size(),
                                iv.data(),
                                result);
                        }

                        if (status != 0)
                            throw runtime_error(
                                "Не удалось расшифровать сообщение.");

                        decrypted = result;
                        hasDecrypted = true;

                        cout << "\nРасшифрование "
                             << (lastMode == 1 ? "ECB" : "CBC")
                             << " выполнено.\n";

                        cout << "Расшифрованный текст: ";
                        cout.write(
                            reinterpret_cast<const char*>(
                                decrypted.data()),
                            static_cast<streamsize>(
                                decrypted.size()));
                        cout << '\n';

                        cout << "Проверка восстановления сообщения: "
                             << (decrypted == message
                                     ? "УСПЕШНО"
                                     : "ОШИБКА")
                             << '\n';
                        break;
                    }

                    // Вычисление MAC
                    case 5:
                    {
                        if (!hasMessage)
                            throw runtime_error(
                                "Сначала введите сообщение "
                                "или прочитайте его из файла.");

                        key = readKey();
                        hasKey = true;

                        BeltCipher cipher(key.data());

                        vector<uint8_t> result(
                            BeltCipher::MAC_SIZE);

                        if (cipher.computeMAC(
                                message.data(),
                                message.size(),
                                result.data()) != 0)
                        {
                            throw runtime_error(
                                "Не удалось вычислить MAC.");
                        }

                        mac = result;
                        hasMAC = true;

                        cout << "\nИмитовставка MAC вычислена.\n";
                        printHex("MAC: ", mac);
                        break;
                    }

                    // Автоматические проверки
                    case 6:
                    {
                        if (!hasMessage)
                            throw runtime_error(
                                "Сначала введите сообщение "
                                "или прочитайте его из файла.");

                        if (!hasKey)
                        {
                            key = readKey();
                            hasKey = true;
                        }

                        BeltCipher cipher(key.data());

                        cout << "\n========================================\n";
                        cout << " Автоматические проверки\n";
                        cout << "========================================\n";

                        bool allPassed = true;

                        // Проверка ECB
                        vector<uint8_t> encECB;
                        vector<uint8_t> decECB;

                        int encStatus = cipher.encryptECB(
                            message.data(),
                            message.size(),
                            encECB);

                        bool ecbPassed = false;

                        if (encStatus == 0)
                        {
                            int decStatus = cipher.decryptECB(
                                encECB.data(),
                                encECB.size(),
                                decECB);

                            ecbPassed =
                                decStatus == 0 && decECB == message;
                        }

                        cout << "1. ECB: "
                             << (ecbPassed ? "УСПЕШНО" : "ОШИБКА")
                             << '\n';

                        allPassed = allPassed && ecbPassed;

                        // Проверка CBC
                        vector<uint8_t> testIV = iv;

                        if (testIV.size() != 16)
                        {
                            testIV.resize(16, 0);
                        }

                        vector<uint8_t> encCBC;
                        vector<uint8_t> decCBC;

                        int cbcEncStatus = cipher.encryptCBC(
                            message.data(),
                            message.size(),
                            testIV.data(),
                            encCBC);

                        bool cbcPassed = false;

                        if (cbcEncStatus == 0)
                        {
                            int cbcDecStatus = cipher.decryptCBC(
                                encCBC.data(),
                                encCBC.size(),
                                testIV.data(),
                                decCBC);

                            cbcPassed =
                                cbcDecStatus == 0 &&
                                decCBC == message;
                        }

                        cout << "2. CBC: "
                             << (cbcPassed ? "УСПЕШНО" : "ОШИБКА")
                             << '\n';

                        allPassed = allPassed && cbcPassed;

                        // Проверка MAC
                        vector<uint8_t> macOriginal(
                            BeltCipher::MAC_SIZE);
                        vector<uint8_t> macChanged(
                            BeltCipher::MAC_SIZE);

                        bool macPassed = false;

                        int macStatus1 = cipher.computeMAC(
                            message.data(),
                            message.size(),
                            macOriginal.data());

                        vector<uint8_t> changed = message;
                        changed[0] ^= 0x01;

                        int macStatus2 = cipher.computeMAC(
                            changed.data(),
                            changed.size(),
                            macChanged.data());

                        if (macStatus1 == 0 && macStatus2 == 0)
                        {
                            macPassed = macOriginal != macChanged;
                        }

                        cout << "3. MAC для изменённого сообщения: "
                             << (macPassed
                                     ? "РАЗЛИЧАЕТСЯ"
                                     : "ПРОВЕРКА НЕ ПРОЙДЕНА")
                             << '\n';

                        allPassed = allPassed && macPassed;

                        // Проверка зависимости CBC от IV
                        vector<uint8_t> iv2 = testIV;
                        iv2[0] ^= 0x01;

                        vector<uint8_t> encCBC2;

                        int secondStatus = cipher.encryptCBC(
                            message.data(),
                            message.size(),
                            iv2.data(),
                            encCBC2);

                        bool ivPassed =
                            cbcEncStatus == 0 &&
                            secondStatus == 0 &&
                            encCBC != encCBC2;

                        cout << "4. CBC при разных IV: "
                             << (ivPassed
                                     ? "ШИФРТЕКСТЫ РАЗЛИЧАЮТСЯ"
                                     : "ПРОВЕРКА НЕ ПРОЙДЕНА")
                             << '\n';

                        allPassed = allPassed && ivPassed;

                        cout << "----------------------------------------\n";
                        cout << "Общий результат: "
                             << (allPassed
                                     ? "ВСЕ ПРОВЕРКИ ПРОЙДЕНЫ"
                                     : "ЕСТЬ НЕПРОЙДЕННЫЕ ПРОВЕРКИ")
                             << '\n';

                        break;
                    }

                    // Сохранение результата
                    case 7:
                    {
                        cout << "\nЧто сохранить?\n";
                        cout << "1. Исходное сообщение\n";
                        cout << "2. Шифртекст\n";
                        cout << "3. Расшифрованное сообщение\n";
                        cout << "4. MAC\n";

                        int what = readChoice(
                            "Ваш выбор: ", 1, 4);

                        const vector<uint8_t>* data = nullptr;
                        string defaultName;

                        if (what == 1 && hasMessage)
                        {
                            data = &message;
                            defaultName = "message.txt";
                        }
                        else if (what == 2 && hasEncrypted)
                        {
                            data = &encrypted;
                            defaultName = "encrypted.txt";
                        }
                        else if (what == 3 && hasDecrypted)
                        {
                            data = &decrypted;
                            defaultName = "decrypted.txt";
                        }
                        else if (what == 4 && hasMAC)
                        {
                            data = &mac;
                            defaultName = "mac.bin";
                        }
                        else
                        {
                            throw runtime_error(
                                "Выбранный результат ещё не создан.");
                        }

                        string filename;

                        cout << "Введите имя выходного файла "
                             << "(по умолчанию "
                             << defaultName << "): ";

                        if (!getline(cin, filename))
                            throw runtime_error(
                                "Не удалось прочитать имя файла.");

                        if (filename.empty())
                            filename = defaultName;

                        if (!writeFile(filename, *data))
                            throw runtime_error(
                                "Не удалось записать данные в файл.");

                        cout << "Результат сохранён в файл: "
                             << filename << '\n';
                        break;
                    }

                    case 0:
                    {
                        running = false;
                        cout << "\nРабота программы завершена.\n";
                        break;
                    }
                }
            }
            catch (const exception& e)
            {
                cout << "\nОшибка: " << e.what() << '\n';
            }
        }
    }
    catch (const exception& e)
    {
        cerr << "\nКритическая ошибка: " << e.what() << '\n';
        return 1;
    }
    catch (...)
    {
        cerr << "\nПроизошла неизвестная ошибка.\n";
        return 1;
    }

    return 0;
}
