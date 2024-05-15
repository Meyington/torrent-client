#ifndef PEERRETRIEVER_H
#define PEERRETRIEVER_H

#include <cpr/cpr.h>
#include <cstdint>
#include <initializer_list>
#include <ios>
#include <istream>
#include <list>
#include <map>
#include <memory>
#include <regex>
#include <sstream>
#include <string>
#include <vector>

struct Peer
{
    std::string ip;               // IP-адрес пира
    int port;                     // Порт пира
};

class PeerRetriever {
    private:
    std::string announceUrl;      // URL трекера
    std::string infoHash;         // Хэш информации
    std::string peerId;           // Идентификатор пира
    int port;                     // Порт для соединения с пирами
    const unsigned long fileSize; // Размер файла
    /*
     декодирует строку ответа, отправленную трекером
     если строка может быть успешно декодирована, возвращает список указателей на структуры peer
     */
    std::vector<Peer *> decodeResponse(std::string response);

    public:
    explicit PeerRetriever(std::string peerId,
                           std::string announceUrl,
                           std::string infoHash,
                           int port,
                           unsigned long fileSize);                       // Конструктор класса
    std::vector<Peer *> retrievePeers(unsigned long bytesDownloaded = 0); // Метод для извлечения списка пиров
};

#endif                                                                    // PEERRETRIEVER_H
