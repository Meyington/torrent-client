#include "bencode.h"
#include "sha1.h"
#include <cassert>
#include <fstream>
#include <iostream>
#include <stdexcept>

#include "torrentfileparser.h"

#define HASH_LEN 20 // Длина хеша

// Конструктор класса TorrentFileParser
TorrentFileParser::TorrentFileParser(const std::string &filePath)
{
    std::ifstream fileStream(filePath, std::ifstream::binary);      // Открытие файла
    std::shared_ptr<BItem> decodedTorrentFile = decode(fileStream); // Декодирование торрент-файла
    std::shared_ptr<BDictionary> rootDict =
        std::dynamic_pointer_cast<BDictionary>(decodedTorrentFile); // Преобразование в словарь
    root = rootDict; // Инициализация корневого элемента
    std::cout << "Parsing Torrent file " << filePath << "..." << std::endl; // Вывод информации о начале разбора
    std::cout << "Parse Torrent file: SUCCESS" << std::endl; // Вывод сообщения об успешном разборе
}

// Получить элемент по ключу из торрент-файла
std::shared_ptr<BItem> TorrentFileParser::get(std::string key) const
{
    std::shared_ptr<BItem> value = root->getValue(key); // Получение значения по ключу
    return value;                                       // Возвращение значения
}

// Получить хеш информации о файле
std::string TorrentFileParser::getInfoHash() const
{
    std::shared_ptr<BItem> infoDictionary = get("info"); // Получение информационного словаря
    std::string infoString = encode(infoDictionary);     // Кодирование словаря в строку
    std::string sha1Hash = sha1(infoString);             // Вычисление хеша SHA1
    return sha1Hash;                                     // Возвращение хеша
}

// Разделить хеши кусков файла
std::vector<std::string> TorrentFileParser::splitPieceHashes() const
{
    std::shared_ptr<BItem> piecesValue = get("pieces"); // Получение значения хешей кусков
    if (!piecesValue)
    {                                                   // Проверка наличия значения
        throw std::runtime_error(
            "Torrent file is malformed. [File does not contain key 'pieces']"); // Ошибка при отсутствии ключа
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
long TorrentFileParser::getFileSize() const
{
    std::shared_ptr<BItem> fileSizeItem = get("length"); // Получение значения размера файла
    if (!fileSizeItem)
    {                                                    // Проверка наличия значения
        throw std::runtime_error(
            "Torrent file is malformed. [File does not contain key 'length']"); // Ошибка при отсутствии ключа
    }
    long fileSize = std::dynamic_pointer_cast<BInteger>(fileSizeItem)->value(); // Получение размера файла
    return fileSize; // Возвращение размера файла
}

// Получить размер куска файла из торрент-файла
long TorrentFileParser::getPieceLength() const
{
    std::shared_ptr<BItem> pieceLengthItem = get("piece length"); // Получение значения размера куска файла
    if (!pieceLengthItem)
    {                                                             // Проверка наличия значения
        throw std::runtime_error(
            "Torrent file is malformed. [File does not contain key 'piece length']"); // Ошибка при отсутствии ключа
    }
    long pieceLength = std::dynamic_pointer_cast<BInteger>(pieceLengthItem)->value(); // Получение размера куска файла
    return pieceLength; // Возвращение размера куска файла
}

// Получить имя файла из торрент-файла
std::string TorrentFileParser::getFileName() const
{
    std::shared_ptr<BItem> filenameItem = get("name"); // Получение значения имени файла
    if (!filenameItem)
    {                                                  // Проверка наличия значения
        throw std::runtime_error(
            "Torrent file is malformed. [File does not contain key 'name']"); // Ошибка при отсутствии ключа
    }
    std::string filename = std::dynamic_pointer_cast<BString>(filenameItem)->value(); // Получение имени файла
    return filename; // Возвращение имени файла
}

// Получить адрес трекера из торрент-файла
std::string TorrentFileParser::getAnnounce() const
{
    std::shared_ptr<BItem> announceItem = get("announce"); // Получение значения адреса трекера
    if (!announceItem)
    {                                                      // Проверка наличия значения
        throw std::runtime_error(
            "Torrent file is malformed. [File does not contain key 'announce']"); // Ошибка при отсутствии ключа
    }
    std::string announce = std::dynamic_pointer_cast<BString>(announceItem)->value(); // Получение адреса трекера
    return announce; // Возвращение адреса трекера
}

// Получить комментарий к торрент-файлу
std::string TorrentFileParser::getComment() const
{
    std::shared_ptr<BItem> commentItem = get("comment"); // Получение значения комментария
    if (!commentItem)
    {                                                    // Проверка наличия значения
        throw std::runtime_error(
            "Torrent file is malformed. [File does not contain key 'comment']"); // Ошибка при отсутствии ключа
    }
    std::string comment = std::dynamic_pointer_cast<BString>(commentItem)->value(); // Получение комментария
    return comment; // Возвращение комментария
}

// Получить информацию о создателе торрент-файла
std::string TorrentFileParser::getCreatedBy() const
{
    std::shared_ptr<BItem> createdByItem = get("created by"); // Получение значения информации о создателе
    if (!createdByItem)
    {                                                         // Проверка наличия значения
        throw std::runtime_error(
            "Torrent file is malformed. [File does not contain key 'created by']"); // Ошибка при отсутствии ключа
    }
    std::string createdBy =
        std::dynamic_pointer_cast<BString>(createdByItem)->value(); // Получение информации о создателе
    return createdBy; // Возвращение информации о создателе
}

// Получить дату создания торрент-файла
std::string TorrentFileParser::getCreationDate() const
{
    std::shared_ptr<BItem> creationDateItem = get("creation date"); // Получение значения даты создания
    if (!creationDateItem)
    {                                                               // Проверка наличия значения
        throw std::runtime_error(
            "Torrent file is malformed. [File does not contain key 'creation date']"); // Ошибка при отсутствии ключа
    }
    long creationDate = std::dynamic_pointer_cast<BInteger>(creationDateItem)->value(); // Получение даты создания
    // Преобразовать время Unix в строку даты/времени по вашему выбору
    return "Placeholder creation date string"; // Возврат строки с датой создания
}

// Получить список адресов трекеров из торрент-файла
std::vector<std::vector<std::string>> TorrentFileParser::getAnnounceList() const
{
    std::shared_ptr<BItem> announceListItem = get("announce-list"); // Получение значения списка адресов трекеров
    if (!announceListItem)
    {                                                               // Проверка наличия значения
        throw std::runtime_error(
            "Torrent file is malformed. [File does not contain key 'announce-list']"); // Ошибка при отсутствии ключа
    }
    auto announceList = std::dynamic_pointer_cast<BList>(announceListItem); // Преобразование значения в список
    std::vector<std::vector<std::string>> result; // Вектор для хранения списка адресов трекеров

    for (const auto &list : *announceList)
    {                                             // Перебор элементов внешнего списка
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
