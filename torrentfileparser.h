#ifndef TORRENTFILEPARSER_H
#define TORRENTFILEPARSER_H

#include "bencode.h"
#include <string>
#include <vector>

// Класс TorrentFileParser предназначен для разбора торрент-файлов
class TorrentFileParser {
    private:
    std::shared_ptr<BDictionary> root; // Корневой элемент торрент-файла

    public:
    explicit TorrentFileParser(const std::string &filePath); // Конструктор класса
    long getFileSize() const;    // Получить размер файла из торрент-файла
    long getPieceLength() const; // Получить размер куска файла из торрент-файла
    std::string getFileName() const; // Получить имя файла из торрент-файла
    std::string getAnnounce() const; // Получить адрес трекера из торрент-файла
    std::shared_ptr<BItem> get(std::string key) const; // Получить элемент по ключу из торрент-файла
    std::string getInfoHash() const;                   // Получить хеш информации о файле
    std::vector<std::string> splitPieceHashes() const; // Разделить хеши кусков файла
    std::string getComment() const; // Получить комментарий к торрент-файлу
    std::string getCreatedBy() const; // Получить информацию о создателе торрент-файла
    std::string getCreationDate() const; // Получить дату создания торрент-файла
    std::vector<std::vector<std::string>> getAnnounceList() const; // Получить список адресов трекеров из торрент-файла
};

#endif                                                             // TORRENTFILEPARSER_H
