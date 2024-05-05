#ifndef BITTORRENTMESSAGE_H
#define BITTORRENTMESSAGE_H

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

// Перечисление для идентификаторов сообщений
enum MessageId
{
    keepAlive = -1,
    choke = 0,
    unchoke = 1,
    interested = 2,
    notInterested = 3,
    have = 4,
    bitField = 5,
    request = 6,
    piece = 7,
    cancel = 8,
    port = 9
};

// Класс для представления сообщений BitTorrent
class BitTorrentMessage {
    private:
    const uint32_t messageLength; // Длина сообщения
    const uint8_t id;             // Идентификатор сообщения
    const std::string payload;    // Нагрузка сообщения

    public:
    // Конструктор
    explicit BitTorrentMessage(uint8_t id, const std::string &payload = "");

    // Получить строковое представление сообщения
    std::string toString();

    // Получить идентификатор сообщения
    uint8_t getMessageId() const;

    // Получить нагрузку сообщения
    std::string getPayload() const;
};

#endif // BITTORRENTMESSAGE_H
