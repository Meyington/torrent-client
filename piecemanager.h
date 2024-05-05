#ifndef PIECEMANAGER_H
#define PIECEMANAGER_H

#include <ctime>
#include <fstream>
#include <map>
#include <mutex>
#include <thread>
#include <vector>

#include "piece.h"
#include "torrentfileparser.h"

struct PendingRequest
{
    Block *block; // Указатель на блок данных, ожидающий загрузки
    time_t timestamp; // Временная метка для отслеживания истечения времени ожидания
};

/**
 * Отвечает за отслеживание всех доступных фрагментов от пиров.
 */
class PieceManager {
    private:
    std::map<std::string, std::string> peers; // Пиры, участвующие в обмене
    std::vector<Piece *> missingPieces;       // Фрагменты, которые еще не загружены
    std::vector<Piece *> ongoingPieces; // Фрагменты, которые находятся в процессе загрузки
    std::vector<Piece *> havePieces;               // Загруженные фрагменты
    std::vector<PendingRequest *> pendingRequests; // Ожидающие запросы на загрузку блоков
    std::ofstream downloadedFile; // Файл, в который происходит запись загруженных данных
    const long pieceLength;              // Размер фрагмента
    const TorrentFileParser &fileParser; // Парсер торрент-файла
    const int maximumConnections;        // Максимальное количество соединений
    int piecesDownloadedInInterval = 0; // Количество загруженных фрагментов за интервал времени
    time_t startingTime;                       // Время начала загрузки
    int totalPieces{};                         // Общее количество фрагментов

    std::mutex lock;                           // Мьютекс для предотвращения гонок

    std::vector<Piece *> initiatePieces();     // Инициализация фрагментов и блоков
    Block *expiredRequest(std::string peerId); // Поиск просроченных запросов
    Block *nextOngoing(std::string peerId);    // Поиск следующего блока для загрузки
    Piece *getRarestPiece(std::string peerId); // Получение редкого фрагмента для загрузки
    void write(Piece *piece);                  // Запись данных фрагмента в файл
    void displayProgressBar();                 // Отображение прогресса загрузки
    void trackProgress();                      // Отслеживание прогресса загрузки

    public:
    explicit PieceManager(const TorrentFileParser &fileParser, const std::string &downloadPath, int maximumConnections);
    ~PieceManager();
    bool isComplete();
    void blockReceived(std::string peerId, int pieceIndex, int blockOffset, std::string data);
    void addPeer(const std::string &peerId, std::string bitField);
    void removePeer(const std::string &peerId);
    void updatePeer(const std::string &peerId, int index);
    unsigned long bytesDownloaded();
    Block *nextRequest(std::string peerId);
};

#endif // PIECEMANAGER_H
