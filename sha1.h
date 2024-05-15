#ifndef SHA1_H
#define SHA1_H

#include <iostream>
#include <string>

class SHA1 {
    public:
    SHA1(); // Конструктор класса, инициализирует объект SHA-1

    // Обновляет хешируемые данные строкой
    void update(const std::string &s);

    // Обновляет хешируемые данные потоком ввода
    void update(std::istream &is);

    // Завершает процесс хеширования и возвращает итоговый хеш в виде строки
    std::string final();

    // Статический метод для вычисления хеша файла по его имени.
    static std::string from_file(const std::string &filename);

    private:
    typedef unsigned long int uint32; // Определение типа для 32-битных беззнаковых целых чисел.
    typedef unsigned long long uint64; // Определение типа для 64-битных беззнаковых целых чисел.

    static const unsigned int DIGEST_INTS = 5; // Количество 32-битных целых чисел в хеше (160 бит).
    static const unsigned int BLOCK_INTS = 16; // Количество 32-битных целых чисел в блоке (512 бит).
    static const unsigned int BLOCK_BYTES = BLOCK_INTS * 4; // Размер блока в байтах.

    uint32 digest[DIGEST_INTS]; // Массив для хранения промежуточных значений хеша.
    std::string buffer;         // Буфер для хранения данных перед обработкой.
    uint64 transforms;          // Счетчик трансформаций блоков данных.

                                // Сбрасывает внутреннее состояние объекта SHA-1.
    void reset();

    // Выполняет основное преобразование SHA-1 для одного блока данных.
    void transform(uint32 block[BLOCK_BYTES]);

    // Преобразует строку в блок 32-битных целых чисел.
    static void buffer_to_block(const std::string &buffer, uint32 block[BLOCK_BYTES]);

    // Читает данные из потока ввода в строку.
    static void read(std::istream &is, std::string &s, int max);
};

// Функция для вычисления SHA-1 хеша строки.
std::string sha1(const std::string &string);

#endif // SHA1_H
