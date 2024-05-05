#include "torrentfile.h"

// Функция parseTorrentFile парсит торрент-файл и создает объект TorrentFile на его основе
TorrentFile parseTorrentFile(const std::string &filePath)
{
    TorrentFileParser parser(filePath); // Создание объекта парсера для заданного пути к файлу
    TorrentFile torrentFile; // Создание объекта TorrentFile для хранения данных о торрент-файле

    // Получение информации из парсера и сохранение ее в объекте TorrentFile
    torrentFile.announce = parser.getAnnounce();
    torrentFile.comment = parser.getComment();
    torrentFile.pieceHashes = parser.splitPieceHashes();
    torrentFile.pieceLength = parser.getPieceLength();
    torrentFile.length = parser.getFileSize();
    torrentFile.name = parser.getFileName();
    torrentFile.infoHash = parser.getInfoHash();

    return torrentFile; // Возвращение объекта TorrentFile
}
