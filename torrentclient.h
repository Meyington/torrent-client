#ifndef TORRENTCLIENT_H
#define TORRENTCLIENT_H

#include "SharedQueue.h"
#include "peerconnection.h"
#include "peerretriever.h"
#include <string>

class TorrentClient {
    private:
    const int threadNum;       // Количество потоков для загрузки
    std::string peerId;        // Идентификатор клиента
    SharedQueue<Peer *> queue; // Общая очередь для обмена данными между потоками
    std::vector<std::thread> threadPool;       // Пул потоков
    std::vector<PeerConnection *> connections; // Вектор для хранения соединений

    public:
    explicit TorrentClient(int threadNum = 5); // Конструктор с параметром по умолчанию
    ~TorrentClient();                          // Деструктор
    void terminate();                          // Завершает загрузку
    void downloadFile(const std::string &torrentFilePath,
                      const std::string &downloadDirectory); // Метод для загрузки файла
};

#endif                                                       // TORRENTCLIENT_H
