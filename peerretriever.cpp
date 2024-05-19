#include "bencode.h"
#include <bitset>
#include <cpr/cpr.h>
#include <iostream>
#include <random>
#include <stdexcept>
#include <string>
#include <utility>

#include "peerretriever.h"
#include "utils.h"
#define TRACKER_TIMEOUT 15000 // Определяет тайм-аут для трекера в миллисекундах

PeerRetriever::PeerRetriever(
    std::string peerId, std::string announceUrl, std::string infoHash, int port, const unsigned long fileSize)
    : fileSize(fileSize)
{
    // Инициализирует переменные класса.
    this->peerId = std::move(peerId);
    this->announceUrl = std::move(announceUrl);
    this->infoHash = std::move(infoHash);
    this->port = port;
}

std::vector<Peer *> PeerRetriever::retrievePeers(unsigned long bytesDownloaded)
{
    // Формирует строку с информацией о параметрах запроса.
    std::stringstream info;
    info << "Retrieving peers from " << announceUrl << " with the following parameters..." << std::endl;
    // хэш информации будет закодирован в URL-формате библиотекой cpr
    info << "info_hash: " << infoHash << std::endl;
    info << "peer_id: " << peerId << std::endl;
    info << "port: " << port << std::endl;
    info << "uploaded: " << 0 << std::endl;
    info << "downloaded: " << std::to_string(bytesDownloaded) << std::endl;
    info << "left: " << std::to_string(fileSize - bytesDownloaded) << std::endl;
    info << "compact: " << std::to_string(1);

    // Выполняет HTTP-запрос к трекеру.
    cpr::Response res = cpr::Get(cpr::Url{announceUrl},
                                 cpr::Parameters{{"info_hash", std::string(hexDecode(infoHash))},
                                                 {"peer_id", std::string(peerId)},
                                                 {"port", std::to_string(port)},
                                                 {"uploaded", std::to_string(0)},
                                                 {"downloaded", std::to_string(bytesDownloaded)},
                                                 {"left", std::to_string(fileSize - bytesDownloaded)},
                                                 {"compact", std::to_string(1)}},
                                 cpr::Timeout{TRACKER_TIMEOUT});

    // Если ответ успешно получен
    if (res.status_code == 200)
    {
        // Декодирует ответ и получает список пиров.
        std::vector<Peer *> peers = decodeResponse(res.text);
        return peers;
    }
    else
    {
        // В случае ошибки в запросе возвращает пустой вектор.
        return std::vector<Peer *>();
    }
}

std::vector<Peer *> PeerRetriever::decodeResponse(std::string response)
{
    // декодирует
    std::shared_ptr<BItem> decodedResponse = decode(response);

    // Проверяет, что ответ является словарем.
    std::shared_ptr<BDictionary> responseDict = std::dynamic_pointer_cast<BDictionary>(decodedResponse);
    if (!responseDict)
    {
        throw std::runtime_error("Response returned by the tracker is not in the correct format. [Not a dictionary]");
    }
    // Получает значение 'peers' из словаря.
    std::shared_ptr<BItem> peersValue = responseDict->getValue("peers");
    if (!peersValue)
    {
        throw std::runtime_error("Response returned by the tracker is not in the correct format. ['peers' not found]");
    }
    // Инициализирует вектор для хранения пиров.
    std::vector<Peer *> peers;

    // Обрабатывает случай, когда информация о пирах отправляется в компактном виде.
    if (typeid(*peersValue) == typeid(BString))
    {
        // Разбирает информацию о пирах.
        const int peerInfoSize = 6;
        std::string peersString = std::dynamic_pointer_cast<BString>(peersValue)->value();

        // Проверяет целостность данных.
        if (peersString.length() % peerInfoSize != 0)
        {
            throw std::runtime_error(
                "Received malformed 'peers' from tracker. ['peers' length needs to be divisible by 6]");
        }
        // Извлекает IP-адреса и порты каждого пира из компактного представления.
        const int peerNum = peersString.length() / peerInfoSize;
        for (int i = 0; i < peerNum; i++)
        {
            int offset = i * peerInfoSize;
            std::stringstream peerIp;
            peerIp << std::to_string((uint8_t)peersString[offset]) << ".";
            peerIp << std::to_string((uint8_t)peersString[offset + 1]) << ".";
            peerIp << std::to_string((uint8_t)peersString[offset + 2]) << ".";
            peerIp << std::to_string((uint8_t)peersString[offset + 3]);
            int peerPort = bytesToInt(peersString.substr(offset + 4, 2));
            // Создает новый объект Peer и добавляет его в вектор пиров.
            Peer *newPeer = new Peer{peerIp.str(), peerPort};
            peers.push_back(newPeer);
        }
    }
    // Обрабатывает случай, когда информация о пирах хранится в виде списка.
    else if (typeid(*peersValue) == typeid(BList))
    {
        // Получает список пиров из словаря.
        std::shared_ptr<BList> peerList = std::dynamic_pointer_cast<BList>(peersValue);
        for (auto &item : *peerList)
        {
            // Преобразует каждый элемент в словарь.
            std::shared_ptr<BDictionary> peerDict = std::dynamic_pointer_cast<BDictionary>(item);

            // Получает IP-адрес пира из словаря.
            std::shared_ptr<BItem> tempPeerIp = peerDict->getValue("ip");
            if (!tempPeerIp)
                throw std::runtime_error("Received malformed 'peers' from tracker. [Item does not contain key 'ip']");
            std::string peerIp = std::dynamic_pointer_cast<BString>(tempPeerIp)->value();

            // Получает порт пира из словаря.
            std::shared_ptr<BItem> tempPeerPort = peerDict->getValue("port");
            if (!tempPeerPort)
                throw std::runtime_error("Received malformed 'peers' from tracker. [Item does not contain key 'port']");
            int peerPort = (int)std::dynamic_pointer_cast<BInteger>(tempPeerPort)->value();

            // Создает новый объект Peer и добавляет его в вектор пиров.
            Peer *newPeer = new Peer{peerIp, peerPort};
            peers.push_back(newPeer);
        }
    }
    else
    {
        throw std::runtime_error(
            "Response returned by the tracker is not in the correct format. ['peers' has the wrong type]");
    }
    return peers;
}
