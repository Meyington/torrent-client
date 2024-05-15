#ifndef TORRENTFILE_H
#define TORRENTFILE_H

#include "bencode.h"
#include <string>
#include <vector>

// Класс TorrentFile предназначен для разбора торрент-файлов
class TorrentFile {
    private:
    std::shared_ptr<BDictionary> root; // Корневой элемент торрент-файла

    public:
    explicit TorrentFile(const std::string &filePath); // Конструктор класса
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

    // Объявляем функцию вывода как friend класса TorrentFile
    friend std::ostream &operator<<(std::ostream &os, const TorrentFile &torrentFile)
    {
        os << "File Name: " << torrentFile.getFileName() << std::endl;
        os << "File Size: " << torrentFile.getFileSize() << " bytes" << std::endl;
        os << "Piece Length: " << torrentFile.getPieceLength() << " bytes" << std::endl;
        os << "Info Hash: " << torrentFile.getInfoHash() << std::endl;
        os << "Announce URL: " << torrentFile.getAnnounce() << std::endl;
        os << "Comment: " << torrentFile.getComment() << std::endl;
        os << "Created By: " << torrentFile.getCreatedBy() << std::endl;
        os << "Creation Date: " << torrentFile.getCreationDate() << std::endl;

        const std::vector<std::vector<std::string>> announceList = torrentFile.getAnnounceList();
        os << "Announce List:" << std::endl;
        for (const auto &list : announceList)
        {
            for (const auto &announce : list)
            {
                os << "    " << announce << std::endl;
            }
        }

        return os;
    }
};

#endif                                                             // TORRENTFILE_H
