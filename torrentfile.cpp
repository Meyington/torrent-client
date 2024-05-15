#include "bencode.h"
#include "sha1.h"
#include "utils.h"
#include <cassert>
#include <fstream>
#include <iostream>
#include <stdexcept>

#include "torrentfile.h"

#define HASH_LEN 20 // Длина хеша

// Конструктор класса TorrentFile
TorrentFile::TorrentFile(const std::string &filePath)
{
    std::ifstream fileStream(filePath, std::ifstream::binary);      // Открытие файла
    std::shared_ptr<BItem> decodedTorrentFile = decode(fileStream); // Декодирование торрент-файла
    std::shared_ptr<BDictionary> rootDict =
        std::dynamic_pointer_cast<BDictionary>(decodedTorrentFile); // Преобразование в словарь
    root = rootDict; // Инициализация корневого элемента
    std::cout << "Парсим торрент " << filePath << "..." << std::endl; // Вывод информации о начале разбора
    std::cout << "Статус парсинга: УСПЕШНО" << std::endl; // Вывод сообщения об успешном разборе
}

// Получить элемент по ключу из торрент-файла
std::shared_ptr<BItem> TorrentFile::get(std::string key) const
{
    std::shared_ptr<BItem> value = root->getValue(key); // Получение значения по ключу
    return value;                                       // Возвращение значения
}

// Получить хеш информации о файле
std::string TorrentFile::getInfoHash() const
{
    std::shared_ptr<BItem> infoDictionary = get("info"); // Получение информационного словаря
    std::string infoString = encode(infoDictionary);     // Кодирование словаря в строку
    std::string sha1Hash = sha1(infoString);             // Вычисление хеша SHA1
    return sha1Hash;                                     // Возвращение хеша
}

// Разделить хеши кусков файла
std::vector<std::string> TorrentFile::splitPieceHashes() const
{
    std::shared_ptr<BItem> piecesValue = get("pieces"); // Получение значения хешей кусков
    if (!piecesValue)
    {                                                   // Проверка наличия значения
        throw std::runtime_error(
            "Торрент-файл поврежден. [Файл не содержит ключ 'pieces']"); // Ошибка при отсутствии ключа
    }
    std::string pieces = std::dynamic_pointer_cast<BString>(piecesValue)->value(); // Получение строки хешей кусков
    std::vector<std::string> pieceHashes; // Вектор для хранения хешей

    assert(pieces.size() % HASH_LEN == 0); // Проверка соответствия длины хешей кусков
    int piecesCount = (int)pieces.size() / HASH_LEN; // Вычисление количества кусков
    pieceHashes.reserve(piecesCount);                // Резервирование памяти
    for (int i = 0; i < piecesCount; i++)
    {                                                // Перебор кусков
        pieceHashes.push_back(pieces.substr(i * HASH_LEN, HASH_LEN)); // Добавление хеша куска в вектор
    }
    return pieceHashes; // Возвращение вектора хешей
}

// Получить размер файла из торрент-файла
long TorrentFile::getFileSize() const
{
    std::shared_ptr<BItem> fileSizeItem = get("length"); // Получение значения размера файла
    if (!fileSizeItem)
    {                                                    // Проверка наличия значения
        throw std::runtime_error(
            "Торрент-файл поврежден. [Файл не содержит ключ 'length']"); // Ошибка при отсутствии ключа
    }
    long fileSize = std::dynamic_pointer_cast<BInteger>(fileSizeItem)->value(); // Получение размера файла
    return fileSize; // Возвращение размера файла
}

// Получить размер куска файла из торрент-файла
long TorrentFile::getPieceLength() const
{
    std::shared_ptr<BItem> pieceLengthItem = get("piece length"); // Получение значения размера куска файла
    if (!pieceLengthItem)
    {                                                             // Проверка наличия значения

        throw std::runtime_error(
            "Торрент-файл поврежден. [Файл не содержит ключ 'piece length']"); // Ошибка при отсутствии ключа
    }
    long pieceLength = std::dynamic_pointer_cast<BInteger>(pieceLengthItem)->value(); // Получение размера куска файла
    return pieceLength; // Возвращение размера куска файла
}

