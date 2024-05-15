#include "bittorrentmessage.h"
#include "log.h"
#include <sstream>

// Конструктор класса BitTorrentMessage
BitTorrentMessage::BitTorrentMessage(const uint8_t id, const std::string &payload)
    : id(id), payload(payload), messageLength(payload.length() + 1)
{
}

// Метод для получения строкового представления сообщения
std::string BitTorrentMessage::toString()
{
    std::stringstream buffer;
    char *messageLengthAddr = (char *)&messageLength;
    std::string messageLengthStr;
    // Байты добавляются в обратном порядке, предполагая,
    // что данные хранятся в локальном порядке little-endian (нам нужен big-endian)
    for (int i = 0; i < 4; ++i)
    {
        messageLengthStr.push_back((char)messageLengthAddr[3 - i]);
    }
    buffer << messageLengthStr;
    buffer << (char)id;
    buffer << payload;
    return buffer.str();
}

// Метод для получения идентификатора сообщения
uint8_t BitTorrentMessage::getMessageId() const
{
    return id;
}

// Метод для получения нагрузки сообщения
std::string BitTorrentMessage::getPayload() const
{
    return payload;
}
