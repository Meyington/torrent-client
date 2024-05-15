#include <iostream>
#include <random>
#include <thread>

#include "peerconnection.h"
#include "peerretriever.h"
#include "piecemanager.h"
#include "torrentclient.h"
#include "torrentfile.h"

#define PORT 8080              // Лучше ставить от 8000 до 16000
#define PEER_QUERY_INTERVAL 60 // Интервал обновления списка пиров

TorrentClient::TorrentClient(const int threadNum) : threadNum(threadNum)
{
    peerId = "-UT2021-";
    // Генерация 12 случайных чисел для формирования уникального peerId
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> distrib(1, 9);
    for (int i = 0; i < 12; i++)
    {
        peerId += std::to_string(distrib(gen));
    }
}

TorrentClient::~TorrentClient() = default;

void TorrentClient::downloadFile(const std::string &torrentFilePath, const std::string &downloadDirectory)
{
    // Парсинг торрент-файла
    // explicit TorrentFile(const std::string &filePath); // Конструктор класса
    // long getFileSize() const;    // Получить размер файла из торрент-файла
    // long getPieceLength() const; // Получить размер куска файла из торрент-файла
    // std::string getFileName() const; // Получить имя файла из торрент-файла
    // std::string getAnnounce() const; // Получить адрес трекера из торрент-файла
    // std::shared_ptr<BItem> get(std::string key) const; // Получить элемент по ключу из торрент-файла
    // std::string getInfoHash() const;                   // Получить хеш информации о файле
    // std::vector<std::string> splitPieceHashes() const; // Разделить хеши кусков файла
    // std::string getComment() const; // Получить комментарий к торрент-файлу
    // std::string getCreatedBy() const; // Получить информацию о создателе торрент-файла
    // std::string getCreationDate() const; // Получить дату создания торрент-файла
    // std::vector<std::vector<std::string>> getAnnounceList() const; // Получить список адресов трекеров из
    // торрент-файла

    TorrentFile torrentFile(torrentFilePath);
    std::string announceUrl = torrentFile.getAnnounce();
    std::vector<std::vector<std::string>> announceList = torrentFile.getAnnounceList(); // Необязательно
    std::string сreationDate = torrentFile.getCreationDate();                           // Необязательно
    std::string comment = torrentFile.getComment();                                     // Необязательно
    std::string createdBy = torrentFile.getCreatedBy();                                 // Необязательно
    long fileSize = torrentFile.getFileSize();

    const std::string infoHash = torrentFile.getInfoHash();
    std::string filename = torrentFile.getFileName();
    std::string downloadPath = downloadDirectory + filename;
    PieceManager pieceManager(torrentFile, downloadPath, threadNum);

    // Инициализация соединений
    for (int i = 0; i < threadNum; i++)
    {
        PeerConnection connection(&queue, peerId, infoHash, &pieceManager);
        connections.push_back(&connection);
        std::thread thread(&PeerConnection::start, connection);
        threadPool.push_back(std::move(thread));
    }

    auto lastPeerQuery = (time_t)(-1);

    std::cout << "Download initiated..." << std::endl;

    // Цикл загрузки файла
    while (true)
    {
        if (pieceManager.isComplete())
        {
            break;
        }

        time_t currentTime = std::time(nullptr);
        auto diff = std::difftime(currentTime, lastPeerQuery);
        if (lastPeerQuery == -1 || diff >= PEER_QUERY_INTERVAL || queue.empty())
        {
            // Получение списка пиров
            PeerRetriever peerRetriever(peerId, announceUrl, infoHash, PORT, fileSize);
            std::vector<Peer *> peers = peerRetriever.retrievePeers(pieceManager.bytesDownloaded());
            lastPeerQuery = currentTime;
            if (!peers.empty())
            {
                queue.clear();
                for (auto *peer : peers)
                {
                    queue.push_back(peer);
                }
            }
        }
    }

    // Завершение загрузки
    terminate();

    if (pieceManager.isComplete())
    {
        std::cout << "Download completed!" << std::endl;
        std::cout << "File downloaded to " << downloadPath << std::endl;
    }
}

void TorrentClient::terminate()
{
    // Отправка заглушечных пиров в очередь для завершения потоков
    for (int i = 0; i < threadNum; i++)
    {
        Peer *dummyPeer = new Peer{"0.0.0.0", 0};
        queue.push_back(dummyPeer);
    }
    // Остановка соединений
    for (auto connection : connections)
    {
        connection->stop();
    }
    // Дожидаемся завершения потоков
    for (std::thread &thread : threadPool)
    {
        if (thread.joinable())
        {
            thread.join();
        }
    }
    // Очистка пула потоков
    threadPool.clear();
}