// Получить имя файла из торрент-файла
std::string TorrentFile::getFileName() const
{
    std::shared_ptr<BItem> filenameItem = get("name"); // Получение значения имени файла
    if (!filenameItem)
    {                                                  // Проверка наличия значения
        throw std::runtime_error(
            "Торрент-файл поврежден. [Файл не содержит ключ 'name']"); // Ошибка при отсутствии ключа
    }
    std::string filename = std::dynamic_pointer_cast<BString>(filenameItem)->value(); // Получение имени файла
    return filename; // Возвращение имени файла
}

// Получить адрес трекера из торрент-файла
std::string TorrentFile::getAnnounce() const
{
    std::shared_ptr<BItem> announceItem = get("announce"); // Получение значения адреса трекера
    if (!announceItem)
    {                                                      // Проверка наличия значения
        throw std::runtime_error(
            "Торрент-файл поврежден. [Файл не содержит ключ 'announce']"); // Ошибка при отсутствии ключа
    }
    std::string announce = std::dynamic_pointer_cast<BString>(announceItem)->value(); // Получение адреса трекера
    return announce;                                     // Возвращение адреса трекера
}

std::string TorrentFile::getComment() const
{
    std::shared_ptr<BItem> commentItem = get("comment"); // Попытка получить значение комментария
    if (!commentItem)
    {                                                    // Если значение не найдено
        std::cout << "Предупреждение: Комментарий не найден в торрент-файле." << std::endl; // Вывод сообщения в консоль
        return "Комментарий отсутствует"; // Возвращение "No comment"
    }

    std::string comment = std::dynamic_pointer_cast<BString>(commentItem)->value(); // Получение комментария
    return comment; // Возвращение комментария
}

// Получить информацию о создателе торрент-файла
std::string TorrentFile::getCreatedBy() const
{
    std::shared_ptr<BItem> createdByItem = get("created by"); // Получение значения информации о создателе

    if (!createdByItem)
    {                                                         // Если значение не найдено
        std::cout << "Предупреждение: Создатель не найден в торрент-файле." << std::endl; // Вывод сообщения в консоль
        return "Создатель не указан";                               // Возвращение "No comment"
    }
    std::string createdBy =
        std::dynamic_pointer_cast<BString>(createdByItem)->value(); // Получение информации о создателе
    return createdBy; // Возвращение информации о создателе
}

// Получить дату создания торрент-файла
std::string TorrentFile::getCreationDate() const
{
    std::shared_ptr<BItem> creationDateItem = get("creation date"); // Получение значения даты создания

    if (!creationDateItem)
    {                                                               // Если значение не найдено
        std::cout << "Предупреждение: Дата создания не найдена в торрент-файле."
                  << std::endl;                                     // Вывод сообщения в консоль
        return "Дата создания не указана";                          // Возвращение "No comment"
    }

    long creationDate = std::dynamic_pointer_cast<BInteger>(creationDateItem)->value(); // Получение даты создания
    // std::string formatTime(long seconds)

    std::cout << creationDate;
    return "Заглушка строки с датой создания"; // Возврат строки с датой создания
}

// Получить список адресов трекеров из торрент-файла
std::vector<std::vector<std::string>> TorrentFile::getAnnounceList() const
{
    std::shared_ptr<BItem> announceListItem = get("announce-list"); // Получение значения списка адресов трекеров
    if (!announceListItem)
    {
        // Проверка наличия значения
        std::cout << "Предупреждение: Список адресов трекеров не найден в торрент-файле."
                  << std::endl;                         // Вывод сообщения в консоль
        return std::vector<std::vector<std::string>>(); // Возвращение пустого вектора
    }

    auto announceList = std::dynamic_pointer_cast<BList>(announceListItem); // Преобразование значения в список
    std::vector<std::vector<std::string>> result; // Вектор для хранения списка адресов трекеров

    for (const auto &list : *announceList)
    {                                             // Перебор элементов внешнего; списка
        std::vector<std::string> innerList; // Внутренний вектор для хранения адресов одного трекера
        for (const auto &item : *std::dynamic_pointer_cast<BList>(list))
        {                                   // Перебор элементов внутренних списков
            innerList.push_back(
                std::dynamic_pointer_cast<BString>(item)->value()); // Добавление адреса трекера во внутренний вектор
        }
        result.push_back(innerList); // Добавление внутреннего вектора во внешний
    }

    return result;                   // Возвращение списка адресов трекеров
}
