#ifndef TORRENTFILE_H
#define TORRENTFILE_H

#include "torrentfileparser.h"
#include <string>
#include <vector>

// Структура TorrentFile содержит информацию о торрент-файле
struct TorrentFile
{
    std::string announce;                 // Адрес трекера
    std::string comment;                  // Комментарий к торрент-файлу
    std::vector<std::string> pieceHashes; // Хеши кусков файла
    size_t pieceLength;                   // Размер куска файла
    size_t length;                        // Общий размер файла
    std::string name;                     // Имя файла
    std::string infoHash;                 // Хеш информации о файле
};

// Функция parseTorrentFile парсит торрент-файл и возвращает объект TorrentFile
TorrentFile parseTorrentFile(const std::string &filePath);

#endif // TORRENTFILE_H
