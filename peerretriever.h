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
    /**
     * Декодирует строку ответа, отправленную трекером.
     * Если строка может быть успешно декодирована, возвращает список указателей на структуры Peer.
     * Обратите внимание, что эта функция обрабатывает два различных представления:
     * одно из них имеет пиров, обозначенных длинным двоичным блоком (компактный),
     * а другое представляет пиров в виде списка с полной информацией.
     * Первое может быть найдено в ответе от трекера kali-linux, в то время как второе
     * можно найти в ответе от других трекеров.
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
